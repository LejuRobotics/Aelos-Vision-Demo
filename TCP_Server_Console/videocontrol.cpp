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
    m_iamge_format = "RGB";
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

void VideoControl::setBrightness(double val)
{
    alpha = val;
}

void VideoControl::setContrast(int val)
{
    beta = val;
}

void VideoControl::setImageFormat(const QString &format)
{
    m_iamge_format = format;
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

            if (m_iamge_format == "RGB")
            {
                rgbImg->save(&buf,"JPEG",g_frame_quality);  //压缩图片大小
            }
            else if (m_iamge_format == "YUV")
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
}


