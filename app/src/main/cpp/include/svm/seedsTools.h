/**
 * @file tools.h
 * @brief  Some helpful tools to handle seeds given by user
 * @author Berengere Mathieu
 * @version 1.0
 */
#ifndef TOOLS_H
#define TOOLS_H

#include "rgb.h"

#include <vector>

using namespace std;

/**
 * @brief Some helpful tools to handle seeds given by user
 */
class SeedsTools{
public:
    /**
    * @brief Find colors of the different class corresponding to element that user wants to extract
    * Seeds is a black image, with colored strokes, the seeds.
    * Each class has a unique color : for example blue for sky.
    * Black is the color for pixels without label
    * @param seeds[i] seeds given by user as colored strokes.
    * @return all colors present in seeds image with the exception of black (color of unlabeled pixels)
    */
    static vector<RGB_Color> extractClassColors(uint32_t* seeds,int widht,int height);
};

#endif // TOOLS_H
