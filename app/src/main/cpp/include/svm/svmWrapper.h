
#ifndef SVM_WRAPPER_H
#define SVM_WRAPPER_H
#include "oversegmentation/superpixel.h"
#include "rgb.h"
#include <vector>

using namespace std;
class AbstractSvm{
protected:
    const vector<SimpleSuperpixel>& superpixels;
    int nbSpsForTraining;
    struct svm_model * model;
    vector< vector<double> > superpixelsProbabilities;
    vector<int> superpixelsClasses;
    vector<RGB_Color> classColors ;
    int width;
    int height;
    int nbFeatures;
public:
    AbstractSvm(uint32_t* seeds,int width, int height,const vector<SimpleSuperpixel>& superpixels);
    virtual ~AbstractSvm(){};
    virtual void train()=0;
    virtual void classify()=0;
    const vector<double> &getProbas(int i) const;
    int getClass(int i) const;
    int getNbTrainingData()const;
    int nbClasses() const;
    RGB_Color getClassColor(int i) const;
};




class SvmWithColorShapeFeatures : public AbstractSvm{
public:
    SvmWithColorShapeFeatures(uint32_t* seeds,int width,int height, const vector<SimpleSuperpixel>& superpixels);
    virtual void train();
    virtual void classify();

};


#endif // TOOLS_H
