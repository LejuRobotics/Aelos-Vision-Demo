/**
 * @file       DiscernColor.cpp
 * @version    1.0
 * @date       2017年07月12日
 * @author     C_Are
 * @copyright  Leju
 *
 * @brief      当前识别颜色的类，DiscernColor类的cpp文件
 */

#include "DiscernColor.h"

int g_color_channel_Y = 120;

/**
 * @brief     DiscernColor类的构造函数，初始化
 */

DiscernColor::DiscernColor(QObject *parent) : QThread(parent)
{
    m_stopped = false;
    m_selected = false;
    isReady = false;
    curPosition = Unknown;
    actionMode = 0;
    actionStatus = Initial;
    m_left_command = 3;
    m_right_command = 4;
    m_bStopEnable = false;
    m_stop_size = 0;
    isArrive = false;
    m_curSize = 0;
    m_action_order = 1;
    w = g_frame_width;
    h = g_frame_height;
    sizeThreshold = 100;
}

/**
 * @brief     DiscernColor类的析构函数
 * @details   关闭线程,如果等待时间超过3s强制关闭
 */

DiscernColor::~DiscernColor()
{
    m_stopped = true;
    this->quit();
    if (!this->wait(3000))
    {
        this->terminate();
    }
}

/**
 * @brief     设置机器人是否可以在目标附近自动停止
 * @param     flag 为true，记录停止需要停止位置的标记框大小，为false,则恢复初始状态
 */

void DiscernColor::setStopEnable(bool flag)
{
    m_bStopEnable = flag;
    if (flag)
    {
        m_stop_size = currentMark.width()*currentMark.height();
        qDebug("record size: %d", m_stop_size);
    }
    else
    {
        m_stop_size = 0;
        isArrive = false;
        actionStatus = Initial;
    }
}

/**
 * @brief     设置机器人是自动还是手动执行动作
 * @param     mode 为0手动，否则自动
 */

void DiscernColor::setActionMode(int mode)
{
    if (mode)
    {
        actionMode = 1;
        isReady = true;
    }
    else
    {
        actionMode = 0;
        isReady = false;
    }
}

/**
 * @brief     设置机器人动作状态，当收到串口值确认动作完成则通过该函数设置动作完成状态
 * @param     status 动作状态
 */

void DiscernColor::setActionStatus(DiscernColor::ActionStatus status)
{
    actionStatus = status;
}

/**
 * @brief     执行完一个动作，会添加一定时间的延迟，延迟时间到会执行该函数，让机器人可以执行下一次动作
 */

void DiscernColor::setActionReady()
{
    isReady = true;
}

/**
 * @brief     接收每一帧图片放到线程中处理
 * @param     iamge 图片
 * @details   每次接收一张图片会用锁锁住，并且复制一张图片，然后放到线程中处理，确保处理好每一帧图片
 */

void DiscernColor::readFrame(QImage *image)
{
    m_mutex.lock();
    m_image = image->copy();
    m_image_queue.append(m_image);
    m_mutex.unlock();
    if (!this->isRunning())
    {
        this->start();  //启动线程，会进入run函数
    }
}

/**
 * @brief     在线程中计算和识别颜色位置，如果是自动模式下，会控制机器人的执行动作
 */

