/**
 * @file       Segmenter.h
 * @version    1.0
 * @date       2017年07月08日
 * @author     C_Are
 * @copyright  Leju
 *
 * @brief      识别颜色类(现在已放到DiscernColor类中实现)，Segmenter类的h文件
 * @details    目前已经将Segmenter类的两个函数放到DiscernColor类中，计算和识别颜色，现在
 *             全部在DiscernColor类的线程中执行
 */

#include <stack>
#include <vector>
#include <string.h>
#include <iostream>

#ifndef SEGMENTER_H
#define SEGMENTER_H

#define getLabel(index) (colorInfo.channelLUT[0][source[3*index+1]]&colorInfo.channelLUT[1][source[3*index+2]])

/**
 * @brief 标记框
 */

struct Object{
    unsigned char ID;
    unsigned char colorID;
    unsigned int centerX,centerY;
    unsigned int minX,maxX,minY,maxY;
    unsigned int pixelCounter = 0;
    //std::vector<unsigned int> pixelIndexList;
};

/**
 * @brief 记录标记目标的平均色和要匹配的颜色范围
 */

struct ColorInfo{
    char counter = 0;
    unsigned int channelLUT[2][256];   //able to identify 32 different colors
    unsigned char meanColor[32][3]; //mean rgb color of each group
    unsigned char channelRange[32][2][2];
};

/**
 * @class     Segmenter
 * @brief     通过分离图片YUV,对图片进行颜色识别
 */

class Segmenter
{
public:
    ColorInfo colorInfo;
    std::vector<Object*> objList;
    unsigned int sizeThreshold = 100;
    Segmenter(int width=640, int height=480);
    void addColor(int rgbMean[3],unsigned char channelRange[2][2]);
    void segment(unsigned char *source, bool mask);
//    void setColor(int colorIndex,int uv,int LUTIndex,bool value);
    void setRange(int pw, int ph);
private:
    int w;   /**< 图片宽度 */
    int h;   /**< 图片高度 */
};

#endif // SEGMENTER_H
