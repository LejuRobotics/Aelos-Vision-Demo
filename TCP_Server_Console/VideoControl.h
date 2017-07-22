/**
 * @file       VideoControl.h
 * @version    1.0
 * @date       2017年07月08日
 * @author     C_Are
 * @copyright  Leju
 *
 * @brief      接收摄像头图像，VideoControl类的h文件
 */

#ifndef VIDEOCONTROL_H
#define VIDEOCONTROL_H

#include "global_var.h"
#include "DiscernColor.h"

/**
 * @class     VideoControl
 * @brief     接收摄像头图像
 */

class VideoControl : public QThread
{
    Q_OBJECT
public:
    explicit VideoControl(QObject *parent = 0);
    ~VideoControl();

    void openUrl(const QString &ip);
    void stop();
    void setBrightness(double val);
    void setContrast(int val);
    void setImageFormat(const QString &format);
    void setCameraResolution(int w, int h);

private slots:
    void restartCamera();

protected:
    virtual void run();

signals:
    void sendInfo(const QString &);
    void sendFrame(QImage *);

private:
    bool isPause;
    bool isSendFrame;         /**< 是否向客户端发送摄像头头像的标志 */

    QString m_client_ip;      /**< 客户端ip */

    QUdpSocket *udpSocket;
    QByteArray byte;

    Mat rgb_mat;
    QImage *rgbImg;
    Mat yuv_mat;
    QImage *yuvImg;

    double alpha;             /**< 亮度 */
    int beta;                 /**< 对比度 */
    QString m_iamge_format;   /**< 图片格式 */
};

#endif // VIDEOCONTROL_H