void DiscernColor::run()
{
    while (!m_stopped && !m_image_queue.isEmpty())
    {
        QImage nextImage = m_image_queue.dequeue();
        Mat frame;
        //把QImage转为Mat，格式为RGB
        frame = Mat(nextImage.height(), nextImage.width(), CV_8UC3, (void*)nextImage.constBits(), nextImage.bytesPerLine());
//        cvtColor(frame,frame,CV_RGB2BGR);

        //[2] 处理标记的目标颜色
        if(m_selected)
        {
            int rgbMean[3];
            unsigned char uvRange[2][2];

            cv::Mat bgrROI;
            bgrROI = frame(cv::Rect(m_select_rect.x(), m_select_rect.y(), m_select_rect.width(), m_select_rect.height()));
            cv::Scalar bgrMean = cv::mean(bgrROI); //计算选中区域的RGB平均值
            rgbMean[2] = bgrMean[0];
            rgbMean[1] = bgrMean[1];
            rgbMean[0] = bgrMean[2];

            cv::Mat yuvROI;
//            cv::cvtColor(bgrROI, yuvROI,CV_BGR2YUV);
            cv::cvtColor(bgrROI, yuvROI,CV_RGB2YUV);
            std::vector<cv::Mat> yuvROISplit;
            cv::split(yuvROI, yuvROISplit);
            yuvROISplit[1] =yuvROISplit[1].reshape(0,1);
            yuvROISplit[2] = yuvROISplit[2].reshape(0,1);

            int len = yuvROISplit[1].size().width-1;
            unsigned char *pChannel = yuvROISplit[0].data;

            cv::sort(yuvROISplit[1], yuvROISplit[0], CV_SORT_EVERY_ROW + CV_SORT_ASCENDING);
            uvRange[0][0] = pChannel[(int)(0.02*len)];
            uvRange[0][1] = pChannel[(int)(0.98*len)];

            cv::sort(yuvROISplit[2], yuvROISplit[0], CV_SORT_EVERY_ROW + CV_SORT_ASCENDING);
            uvRange[1][0] = pChannel[(int)(0.02*len)];
            uvRange[1][1] = pChannel[(int)(0.98*len)];
            addColor(rgbMean,uvRange);
        }
        //[2]

        //[3] 计算识别物体位置
        if(colorInfo.counter)
        {
            if (m_selected)
            {
                m_selected = false;
            }
            else
            {
                if (!actionMode || actionStatus == Doing || !isReady || isArrive)
                {
                    continue;
                }
            }

//            t.start(); //开始计时
            Mat yuv_mat;
//            cv::cvtColor(frame, yuv_mat,CV_BGR2YUV);
            cv::cvtColor(frame, yuv_mat,CV_RGB2YUV);  //进行颜色识别，先转为YUV格式
            for(int i = 0; i < g_frame_height; i++)
            {
                Vec3b *p = yuv_mat.ptr<Vec3b>(i);  //通过指针遍历每一个像素点
                for(int j = 0; j < g_frame_width; j++)
                {
//                    yuv_mat.at<cv::Vec3b>(i,j)[0] = 128;
                    p[j][0] = g_color_channel_Y;
                }
            }

            segment((unsigned char*)(yuv_mat.data), 1); //核心函数，识别颜色
//            qDebug("Time elapsed: %d ms", t.elapsed());  //打印执行计算识别颜色函数消耗的时间

            QString msg("@Begin:\r\n");
            if (getCurrentMark(objList)) //当识别到物体位置
            {
                msg.append(QString("Mark.Rect=%1,%2,%3,%4\r\n")
                           .arg(currentMark.x())
                           .arg(currentMark.y())
                           .arg(currentMark.width())
                           .arg(currentMark.height()));

                if (m_bStopEnable)
                {
                    if (compareMark())
                    {
                        isArrive = true;
                        msg.append(QString("Reach.Target=%1\r\n").arg(m_curSize));
                    }
                }

                calculateDirection();
            }
            else //没有识别到物体位置
            {
                curPosition = Unknown;
                msg.append("Unrecognized color\r\n");
            }

            msg.append("@End\r\n");
            emit sendInfo(msg);  //发送结果给客户端

            if (isArrive)
                continue;

            if (actionMode)
            {
                actionStatus = Doing;
                isReady = false;
                switch (curPosition)
                {
                case Center: //前进
                    m_action_order = g_forward_command;
                    break;
                case Left:  //左转
                    m_action_order = m_left_command;
                    break;
                case Right: //右转
                    m_action_order = m_right_command;
                    break;
                case Unknown: //没有识别到颜色，持续右转
                    m_action_order = g_right_l_command;
                    break;
                default:
                    break;
                }
                emit directionChanged(m_action_order);
            }
        }
        //[3]

        msleep(1);
    }
}

/**
 * @brief     接收客户端标记的物体进行颜色识别
 * @param     _rect 标记物体的大小位置
 */

void DiscernColor::setSelectRect(const QRect &_rect)
{
    m_selected = true;
    m_select_rect = _rect;
    isReady = false;
}

/**
 * @brief     通过objList中的对象，选择一个作为识别框的位置,有多个的情况下目前选择面积最大的那个
 * @param     objList 存储识别到的颜色的位置的容器
 */

