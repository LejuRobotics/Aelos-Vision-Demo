/**
 * @file       VideoControl.cpp
 * @version    1.0
 * @date       2017年07月08日
 * @author     C_Are
 * @copyright  Leju
 *
 * @brief      接收摄像头图像，VideoControl类的cpp文件
 */

#include "VideoControl.h"

/**
 * @brief     VideoControl类的构造函数
 * @param     parent 父对象
 */

VideoControl::VideoControl(QObject *parent) : QThread(parent)
{
    isPause = false;
    isSendFrame = true;
    alpha = 1.0;
    beta = 0;
}

/**
 * @brief     VideoControl类的析构函数
 * @details   关闭并销毁线程，等待3秒，否则强制关闭
 */

VideoControl::~VideoControl()
{
    isPause = true;
    if (udpSocket != NULL)
    {
        delete udpSocket;
    }
    this->quit();
    if (!this->wait(3000))
    {
        this->terminate();
    }
}

/**
 * @brief     准备向已连接的客户端发送摄像头图像
 * @param     ip 客户端ip地址
 */

void VideoControl::openUrl(const QString &ip)
{
    m_client_ip = ip;
    isPause = false;
    isSendFrame = true;
    if (!this->isRunning())
    {
        this->start();  //开启线程，进入run()函数
    }
}

/**
 * @brief     停止向客户端发送摄像头图像
 */

void VideoControl::stop()
{
    isSendFrame = false;
}

/**
 * @brief     设置亮度
 * @param     val 亮度
 */

void VideoControl::setBrightness(double val)
{
    alpha = val;
}

/**
 * @brief     设置对比度
 * @param     val 对比度
 */

void VideoControl::setContrast(int val)
{
    beta = val;
}

/**
 * @brief     设置摄像头分辨率
 * @param     w 宽
 * @param     h 高
 */

void VideoControl::setCameraResolution(int w, int h)
{
    if (w != g_frame_width)
    {
        isPause = true;
        g_frame_width = w;
        g_frame_height = h;
        if (w > 320 && g_frame_quality > 90)
        {
            g_frame_quality = -1;
        }
        QTimer::singleShot(1500, this, SLOT(restartCamera()));
    }
}

/**
 * @brief     设置通过hsv识别颜色的参数大小
 * @param     type 类型
 * @param     val 大小
 */

void VideoControl::setHsvInRange(const QString &type, int val)
{
//    qDebug()<< "setHsvInRange: "<<type<<val;
    if (type == "MinH")
    {
        g_hsv_lower[0] = val;
    }
    else if (type == "MinS")
    {
        g_hsv_lower[1] = val;
    }
    else if (type == "MinV")
    {
        g_hsv_lower[2] = val;
    }
    else if (type == "MaxH")
    {
        g_hsv_upper[0] = val;
    }
    else if (type == "MaxS")
    {
        g_hsv_upper[1] = val;
    }
    else if (type == "MaxV")
    {
        g_hsv_upper[2] = val;
    }
}

/**
 * @brief     重新开启摄像头
 */

void VideoControl::restartCamera()
{
    isPause = false;
    if (!this->isRunning())
    {
        this->start();  //开启线程，进入run()函数
    }
}

