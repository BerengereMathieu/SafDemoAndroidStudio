
#include "svm/svmWrapper.h"
#include "svm/svm.h"
#include "svm/seedsTools.h"

SvmWithColorShapeFeatures::SvmWithColorShapeFeatures(uint32_t* seeds, int width, int height, const vector<SimpleSuperpixel> &superpixels) : AbstractSvm(seeds,width,height,superpixels){
    nbFeatures=5;
}

void SvmWithColorShapeFeatures::train(){
    struct svm_parameter param;		// set by parse_command_line
    //Set parameters
    // default values
    param.svm_type = C_SVC;
    //    if(this->colorspace.getColorSpace()==ColorSpace::HSI_CS){
    //        param.kernel_type = HSI_RBF;
    //    }else{
    param.kernel_type = RBF;
    //}

    //SVM parameters : default values used in libsvm implementation
    param.degree = 3;
    param.gamma =4.0;	// 1/num_features
    param.coef0 = 0;
    param.nu = 0.5;
    param.cache_size = 100;
    param.C = 4.0;
    param.eps = 1e-3;
    param.p = 0.1;
    param.shrinking = 0;
    param.probability = 1;
    param.nr_weight = 0;
    param.weight_label = NULL;
    param.weight = NULL;
    //Initialize prob with training data
    struct svm_problem prob;
    prob.l=nbSpsForTraining ;
    //to store class labels
    prob.y = (double*) malloc(sizeof(double)*prob.l);
    //features access
    prob.x=(struct svm_node**) malloc(sizeof(struct svm_node*)*prob.l);
    //to store all features and their index
    //if a feature is equal to 0, it is not stored
    //index of the feature is used to know each value corresponding to the same feature
    struct svm_node *x_space;
    x_space=(struct svm_node*) malloc(sizeof(struct svm_node)*prob.l*(nbFeatures+1));

    {
        int j=0;//index of the svm_node for store the next feature
        int i=0;
        for(int index=0;index<superpixels.size();index++){
            if(superpixelsClasses[index]>=0){
                int k=0;//current feature index
                //store class label of superpixel
                prob.y[i]=superpixelsClasses[index];
                //store features of superpixel
                prob.x[i]=&(x_space[j]);
                //if a feature is equal to 0, it is not stored
                if(superpixels[index].getNormalizedRed() !=0 ){ // first color coordinate
                    x_space[j].index=k+1;
                    x_space[j].value=superpixels[index].getNormalizedRed();
                    ++j;
                }
                k++;
                if(superpixels[index].getNormalizedGreen() !=0 ){ // second color coordinate
                    x_space[j].index=k+1;
                    x_space[j].value=superpixels[index].getNormalizedGreen();
                    ++j;
                }
                k++;
                if(superpixels[index].getNormalizedBlue() !=0 ){ // third color coordinate
                    x_space[j].index=k+1;
                    x_space[j].value=superpixels[index].getNormalizedBlue();
                    ++j;
                }
                k++;
                if(superpixels[index].getOrdinate()/double(height-1) !=0 ){// ordinate
                    x_space[j].index=k+1;
                    x_space[j].value=superpixels[index].getOrdinate()/double(height-1);
                    ++j;
                }
                k++;
                if(superpixels[index].getAbscissa()/double(width-1) !=0 ){// abscissa
                    x_space[j].index=k+1;
                    x_space[j].value=superpixels[index].getAbscissa()/double(width-1);
                    ++j;
                }
                x_space[j].index=-1;
                ++j;
                ++i;
            }

        }
    }

    //train SVM using features of superpixels
    model = svm_train(&prob,&param);
}


void SvmWithColorShapeFeatures::classify(){
    struct svm_node x[nbFeatures+1];
    for(int index=0;index<superpixels.size();++index){//for each superpixel
        int j=0;//svm_node index used to store features of the current superpixel
        int k=0;//feature index

        if(superpixels[index].getNormalizedRed() !=0 ){ // first color coordinate
            x[j].index=k+1;
            x[j].value=superpixels[index].getNormalizedRed();
            ++j;
        }
        k++;
        if(superpixels[index].getNormalizedGreen() !=0 ){ // second color coordinate
            x[j].index=k+1;
            x[j].value=superpixels[index].getNormalizedGreen();
            ++j;
        }
        k++;
        if(superpixels[index].getNormalizedBlue() !=0 ){ // third color coordinate
            x[j].index=k+1;
            x[j].value=superpixels[index].getNormalizedBlue();
            ++j;
        }
        k++;
        if(superpixels[index].getOrdinate()/double(height-1) !=0 ){// ordinate
            x[j].index=k+1;
            x[j].value=superpixels[index].getOrdinate()/double(height-1);
            ++j;
        }
        k++;
        if(superpixels[index].getAbscissa()/double(width-1) !=0 ){// abscissa
            x[j].index=k+1;
            x[j].value=superpixels[index].getAbscissa()/double(width-1);
            ++j;
        }
        x[j].index=-1;//end of features
        //get probabilities for each class for each superpixel

        vector<double> probabilities;
        probabilities.assign(classColors.size(),0);
        //test if superpixel is a seed
        if(superpixelsClasses[index]<0){

            double probasSVM[classColors.size()];
            //predict c‚lass of superpixel
            int predict_label = svm_predict_probability(model,x,probasSVM);
            //probabilities are not in the same order than class label
            for(int p=0;p<probabilities.size();p++){
                //reorder
                probabilities[model->label[p]]=probasSVM[p];

            }
            //store class label
            superpixelsClasses[index]=predict_label;
        }else{
            for(int p=0;p<probabilities.size();p++){
                probabilities[p]=p==superpixelsClasses[index]?1:0;
            }
        }

        superpixelsProbabilities.push_back(probabilities);


    }
}





AbstractSvm::AbstractSvm(uint32_t* seeds,int width, int height, const vector<SimpleSuperpixel> &superpixels) : superpixels(superpixels){
    superpixelsClasses.assign(superpixels.size(),-1);
    classColors = SeedsTools::extractClassColors(seeds,width,height);
    nbSpsForTraining=0;

    for(int i=0;i<superpixels.size();i++){
        int spClass=-1;
        for(int j=0;j<superpixels[i].getPixelsCoordinates().size();j++){
            Point p=superpixels[i].getPixelsCoordinates()[j];
            int i=p.x+p.y*width;
            uint32_t pixel=(uint32_t )seeds[i];
            int red =   (pixel & 0x00ff0000) >> 16;
            int green = (pixel & 0x0000ff00) >> 8;
            int blue =  (pixel & 0x000000ff) >> 0;
            RGB_Color pixelColor(red, green, blue);
            for (int k = 0; k < classColors.size(); ++k) {
                if (pixelColor == classColors[k]) {
                    if (spClass < 0) {
                        spClass = k;
                    } else {
                        if (spClass != k) spClass = -2;
                    }
                }
                if (spClass == -2) {
                    //several classes given to superpixel
                    break;
                }
            }

            if (spClass == -2) {
                //several classes given to superpixel
                break;
            }
        }
        superpixelsClasses[i]=spClass;
        if(spClass>=0){
            nbSpsForTraining++;
        }

    }

}

int AbstractSvm::getNbTrainingData() const{
    return nbSpsForTraining;
}

const vector<double>& AbstractSvm::getProbas(int i) const{
    return superpixelsProbabilities[i];
}

int AbstractSvm::getClass(int i) const{
    return superpixelsClasses[i];
}

int AbstractSvm::nbClasses() const{
    return classColors.size();
}

RGB_Color AbstractSvm::getClassColor(int i) const{
    return classColors[i];
}
