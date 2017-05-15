#include "oversegmentation/superpixel.h"

#include <stdexcept>

/**
     * @brief SuperpixelFeatures create a superpixel descriptor
     * @param c1[i] first average color channel
     * @param c2[i] seconde average color channel
     * @param c3[i] third average color channel
     * @param r[i] center of mass ordinate
     * @param c[i] center of mass abscissa
     * @param nbPixels number of pixels belonging to the superpixel
     */
SimpleSuperpixel::SimpleSuperpixel():redAtt(0),greenAtt(0),blueAtt(0),ordinate(0),abscissa(0),classLabel(-1){};

void SimpleSuperpixel::addPixel(double red,double green,double blue,double x, double y){
    this->redAtt+=red;
    this->greenAtt+=green;
    this->blueAtt+=blue;
    this->abscissa+=x;
    this->ordinate+=y;
    pixelsCoordinates.push_back(Point(x,y));
}

void SimpleSuperpixel::addNeighbor(int label){
    neighbors.insert(label);
}

const vector<Point>& SimpleSuperpixel::getPixelsCoordinates() const{
    return this->pixelsCoordinates;
}

int SimpleSuperpixel::getNbNeighbors() const{
    return neighbors.size();
}

int SimpleSuperpixel::getNeighbor(int idx) const{
    if(idx>=neighbors.size()) throw runtime_error("invalid neighbor index");
    set<int>::iterator it=neighbors.begin();
    for(int i=0;i<idx;i++){
        it++;
    }
    return *it;
}

double SimpleSuperpixel::getNormalizedRed() const {
    return redAtt/(pixelsCoordinates.size()*255.);
}

double SimpleSuperpixel::getNormalizedGreen() const{
    return greenAtt/(pixelsCoordinates.size()*255.);
}

double SimpleSuperpixel::getNormalizedBlue() const{
    return blueAtt/(pixelsCoordinates.size()*255.);
}


double SimpleSuperpixel::getAbscissa() const{
    return abscissa/double(pixelsCoordinates.size());
}


double SimpleSuperpixel::getOrdinate() const{
    return ordinate/double(pixelsCoordinates.size());
}



double SimpleSuperpixel::dist(const SimpleSuperpixel& other){
    double dist=pow(getNormalizedRed()-other.getNormalizedRed(),2);
    dist+=pow(getNormalizedGreen()-other.getNormalizedGreen(),2);
    dist+=pow(getNormalizedBlue()-other.getNormalizedBlue(),2);
    dist=sqrt(dist);
    return dist;
}

/**
     * @brief print print descriptor contents
     */
void SimpleSuperpixel::print() const {
    cout << "( " << getNormalizedRed() << " , " << getNormalizedGreen() << " , " << getNormalizedBlue() << " , " << getOrdinate() << " , " << getAbscissa() << " )" << endl;
}


LBPSuperpixel::LBPSuperpixel():redAtt(0),greenAtt(0),blueAtt(0),ordinate(0),abscissa(0),classLabel(-1){
    lbpsHist.assign(64,-1);
};

void LBPSuperpixel::addPixel(double red, double green, double blue, double x, double y, double lbp){
    this->redAtt+=red;
    this->greenAtt+=green;
    this->blueAtt+=blue;
    this->abscissa+=x;
    this->ordinate+=y;
    this->lbpsHist[lbp]++;
    pixelsCoordinates.push_back(Point(x,y));
}

void LBPSuperpixel::addNeighbor(int label){
    neighbors.insert(label);
}

const vector<Point>& LBPSuperpixel::getPixelsCoordinates() const{
    return this->pixelsCoordinates;
}

int LBPSuperpixel::getNbNeighbors() const{
    return neighbors.size();
}

int LBPSuperpixel::getNeighbor(int idx) const{
    if(idx>=neighbors.size()) throw runtime_error("invalid neighbor index");
    set<int>::iterator it=neighbors.begin();
    for(int i=0;i<idx;i++){
        it++;
    }
    return *it;
}

double LBPSuperpixel::getNormalizedRed() const {
    return redAtt/(pixelsCoordinates.size()*255.);
}

double LBPSuperpixel::getNormalizedGreen() const{
    return greenAtt/(pixelsCoordinates.size()*255.);
}

double LBPSuperpixel::getNormalizedBlue() const{
    return blueAtt/(pixelsCoordinates.size()*255.);
}

vector<double> LBPSuperpixel::getNormalizedHist() const{
    vector<double> normHist;
    for(int i=0;i<lbpsHist.size();i++){
        normHist.push_back(lbpsHist[i]/double(pixelsCoordinates.size()));
    }
    return normHist;
}


double LBPSuperpixel::getAbscissa() const{
    return abscissa/double(pixelsCoordinates.size());
}


double LBPSuperpixel::getOrdinate() const{
    return ordinate/double(pixelsCoordinates.size());
}

double LBPSuperpixel::dist(const LBPSuperpixel& other){
    double dist=pow(getNormalizedRed()-other.getNormalizedRed(),2);
    dist+=pow(getNormalizedGreen()-other.getNormalizedGreen(),2);
    dist+=pow(getNormalizedBlue()-other.getNormalizedBlue(),2);
    dist=sqrt(dist);
    return dist;
}



/**
     * @brief print print descriptor contents
     */
void LBPSuperpixel::print() const {
    cout << "( " << getNormalizedRed() << " , " << getNormalizedGreen() << " , " << getNormalizedBlue() << " , " << getOrdinate() << " , " << getAbscissa() << " )" << endl;
}

