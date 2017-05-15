
/**
 * @file tools.cpp
 * @brief  Some helpful tools to handle seeds given by user
 * @author Berengere Mathieu
 * @version 1.0
 */

#include "svm/seedsTools.h"

vector<RGB_Color> SeedsTools::extractClassColors(uint32_t* seeds,int height, int width){


    vector<RGB_Color> classColors;
    RGB_Color pixelColor;
    RGB_Color black(0,0,0);
    bool knownClass;
    for(int r=0;r<height;++r){
        for(int c=0;c<width;++c){
            int i=c+r*width;
            uint32_t pixel=(uint32_t )seeds[i];
            int red =   (pixel & 0x00ff0000) >> 16;
            int green = (pixel & 0x0000ff00) >> 8;
            int blue =  (pixel & 0x000000ff) >> 0;
            pixelColor=RGB_Color(red,green,blue);
            if(pixelColor!=black){
                knownClass=false;
                for(int i=0;i<classColors.size();++i){
                    if(pixelColor==classColors[i]){
                        knownClass=true;
                        break;
                    }
                }
                if(!knownClass) classColors.push_back(pixelColor);
            }
        }
    }


    return classColors;

}
