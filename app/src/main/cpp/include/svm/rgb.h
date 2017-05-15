/**
 * @file rgb.h
 * @brief to store and read an color in RGB color space
 * @author Berengere MATHIEU
 * @version 1.0
 */

#ifndef RGB_H
#define RGB_H

#include <iostream>

using namespace std;
/**
 * @class RGB_Color
 * @brief Colour in red, green, blue color space
 */
class RGB_Color{
private:    
    unsigned char red_;/**< red value*/
    unsigned char green_;/**< green value*/
    unsigned char blue_;/**< blue value*/
public:
    /**
     * @brief create rgb color
     * @param{in} red red value
     * @param{in} green green value
     * @param{in} blue blue value
     */
    RGB_Color(const unsigned char red=0,const unsigned char green=0,const unsigned char blue=0);

    /**
     * @brief create rgb color
     * @param{in} other color which must by copied
     */
    RGB_Color(const RGB_Color& other);

    /**
     * @brief operator =
     * @param{in} other color which must by copied
     * @return{in} update the current color RGB values with RGB values of color "other"
     */
    RGB_Color operator =(const RGB_Color& other);

    /**
     * @brief delete delete current color
    */
    ~RGB_Color(){ }


    /**
     * @brief red getter for red value
     * @return red value
     */
    unsigned char red() const;
    /**
     * @brief green getter for green value
     * @return green value
     */
    unsigned char green() const;

    /**
     * @brief blue getter for blue value
     * @return blue value
     */
    unsigned char blue() const;

    /**
     * @brief operator == compare RGB values of current color with an other color
     * @param other an other color
     * @return true if the two colors have the same RGB values
     */
    bool operator==(const RGB_Color& other) const;

    /**
     * @brief operator != compare RGB values of current color with an other color
     * @param other an other color
     * @return false if the two colors have the same RGB values
     */
    bool operator!=(const RGB_Color& other) const;
    /**
     * @brief print print red, green and blue values of the current color
     */
    void print() const;

};

#endif // RGB_H
