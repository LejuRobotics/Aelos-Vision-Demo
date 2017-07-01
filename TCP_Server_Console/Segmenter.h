#include <stack>
#include <vector>
#include <string.h>
#include <iostream>

#ifndef SEGMENTER_H
#define SEGMENTER_H

#define getLabel(index) (colorInfo.channelLUT[0][source[3*index+1]]&colorInfo.channelLUT[1][source[3*index+2]])

struct Object{
    unsigned char ID;
    unsigned char colorID;
    unsigned int centerX,centerY;
    unsigned int minX,maxX,minY,maxY;
    unsigned int pixelCounter = 0;
    //std::vector<unsigned int> pixelIndexList;
};
struct ColorInfo{
    char counter = 0;
    unsigned int channelLUT[2][256];   //able to identify 32 different colors
    unsigned char meanColor[32][3]; //mean rgb color of each group
    unsigned char channelRange[32][2][2];
};
class Segmenter
{
public:
    ColorInfo colorInfo;
    std::vector<Object*> objList;
    unsigned int sizeThreshold = 100;
    Segmenter(int width=640, int height=480);
    void addColor(int rgbMean[3],unsigned char channelRange[2][2]);
    void segment(unsigned char *source, bool mask);
    void setColor(int colorIndex,int uv,int LUTIndex,bool value);
    void setRange(int pw, int ph);
private:
    int w;
    int h;
};

#endif // SEGMENTER_H
