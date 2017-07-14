/**
 * @file       DiscernColor.h
 * @version    1.0
 * @date       2017年07月12日
 * @author     C_Are
 * @copyright  Leju
 *
 * @brief      当前识别颜色的类，DiscernColor类的h文件
 */

#ifndef DISCERNCOLOR_H
#define DISCERNCOLOR_H

#include "global_var.h"
#include <opencv2/opencv.hpp>
#include "Segmenter.h"

using namespace cv;
using namespace std;

extern int g_color_channel_Y; /* YUV格式的Y通道 */

/**
 * @class     DiscernColor
 * @brief     识别颜色位置，控制机器人动作
 */

class DiscernColor : public QThread
{
    Q_OBJECT
public:
    explicit DiscernColor(QObject *parent = 0);
    ~DiscernColor();

    /**
     * @brief 识别到的标记框的位置
     */

    enum Position{ Unknown, Left, Right, Center };

    /**
     * @brief 机器人动作状态
     */

    enum ActionStatus { Initial, Doing, Finished };

    void setSelectRect(const QRect &_rect);
    void setStopEnable(bool flag);
    void setActionMode(int mode);
    void setActionStatus(ActionStatus status);
    void setActionReady();
    void setColorChannelY(int val);

public slots:
    void readFrame(QImage *image);

protected:
    virtual void run();

signals:
    void directionChanged(int);
    void sendInfo(const QString &);
    void startMoveOn();

private:
    bool getCurrentMark(const vector<Object*> &objList);
    void calculateDirection();
    bool compareMark();

    void addColor(int rgbMean[3],unsigned char channelRange[2][2]);
    void segment(unsigned char *source, bool mask);

private:
    bool m_stopped;
    QMutex m_mutex;                 /**< 读取图片的锁 */
    QQueue<QImage> m_image_queue;   /**< 保存每一帧图片的队列 */
    QImage m_image;

    bool m_selected;
    QRect m_select_rect;
    Position curPosition;       /**< 当前目标所在的方向 */

    QRect currentMark;   /**< 识别物体的标记框位置 */
    int actionMode;      /**< 动作模式: 0,手动(测试阶段默认)  1,自动 */
    bool isReady;        /**< 是否做好动作准备 */

    ActionStatus actionStatus;   /**< 记录当前机器人动作状态 */

    bool m_bStopEnable;        /**< 是否可以到达指定位置后停止动作 */
    int m_stop_size;           /**< 停止位置的标记框大小 */
    bool isArrive;             /**< 是否到达标记位置 */
    int m_curSize;

    int m_left_command;
    int m_right_command;
    int m_action_order;

    QString m_mark_rgb;                 /**< 标记框颜色 */
    ColorInfo colorInfo;                /**< 记录标记的颜色的信息 */
    std::vector<Object*> objList;       /**< 识别到颜色位置的对象的容器 */
    unsigned int sizeThreshold;         /**< 识别到颜色位置的对象的像素点阀值 */
    int w;
    int h;
};

#endif // DISCERNCOLOR_H
