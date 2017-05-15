#include <jni.h>

#include <android/log.h>
#include <android/bitmap.h>
#include <stdexcept>

#include "oversegmentation/SLIC.h"
#include "oversegmentation/superpixel.h"
#include "svm/svmWrapper.h"
#include "svm/seedsTools.h"

#include "opengm/opengm.h"
#include "opengm/graphicalmodel/graphicalmodel.h"
#include "opengm/graphicalmodel/space/simplediscretespace.h"
#include "opengm/functions/potts.h"

#include "opengm/inference/alphaexpansionfusion.h"



extern "C" {

typedef double                                                                 ValueType;
typedef size_t                                                                 IndexType;          // type used for indexing nodes and factors (default : size_t)
typedef size_t                                                                 LabelType;          // type
typedef opengm::ExplicitFunction<ValueType,IndexType,LabelType>                ExplicitFunction;   // shortcut for explicit function
typedef opengm::PottsFunction<ValueType,IndexType,LabelType>                   PottsFunction;      // shortcut for Potts function
typedef opengm::meta::TypeListGenerator<ExplicitFunction,PottsFunction>::type  FunctionTypeList;   // list of all function the model can use (this trick avoids virtual methods) - here only one
typedef opengm::GraphicalModel<float, opengm::Adder, opengm::ExplicitFunction<ValueType>,opengm::SimpleDiscreteSpace<> > Model;
typedef Model::FunctionIdentifier FunctionIdentifier;
//inference
typedef opengm::AlphaExpansionFusion<Model,opengm::Minimizer>  ALPHA_EXP;

#define  LOG_TAG    "libSCIS"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#define WRONG_BITMAP_TYPE 1;
struct Neighbor{
    double dist;
    int idx;
    Neighbor(double dist=-1,int idx=-1):dist(dist),idx(idx){

    };
    bool operator < (const Neighbor& other) const
    {
        return (dist < other.dist);
    }
};

class SCIS{
private:
    vector<SimpleSuperpixel>  superpixels;
    int height;
    int width;
    Model model;
    SvmWithColorShapeFeatures* svm;
    vector<vector<double>> regularizationPrior;
    //parameter
    double lambda=0.7;//regularization term weight
    int maxNeighbors=4;//maximum number of significativ neighbors

    void computeDataTerm(uint32_t* seeds,int height,int width){
        //use SVM to compute data term
        svm=new SvmWithColorShapeFeatures(seeds,width,height,superpixels);
        svm->train();
        svm->classify();
        //compute regularization apriori
        regularizationPrior.assign(svm->nbClasses(),vector<double>(svm->nbClasses(),0.));
        vector<double> nbRegData(svm->nbClasses(),0);
        for(int spIdx1=0;spIdx1<superpixels.size();spIdx1++){
            for(int i=0;i<superpixels[spIdx1].getNbNeighbors();i++){
                int spIdx2=superpixels[spIdx1].getNeighbor(i);
                int c1=svm->getClass(spIdx1);
                int c2=svm->getClass(spIdx2);
                regularizationPrior[c1][c2]++;
                nbRegData[c1]++;
            }
        }
        //normalize regularization apriori
        for(int i=0;i<regularizationPrior.size();i++){
            for(int j=0;j<regularizationPrior[i].size();j++){
                regularizationPrior[i][j]/=nbRegData[i];
            }
        }
    }

    void createModel(){
        size_t nrOfVariables=superpixels.size();
        size_t nbPossiblesValues=svm->nbClasses();
        model=Model(opengm::SimpleDiscreteSpace<>(nrOfVariables,nbPossiblesValues));
        const size_t shape_data[] = {nbPossiblesValues};
        opengm::ExplicitFunction<ValueType> f_data(shape_data, shape_data + 1);
        const size_t shape_smoothness[] = {nbPossiblesValues,nbPossiblesValues};
        opengm::ExplicitFunction<ValueType> f_smoothness(shape_smoothness, shape_smoothness + 2);
        for(int spIdx1=0;spIdx1<superpixels.size();spIdx1++){
            //data term
            for(int j=0;j<nbPossiblesValues;j++){
                //minimise
                f_data(j)=(1-svm->getProbas(spIdx1)[j]);//probability for each class
            }
            FunctionIdentifier id = model.addFunction(f_data);
            size_t vi[] = {size_t(spIdx1)};
            model.addFactor(id, vi, vi + 1);
            vector<Neighbor> neighbors;
            for(int i=0;i<superpixels[spIdx1].getNbNeighbors();i++){
                int spIdx2=superpixels[spIdx1].getNeighbor(i);
                if(spIdx1<spIdx2){
                    neighbors.push_back(Neighbor(superpixels[spIdx1].dist(superpixels[spIdx2]),spIdx2));
                }
            }

            sort(neighbors.begin(),neighbors.end());
            int nbSignificativeNeighbors=min(maxNeighbors,int(neighbors.size()));
            double neighborsRegWeight=lambda/double(nbSignificativeNeighbors);
            for(int n=0;n<nbSignificativeNeighbors;n++){
                for(int j=0;j<nbPossiblesValues;j++){
                    for(int k=0;k<nbPossiblesValues;k++){
                        f_smoothness(j,k)=neighborsRegWeight*(1-regularizationPrior[j][k]);
                    }
                }
                size_t vi2[] = {size_t(spIdx1),size_t(neighbors[n].idx)};
                FunctionIdentifier id = model.addFunction(f_smoothness);
                model.addFactor(id, vi2, vi2 + 2);
            }

        }
    }
public:
    SCIS(){};
    ~SCIS(){
        delete svm;

    };
    int nbSuperpixels(){
        return superpixels.size();
    }
    void oversegmentWithSlic(uint32_t* pixels,int height,int width){
        SLIC slic;
        int* labels;
        int nbSuperpixels;
        this->height=height;
        this->width=width;
        //oversegment with slic
        slic.DoSuperpixelSegmentation_ForGivenNumberOfSuperpixels(pixels,width,height,labels,nbSuperpixels,1000,10.);


        superpixels.assign(nbSuperpixels,SimpleSuperpixel());
        //store superpixel features
        for(int y=0;y<height;y++){
            for(int x=0;x<width;x++){
                //linear index
                int i=x+y*width;

                //get color
                uint32_t pixel=(uint32_t )pixels[i];
                int red = (pixel & 0xff0000) >> 16;
                int green = (pixel & 0x00ff00) >> 8;
                int blue = (pixel & 0x0000ff) >> 0;

                int labelSp=labels[i];
                //update color features
                //  superpixels[labelSp].addPixel(red[y][x],green[y][x],blue[y][x],x,y,lbps[x+y*width]);
                superpixels[labelSp].addPixel(red,green,blue,x,y);
                //update neighbors
                int uMin=max(x-1,0);
                int vMin=max(y-1,0);
                int uMax=min(x+2,width);
                int vMax=min(y+2,height);
                for(int v=vMin;v<vMax;v++){
                    for(int u=uMin;u<uMax;u++){
                        int j=u+v*width;
                        if(labels[i]!=labels[j]) superpixels[labelSp].addNeighbor(int(labels[j]));
                    }
                }

            }
        }
    }

    int interactiveSegmentation(uint32_t* seeds,uint32_t * segmentation, int height,int width){
        if(height!=this->height|| width!=this->width){
            return -1;
        }
        computeDataTerm(seeds,height,width);
        createModel();

        ALPHA_EXP inferenceAlgo(model);

        inferenceAlgo.infer();
        std::vector<LabelType> labeling(model.numberOfVariables());
        inferenceAlgo.arg(labeling);


        int maxClass=0;
        for(int spIdx=0;spIdx<superpixels.size();spIdx++){
            RGB_Color color=svm->getClassColor(labeling[spIdx]);
            uint32_t pixel=0xFFFFFFFF;
            pixel=pixel<<8;
            pixel=pixel|color.red();
            pixel=pixel<<8;
            pixel=pixel|color.green();
            pixel=pixel<<8;
            pixel=pixel|color.blue();
            for(int j=0;j<superpixels[spIdx].getPixelsCoordinates().size();j++) {
                Point p = superpixels[spIdx].getPixelsCoordinates()[j];
                int i = p.x + p.y * width;
                segmentation[i]=pixel;
            }
        }

        return maxClass;


    }

    int computeNbSeeds(uint32_t* seeds,int width,int height){

        vector<RGB_Color> classColors = SeedsTools::extractClassColors(seeds,width,height);

        return classColors.size();

    }

};



static SCIS algo;


JNIEXPORT jint JNICALL
Java_fr_mathieu_berengere_safdemo_InteractiveSegmentationActivity_interactiveSegmentation(JNIEnv * env,
                                                                                       jobject obj,
                                                                                       jobject seedsBitmap,
                                                                                       jobject segmentationBitmap) {
    AndroidBitmapInfo info;
    uint32_t *seedsPixels;
    int retSeeds,retSeg;

    //get seeds bitmap information
    retSeeds = AndroidBitmap_getInfo(env, seedsBitmap, &info);
    if (retSeeds < 0) {
        LOGE("AndroidBitmap_getInfo() failed ! error=%d", retSeeds);
        return retSeeds;
    }
    //check seeds bitmap type
    if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        LOGE("Bitmap format is not RGBA_8888 !");
        return WRONG_BITMAP_TYPE;
    }
    //load seedsPixels
    void *bitmapSeedPixels;
    retSeeds = AndroidBitmap_lockPixels(env, seedsBitmap, &bitmapSeedPixels);
    seedsPixels = (uint32_t *) bitmapSeedPixels;
    if (retSeeds < 0) {
        LOGE("AndroidBitmap_lockPixels() failed ! error=%d", retSeeds);
        return retSeeds;
    }

    //get segmentation bitmap information
    retSeg = AndroidBitmap_getInfo(env, segmentationBitmap, &info);
    if (retSeg < 0) {
        LOGE("AndroidBitmap_getInfo() failed ! error=%d", retSeg);
        return retSeg;
    }
    //check bitmap type
    if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        LOGE("Bitmap format is not RGBA_8888 !");
        return WRONG_BITMAP_TYPE;
    }
    //load segmentation pixels
    void *bitmapSegPixels;
    retSeg = AndroidBitmap_lockPixels(env, segmentationBitmap, &bitmapSegPixels);
    uint32_t * segmentationPixels = (uint32_t *) bitmapSegPixels;
    if (retSeg < 0) {
        LOGE("AndroidBitmap_lockPixels() failed ! error=%d", retSeg);
        return retSeg;
    }

    int res=algo.interactiveSegmentation(seedsPixels,segmentationPixels,info.height,info.width);
    AndroidBitmap_unlockPixels(env,seedsBitmap);
    AndroidBitmap_unlockPixels(env,segmentationBitmap);



    return res;
}

