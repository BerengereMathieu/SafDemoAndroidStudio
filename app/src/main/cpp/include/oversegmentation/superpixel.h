#ifndef SUPERPIXELSFEATURES_H
#define SUPERPIXELSFEATURES_H

#include <iostream>
#include <vector>
#include <set>
#include <cmath>
using namespace std;

struct Point{
    double x;
    double y;
    Point(double x=0,double y=0):x(x),y(y){};
};

/**
 * @class SuperpixelFeatures
 * @brief Superpixel descriptor
 *
 * A superpixel is describe one or several of this attributs :
 *  - its average color
 *  - its center of mass coordinates
 *  - its number of pixels
 *  - its class
 */
struct SimpleSuperpixel {
private:
    double redAtt;/**< first average color channel*/
    double greenAtt;/**< second average color channel*/
    double blueAtt;/**< third average color channel*/
    double ordinate;/**< center of mass ordinate */
    double abscissa;/**< center of mass abscissa */
    vector<Point> pixelsCoordinates;
    set<int> neighbors;
public:
    int classLabel;/**< class label of the superpixel*/

    /**
     * @brief SuperpixelFeatures create a superpixel descriptor
     * @param c1[i] first average color channel
     * @param c2[i] seconde average color channel
     * @param c3[i] third average color channel
     * @param r[i] center of mass ordinate
     * @param c[i] center of mass abscissa
     * @param nbPixels number of pixels belonging to the superpixel
     */
    SimpleSuperpixel();

    void addPixel(double red,double green,double blue,double x, double y);

    void addNeighbor(int label);
    const vector<Point>& getPixelsCoordinates() const;
    int getNbNeighbors() const;

    int getNeighbor(int idx)const ;

    /**
     * @brief getNormalizedRed
     * @return normalized red value
     */
    double getNormalizedRed() const;

    /**
     * @brief getNormalizedGreen
     * @return normalized green value
     */
    double getNormalizedGreen() const;

    /**
     * @brief getNormalizedBlue
     * @return normalized blue value
     */
    double getNormalizedBlue() const;

    double getOrdinate() const;

    double getAbscissa() const;

    double dist(const SimpleSuperpixel& other);

    /**
     * @brief print print descriptor contents
     */
    void print() const ;
};


struct LBPSuperpixel {
private:
    double redAtt;/**< first average color channel*/
    double greenAtt;/**< second average color channel*/
    double blueAtt;/**< third average color channel*/
    double ordinate;/**< center of mass ordinate */
    double abscissa;/**< center of mass abscissa */
    vector<double> lbpsHist;/**< lbps histogram */
    vector<Point> pixelsCoordinates;
    set<int> neighbors;
public:
    int classLabel;/**< class label of the superpixel*/

    /**
     * @brief SuperpixelFeatures create a superpixel descriptor
     * @param c1[i] first average color channel
     * @param c2[i] seconde average color channel
     * @param c3[i] third average color channel
     * @param r[i] center of mass ordinate
     * @param c[i] center of mass abscissa
     * @param nbPixels number of pixels belonging to the superpixel
     */
    LBPSuperpixel();

    void addPixel(double red,double green,double blue,double x, double y,double lbp);

    void addNeighbor(int label);
    const vector<Point>& getPixelsCoordinates() const;
    int getNbNeighbors() const;

    int getNeighbor(int idx)const ;

    /**
     * @brief getNormalizedRed
     * @return normalized red value
     */
    double getNormalizedRed() const;

    /**
     * @brief getNormalizedGreen
     * @return normalized green value
     */
    double getNormalizedGreen() const;

    /**
     * @brief getNormalizedBlue
     * @return normalized blue value
     */
    double getNormalizedBlue() const;

    vector<double> getNormalizedHist() const;

    double getOrdinate() const;

    double getAbscissa() const;

    double dist(const LBPSuperpixel &other);

    /**
     * @brief print print descriptor contents
     */
    void print() const ;
};




#endif // SUPERPIXELSFEATURES_H
