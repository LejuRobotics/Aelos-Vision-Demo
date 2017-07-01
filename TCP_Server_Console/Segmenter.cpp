#include "Segmenter.h"
#include <QDebug>

Segmenter::Segmenter(int width,int height){
    w = width;
    h = height;
//    memset(colorInfo.channelLUT,0,2*256*sizeof(unsigned int)); 改为在addColor函数里面初始化
}

void Segmenter::setColor(int colorIndex,int uv,int LUTIndex,bool value) {
    if(value)
        colorInfo.channelLUT[uv][LUTIndex] |= (1<<colorIndex);       //set to 1
    else
        if((colorInfo.channelLUT[uv][LUTIndex]>>colorIndex)%2)       //if is 1, set to 0
            colorInfo.channelLUT[uv][LUTIndex] -= (1<<colorIndex);
}

void Segmenter::setRange(int pw, int ph)
{
    w = pw;
    h = ph;
}
void Segmenter::addColor(int rgbMean[3],unsigned char channelRange[2][2]) {
    //添加25行和27行，支持重新识别标记的颜色
    memset(colorInfo.channelLUT,0,2*256*sizeof(unsigned int));
    int colorIndex = colorInfo.counter;
    colorIndex = 0;

    colorInfo.meanColor[colorIndex][0] = rgbMean[0];
    colorInfo.meanColor[colorIndex][1] = rgbMean[1];
    colorInfo.meanColor[colorIndex][2] = rgbMean[2];

    colorInfo.channelRange[colorIndex][0][0] = channelRange[0][0];
    colorInfo.channelRange[colorIndex][0][1] = channelRange[0][1];
    colorInfo.channelRange[colorIndex][1][0] = channelRange[1][0];
    colorInfo.channelRange[colorIndex][1][1] = channelRange[1][1];

    for(int i=channelRange[0][0]; i<channelRange[0][1]+1; i++)
        colorInfo.channelLUT[0][i] |= (1<<colorIndex);
    for(int i=channelRange[1][0]; i<channelRange[1][1]+1; i++)
        colorInfo.channelLUT[1][i] |= (1<<colorIndex);
    colorInfo.counter++;
}
void Segmenter::segment(unsigned char *source, bool mask){
    static Object *tmpObj = NULL;
    static std::stack<int> pixelStack;
    static short int *isVisited = new short int[w*h];

    objList.clear();
    //size_t objCounter = objList.size();
    //for (size_t i = 0; i < objCounter; i++)
    //    delete objList[i];
    memset(isVisited,-1,w*h*sizeof(short int));

    for (int i = 0; i < w*h; i ++) {  //for each big pixel
        if(isVisited[i] == -1){        //-1 means it does not belong to any object yet(including the "not interested" object, which is labeled 0)
            int tmpLabel = getLabel(i); //i*3 inside getColor
            if(tmpLabel != 0){
                tmpObj = new(Object);
                tmpObj->colorID = tmpLabel;
                tmpObj->ID = objList.size();
                tmpObj->maxX = tmpObj->maxY = 0;
                tmpObj->minX = tmpObj->minY = 65535;
                pixelStack.push(i);     //find new object, which color is pixelColor
            }else
                isVisited[i] = 0;	//"not interested" object
        }
        while(pixelStack.empty() == false) {
            int index = pixelStack.top();
            pixelStack.pop();
            if(getLabel(index) == tmpObj->colorID) {   //has same color as current scanning object
                    isVisited[index] = 1;   //label as current object ID
                    //isVisited[index+1] = 1;   //????????????
                    if(((index-w)>=0)&&(isVisited[index-w]==-1)){			//using unsigned may cause BUG here
                        pixelStack.push(index-w);   //Up
                        isVisited[index-w] = 0;
                    }
                    if(((index+w)<w*h)&&(isVisited[index+w]==-1)){  //!!!!!!!!!!!!!!!!!! check range first
                        pixelStack.push(index+w);   //Down
                        isVisited[index+w] = 0;
                    }
                    if(((index-1)%w>=0)&&(isVisited[index-1]==-1)){
                        pixelStack.push(index-1);   //Left pixel
                        isVisited[index-1] = 0;
                    }
                    if(((index+1)%w<w)&&(isVisited[index+1]==-1)){
                        pixelStack.push(index+1);
                        isVisited[index+1] = 0;
                    }
                    /*if(((index-w-1)>=0)&&(isVisited[index-w-1]==-1)){
                        pixelStack.push(index-w-1);
                        isVisited[index-w-1] = 0;
                    }
                    if(((index-w+1)<w*h)&&(isVisited[index-w+1]==-1)){
                        pixelStack.push(index-w+1);
                        isVisited[index-w+1] = 0;
                    }
                    if(((index+w-1)>=0)&&(isVisited[index+w-1]==-1)){
                        pixelStack.push(index+w-1);
                        isVisited[index+w-1] = 0;
                    }
                    if(((index+w+1)<w*h)&&(isVisited[index+w+1]==-1)){
                        pixelStack.push(index+w+1);
                        isVisited[index+w+1] = 0;
                    }*/
                    unsigned int row = index/w;
                    unsigned int col = index%w;
                    tmpObj->minX = (col<tmpObj->minX)?col:tmpObj->minX;
                    tmpObj->maxX = (col>tmpObj->maxX)?col:tmpObj->maxX;
                    tmpObj->minY = (row<tmpObj->minY)?row:tmpObj->minY;
                    tmpObj->maxY = (row>tmpObj->maxY)?row:tmpObj->maxY;
                    tmpObj->pixelCounter++;
                    if(mask){
                        int colorIndex = tmpObj->colorID-1;
                        source[3*index+1] = colorInfo.meanColor[colorIndex][1];
                        source[3*index+2] = colorInfo.meanColor[colorIndex][2];
                    }
            }else{
                isVisited[index]= -1;
            }
        }//search of one object end, make some mark
        if(tmpObj){
            if((tmpObj->pixelCounter)>=sizeThreshold){
                tmpObj->centerX = ((tmpObj->minX)+(tmpObj->maxX))/2;
                tmpObj->centerY = ((tmpObj->minY)+(tmpObj->maxY))/2;
                objList.push_back(tmpObj);
            }else
                delete tmpObj;
        }
        tmpObj = NULL;
    }
}
