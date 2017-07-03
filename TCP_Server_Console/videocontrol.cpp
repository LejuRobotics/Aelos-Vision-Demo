#include "VideoControl.h"

VideoControl::VideoControl(QObject *parent) : QThread(parent)
{
    isPause = false;
    isSendFrame = true;
    isSelected = false;
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
}

VideoControl::~VideoControl()
{
    isPause = true;
    this->quit();
    if (!this->wait(100))
    {
        this->terminate();
    }
}

void VideoControl::setSelectRect(const QRect &tmp)
{
    isSelected = true;
    m_select_rect = tmp;
    isReady = false;
    if (!this->isRunning())
        this->start();
}

void VideoControl::openUrl(const QString &ip)
{
    qDebug()<<ip<<"is connected !";
    server_ip = ip;
    isPause = false;
    isSendFrame = true;
    if (!this->isRunning())
    {
        this->start();
    }
}

void VideoControl::stop()
{
    isSendFrame = false;
}

void VideoControl::setStopEnable(bool flag)
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
    }
}

void VideoControl::run()
{
    VideoCapture cap;
    cap.open(0);
    if (!cap.isOpened())
    {
        qDebug("cannot find camera !");
        emit sendInfo("cannot find camera !\r\n");
        return;
    }

    cap.set(CV_CAP_PROP_FRAME_WIDTH, g_frame_width);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, g_frame_height);
//    cap.set(CV_CAP_PROP_FPS, 15);

    Segmenter segmenter(g_frame_width, g_frame_height);

    udpSocket = new QUdpSocket;

//    QTime t;

    while(!isPause)
    {
        Mat frame;
        cap >> frame;
        if (frame.empty())
        {
            qDebug("frame is empty !");
            emit sendInfo("cannot read frame !\r\n");
            continue;
        }

        //[1]将摄像头图像发送给客户端
        if (isSendFrame)
        {
            cvtColor(frame,rgb_mat,CV_BGR2RGB);
            rgbImg = new QImage((uchar*) rgb_mat.data, rgb_mat.cols, rgb_mat.rows,
                                rgb_mat.cols*rgb_mat.channels(), QImage::Format_RGB888);

            QBuffer buf(&byte);
            buf.open(QIODevice::WriteOnly);
            rgbImg->save(&buf,"JPEG",g_frame_quality);
            udpSocket->writeDatagram(byte.data(),byte.size(),QHostAddress(server_ip),g_broadcast_port); //向指定ip地址发送图像
            byte.resize(0);
        }
        //[1]

        //[2] 处理选中的物体
        if(isSelected){
            int rgbMean[3];
            unsigned char uvRange[2][2];
            cv::Mat bgrROI, yuvROI;
            std::vector<cv::Mat> yuvROISplit;

            bgrROI = frame(cv::Rect(m_select_rect.x(), m_select_rect.y(), m_select_rect.width(), m_select_rect.height()));
            cv::Scalar bgrMean = cv::mean(bgrROI); //计算选中区域的RGB平均值
            rgbMean[2] = bgrMean[0];
            rgbMean[1] = bgrMean[1];
            rgbMean[0] = bgrMean[2];
//            qDebug("rgbMean: %d,%d,%d",rgbMean[0],rgbMean[1],rgbMean[2]);

            cv::cvtColor(bgrROI, yuvROI,CV_BGR2YUV);
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

            segmenter.addColor(rgbMean,uvRange);
        }
        //[2]

        //[3] 计算识别物体位置
        if(segmenter.colorInfo.counter)
        { 
            if (isSelected)
            {
                isSelected = false;
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
            cv::cvtColor(frame, yuv_mat,CV_BGR2YUV);
            for(int i = 0; i < g_frame_height; i++)
            {
                for(int j = 0; j < g_frame_width; j++)
                {
                    yuv_mat.at<cv::Vec3b>(i,j)[0] = 128;
                }
            }

            segmenter.segment((unsigned char*)(yuv_mat.data), 1); //核心函数，识别颜色
//            qDebug("Time elapsed: %d ms", t.elapsed());  //打印执行计算识别颜色函数消耗的时间

            QString msg("@Begin:\r\n");
            if (getCurrentMark(segmenter.objList)) //当识别到物体位置
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
            emit sendInfo(msg);

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
        else
        {
            //now nothing to do ……
        }
        //[3]

        msleep(1);
    }

    if (isPause)
    {
        cap.release();  //关闭摄像头
    }
}

void VideoControl::setRebotStatus(VideoControl::ActionStatus status)
{
    actionStatus = status;
}

//更新标记框位置，当存在多个标记框的时候，取其中面积最大的
bool VideoControl::getCurrentMark(const vector<Object*> &objList)
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

//计算移动方向
void VideoControl::calculateDirection()
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

bool VideoControl::compareMark()
{
    m_curSize = currentMark.width()*currentMark.height();
    return m_curSize >= m_stop_size;
}