JNIEXPORT jint JNICALL
Java_fr_mathieu_berengere_safdemo_InteractiveSegmentationActivity_checkNumberOfClasses(JNIEnv * env, jobject obj, jobject bitmap) {
    AndroidBitmapInfo info;
    uint32_t *pixels;
    int ret;

    //get bitmap information
    ret = AndroidBitmap_getInfo(env, bitmap, &info);
    if (ret < 0) {
        LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
        return ret;
    }


    //check bitmap type
    if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        LOGE("Bitmap format is not RGBA_8888 !");
        return WRONG_BITMAP_TYPE;
    }

    //load pixels
    void *bitmapPixels;
    ret = AndroidBitmap_lockPixels(env, bitmap, &bitmapPixels);
    pixels = (uint32_t *) bitmapPixels;
    if (ret < 0) {
        LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
        return ret;
    }
    AndroidBitmap_unlockPixels(env,bitmap);

    return algo.computeNbSeeds(pixels,info.width,info.height);
}
JNIEXPORT jint JNICALL
Java_fr_mathieu_berengere_safdemo_InteractiveSegmentationActivity_oversegmentWithSLIC(JNIEnv * env, jobject obj, jobject bitmap) {


    AndroidBitmapInfo info;
    uint32_t *pixels;
    int ret;

    //get bitmap information
    ret = AndroidBitmap_getInfo(env, bitmap, &info);
    if (ret < 0) {
        LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
        return ret;
    }


    //check bitmap type
    if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        LOGE("Bitmap format is not RGBA_8888 !");
        return WRONG_BITMAP_TYPE;
    }

    //load pixels
    void *bitmapPixels;
    ret = AndroidBitmap_lockPixels(env, bitmap, &bitmapPixels);
    pixels = (uint32_t *) bitmapPixels;
    if (ret < 0) {
        LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
        return ret;
    }

    algo.oversegmentWithSlic(pixels, info.height, info.width);

    AndroidBitmap_unlockPixels(env, bitmap);

    return algo.nbSuperpixels();
}
}