/**
 * @brief     在该线程中接收摄像头图像并通过UDP发送给客户端，通过信号槽发送每一帧图片给另外一个线程识别颜色
 */

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

    udpSocket = new QUdpSocket;
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

        //[1] 调节亮度和对比度
        if (alpha > 1.0 || beta > 0)
        {
            for( int y = 0; y < frame.rows; y++ )
            {
                Vec3b *p = frame.ptr<Vec3b>(y);  //通过指针遍历每一个像素点
                for( int x = 0; x < frame.cols; x++ )
                {
                    for( int c = 0; c < 3; c++ )
                    {
//                        frame.at<Vec3b>(y,x)[c] = saturate_cast<uchar>( alpha*( frame.at<Vec3b>(y,x)[c] ) + beta );
                        p[x][c] = saturate_cast<uchar>( alpha*( frame.at<Vec3b>(y,x)[c] ) + beta );
                    }
                }
            }
        }
        //[1]

        //[2] 发送图片给其它线程进行识别颜色
        cvtColor(frame,rgb_mat,CV_BGR2RGB);
        rgbImg = new QImage((uchar*) rgb_mat.data, rgb_mat.cols, rgb_mat.rows,
                            rgb_mat.cols*rgb_mat.channels(), QImage::Format_RGB888);
        emit sendFrame(rgbImg);
        //[2]

        //[3]将摄像头图像发送给客户端
        if (isSendFrame)
        {
            QBuffer buf(&byte);
            buf.open(QIODevice::WriteOnly);

            if (G_Image_Format == "RGB")
            {
                rgbImg->save(&buf,"JPEG",g_frame_quality);  //压缩图片大小
            }
            else if (G_Image_Format == "YUV")
            {
                cv::cvtColor(frame, yuv_mat,CV_BGR2YUV);
                for(int i = 0; i < g_frame_height; i++)
                {
                    Vec3b *p = yuv_mat.ptr<Vec3b>(i);  //通过指针遍历每一个像素点
                    for(int j = 0; j < g_frame_width; j++)
                    {
    //                    yuv_mat.at<cv::Vec3b>(i,j)[0] = 128;
                        p[j][0] = g_color_channel_Y;
                    }
                }
                yuvImg = new QImage((uchar*) yuv_mat.data, yuv_mat.cols, yuv_mat.rows,
                                    yuv_mat.cols*yuv_mat.channels(), QImage::Format_RGB888);
                yuvImg->save(&buf,"JPEG",g_frame_quality);  //压缩图片大小
            }
            else if (G_Image_Format == "HSV")
            {
                //转到HSV空间
                cvtColor(frame,hsv_mat,COLOR_BGR2HSV);

                //根据阈值构建掩膜
                inRange(hsv_mat, g_hsv_lower, g_hsv_upper, hsv_mat);

                int g_nStructElementSize = 9; //结构元素(内核矩阵)的尺寸
                cv::Mat str_el = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(g_nStructElementSize, g_nStructElementSize));

                //腐蚀操作
                erode(hsv_mat, hsv_mat, str_el);
                //膨胀操作，其实先腐蚀再膨胀的效果是开运算，去除噪点
                dilate(hsv_mat, hsv_mat, str_el);

                //轮廓检测
                vector<vector<Point> > contours;
                findContours(hsv_mat, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

                vector<Moments>mu(contours.size());
                vector<Point2f>mc(contours.size());
                vector<double> areaVec;
                for (size_t i=0; i<contours.size(); ++i)
                {
                    //计算轮廓的面积
                    double tmparea = fabs(contourArea(contours[i]));
                    areaVec.push_back(tmparea);

                    mu[i] = moments(contours[i], false);
                    mc[i] = Point2f(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);
                }

                //选择最大面积的轮廓
                int max_pos = (int)(max_element(areaVec.begin(), areaVec.end()) - areaVec.begin());

                cvtColor(hsv_mat,hsv_mat,COLOR_GRAY2RGB);
                if (max_pos < (int)contours.size())
                {

                    Rect findRect = boundingRect(contours[max_pos]);
                    rectangle(hsv_mat, findRect, Scalar(0,255,0),2);
                    circle(hsv_mat, mc[max_pos], 5, Scalar(0,0,255)); //在重心坐标画圆
                }
                hsvImg = new QImage((uchar*) hsv_mat.data, hsv_mat.cols, hsv_mat.rows,
                                    hsv_mat.cols*hsv_mat.channels(), QImage::Format_RGB888);

                hsvImg->save(&buf,"JPEG",g_frame_quality);  //压缩图片大小
            }

            QByteArray datagram;
            QDataStream out(&datagram, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_5_3);
            out << QString("IMAGE").toUtf8() << byte;
            udpSocket->writeDatagram(datagram, QHostAddress(m_client_ip),g_broadcast_port);  // 向指定ip地址发送图像
            byte.resize(0);
        }
        //[3]
        msleep(1);
    }

    if (isPause)
    {
        cap.release();
    }
}

