#ifndef VIDEOCONTROL_H
#define VIDEOCONTROL_H

#include <QObject>
#include <QThread>
#include <QImage>
#include <QBuffer>
#include <QTcpSocket>
#include <QTimer>
#include <QTime>
#include <QUdpSocket>

#include <opencv2/opencv.hpp>
#include "Segmenter.h"
#include "global_var.h"

using namespace cv;
using namespace std;

class VideoControl : public QThread
{
    Q_OBJECT
public:
    explicit VideoControl(QObject *parent = 0);
    ~VideoControl();

    enum Position{ Unknown, Left, Right, Center };

    enum ActionStatus { Initial, Doing, Finished};

    QRect currentMark;   //识别物体的标记框位置
    int actionMode;      //动作模式: 0,手动(测试阶段默认)  1,自动
    bool isReady;        //是否做好动作准备

    ActionStatus actionStatus;

    void setSelectRect(const QRect &tmp); //标记需要识别的物体
    void openUrl(const QString &ip);

    void stop();

protected:
    virtual void run();

signals:
    void markChanged(bool);
    void directionChanged(int);
    void sendInfo(const QString &);

public slots:
    void setRebotStatus(ActionStatus status);

private:
    bool getCurrentMark(const vector<Object*> &objList); //获取最接近目标的标记框
    void calculateDirection(); //计算物体方向(左，中，右)

private:
    bool isPause;
    bool isSendFrame;
    QImage *rgbImg;
    Mat rgb_mat;

    QString server_ip;

    bool isSelected;
    QRect m_select_rect;
    Position curPosition; //物体所处的方向
    int m_left_command;
    int m_right_command;

    QUdpSocket *udpSocket;
    QByteArray byte;
};

#endif // VIDEOCONTROL_H
