/**
 * @file rgb.cpp
 * @brief to store and read an color in RGB color space
 * @author Berengere MATHIEU
 * @version 1.0
 */

#include "svm/rgb.h"

#include <iostream>

RGB_Color::RGB_Color(const unsigned char red, const unsigned char green, const unsigned char blue) : red_(red), green_(green), blue_(blue){

}

RGB_Color::RGB_Color(const RGB_Color& other) :red_(other.red()), green_(other.green()), blue_(other.blue()) {

}

RGB_Color RGB_Color::operator =(const RGB_Color& other) {

    if(this != &other){
        this->red_=other.red();
        this->green_=other.green();
        this->blue_=other.blue();
    }

    return *this;
}

unsigned char RGB_Color::red() const{
    return red_;
}

unsigned char RGB_Color::green() const{
    return green_;
}

unsigned char RGB_Color::blue() const{
    return blue_;
}


bool RGB_Color::operator==(const RGB_Color& other) const{
    return (this->red_==other.red_) && (this->green_==other.green_) && (this->blue_==other.blue_) ;
}

bool RGB_Color::operator!=(const RGB_Color& other) const{
    return !((*this)==(other));
}

void RGB_Color::print() const{
    std::cout << "( " << int(this->red_) << " , " << int(this->green_) << " , " << int(this->blue_) << " )" << std::endl;
}