bool DiscernColor::getCurrentMark(const vector<Object *> &objList)
{
    if (objList.empty()) //没有标记框的时候
    {
        return false;
    }

    int rectX,rectY,rectW,rectH;
    if (objList.size() == 1) //当只有一个标记框
    {
        rectX = objList[0]->minX;
        rectY = objList[0]->minY;
        rectW = objList[0]->maxX - rectX;
        rectH = objList[0]->maxY - rectY;
    }
    else //当存在多个标记框
    {
        vector<uint> areaVec;
        for (size_t i = 0; i < objList.size(); i++) {
            //计算标记框的面积
            uint area = (objList[i]->maxX - objList[i]->minX)*(objList[i]->maxY - objList[i]->minY);
            areaVec.push_back(area);
        }

        //从小到大排序
        std::sort(areaVec.begin(), areaVec.end());
        //选择最大的标记框
        rectX = objList[areaVec.size()-1]->minX;
        rectY = objList[areaVec.size()-1]->minY;
        rectW = objList[areaVec.size()-1]->maxX - rectX;
        rectH = objList[areaVec.size()-1]->maxY - rectY;
    }

//    //排除过大和过小的标记框
//    int nMaxSize = 20000;
//    int nMinSize = 30;
//    int curSize = rectW*rectH;
//    qDebug()<< "curSize: "<<curSize;
//    if (curSize < nMinSize || curSize > nMaxSize)
//    {
//        return false;
//    }

    //更新标记框
    currentMark.setX(rectX);
    currentMark.setY(rectY);
    currentMark.setWidth(rectW);
    currentMark.setHeight(rectH);

    return true;
}

/**
 * @brief     根据计算识别到的标记框位置判断物体方向
 * @details   目前我们划分了一个中间区域，由g_horizontal_ratio这个值控制，该值可以通过配置文件更改
 */

void DiscernColor::calculateDirection()
{
    if ( (currentMark.x()+currentMark.width()) <= (g_frame_width*g_horizontal_ratio) )
    {
        curPosition = Left;
    }
    else if ( (currentMark.x()+currentMark.width() - g_frame_width*g_horizontal_ratio) <= (currentMark.width()*g_object_ratio) )
    {
        curPosition = Left;
    }

    else if (currentMark.x() >= g_frame_width*(1.0-g_horizontal_ratio))
    {
        curPosition = Right;
    }
    else if ( (currentMark.x() < g_frame_width*(1.0-g_horizontal_ratio)) &&
             ((currentMark.x()+currentMark.width() - g_frame_width*(1.0-g_horizontal_ratio)) >= (currentMark.width()*g_object_ratio)) )
    {
        curPosition = Right;
    }
    else
    {
        curPosition = Center;
    }

    if (curPosition == Left)
    {
        if (currentMark.x() <= qRound(g_frame_width*g_horizontal_ratio*g_rotation_range))
        {
            m_left_command = g_left_l_command;
        }
        else
        {
            m_left_command = g_left_s_command;
        }
    }

    if (curPosition == Right)
    {
        if (currentMark.x() <= (qRound(g_frame_width*(1-g_horizontal_ratio)) + qRound(g_frame_width*g_horizontal_ratio*(1-g_rotation_range))))
        {
            m_right_command = g_right_s_command;
        }
        else
        {
            m_right_command = g_right_l_command;
        }
    }
}


/**
 * @brief     将识别到的标记框大小与记录的标记框大小进行比较
 * @return    true则判定机器人到达标记位置，false还没有到达标记位置
 */

bool DiscernColor::compareMark()
{
    m_curSize = currentMark.width()*currentMark.height();
    return m_curSize >= m_stop_size;
}

/**
 * @brief     记录标记的颜色信息
 * @param     rgbMean[3]  目标的平均RGB平均色
 * @param     channelRange[2][2] YUV格式的中U,V的范围
 */

void DiscernColor::addColor(int rgbMean[3],unsigned char channelRange[2][2]) {
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

/**
 * @brief     计算识别颜色核心函数
 * @param     source  指向图片的内存
 * @param     mask 为true则让该像素点变成平均色
 */

void DiscernColor::segment(unsigned char *source, bool mask)
{
    static Object *tmpObj = NULL;
    static std::stack<int> pixelStack;
    static short int *isVisited = new short int[w*h];

    objList.clear();
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



