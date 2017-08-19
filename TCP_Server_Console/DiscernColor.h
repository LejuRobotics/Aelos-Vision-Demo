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

using namespace cv;
using namespace std;

extern int g_color_channel_Y; /** < YUV格式的Y通道 */
extern Scalar g_hsv_lower;
extern Scalar g_hsv_upper;

const int g_nStructElementSize = 9;  /**< 结构元素(内核矩阵)的尺寸*/

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

    enum Position{
        Unknown,
        LeftNear,
        LeftFar,
        RightNear,
        RightFar,
        Center
    };

    /**
     * @brief 机器人动作状态
     */

    enum ActionStatus { Initial, Doing, Finished };

    /**
     * @brief 动作模式
     */

    enum MoveMode {
        Track,       /**< 快速靠近模式 */
        Access,      /**< 慢慢靠近模式 */
        Obstacle,    /**< 避障模式 */       
        Wait         /**< 等待模式 */
    };

    /**
     * @brief 标记框
     */

    struct Object{
        unsigned char ID;
        unsigned char colorID;
        unsigned int centerX,centerY;
        unsigned int minX,maxX,minY,maxY;
        unsigned int pixelCounter = 0;
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

    struct Target {
        QString name;           /**< 目标名称 */
        int type;               /**< 目标类型，0障碍物，1目标 */
        int maxWidth;           /**< 在停止靠近目标的位置识别到的标记框宽度 */
        int turn;               /**< 避障转动方向，0左边，1右边 */
        ColorInfo colorInfo;    /**< 目标的颜色信息 */
        int state;              /**< 状态，0未完成，1已完成 */
    };

    struct HSV_Target {
        QString name;           /**< 目标名称 */
        int type;               /**< 目标类型，0障碍物，1目标 */
        int maxWidth;           /**< 在停止靠近目标的位置识别到的标记框宽度 */
        int turn;               /**< 避障转动方向，0左边，1右边 */
        Scalar hsvLower;        /**< hsv识别颜色inrange函数中的颜色范围下界 */
        Scalar hsvUpper;        /**< hsv识别颜色inrange函数中的颜色范围上界 */
        int state;              /**< 状态，0未完成，1已完成 */
    };

    void Reset();
    void setSelectRect(const QRect &_rect);
    bool addTarget(const QStringList &list);
    bool setTargetType(int index, int type);
    bool setTargetTurn(int index, int turn);
    bool removeTarget(int index);
    void setActionMode(int mode);
    void setActionStatus(ActionStatus status);
    void setActionReady();
    void setColorChannelY(int val);

    void startAddHsvTarget();

public slots:
    void readFrame(QImage &image);

protected:
    virtual void run();

signals:
    void directionChanged(int);
    void sendInfo(const QString &);
    void startMoveOn(int msec);

private:
    void findColorFromYUV(Mat &frame);
    void findColorFaromHSV(Mat &frame);

    void findContoursFromHSV(Mat &frame, int type);

    bool getCurrentMark(const vector<Object*> &objList);
    void calculateDirection();
    void calculateDirection2(const QPoint &pos);
    void compareTargetWidth(int index);
    int  currentTarget() const;

    void findTarget();

    void trackingTarget();
    void accessTarget();
    void obstacleAvoidance();

    void addColor(Mat &frame);
    void segment(Mat &frame, ColorInfo &info, bool mask);
    unsigned char GetLabel(ColorInfo &info, unsigned char *source, int pIndex);

private:
    bool m_stopped;
    QMutex m_mutex;                 /**< 读取图片的锁 */
    QQueue<QImage> m_image_queue;   /**< 保存每一帧图片的队列 */
    QImage m_image;

    bool m_selected;
    QRect m_select_rect;
    Position curPosition;          /**< 当前目标所在的方向 */

    QRect currentMark;             /**< 识别物体的标记框位置 */
    QPoint currentCenterPoint;
    int actionMode;                /**< 动作模式: 0,手动(测试阶段默认)  1,自动 */
    bool isReady;                  /**< 是否做好动作准备 */

    ActionStatus actionStatus;     /**< 记录当前机器人动作状态 */

    MoveMode moveMode;

    int targetNumber;
    int obstacleCount;
    int m_findCount;
    int m_nMoveOnTimeCount;

    QString m_mark_rgb;                 /**< 标记框颜色 */
    ColorInfo colorInfo;                /**< 记录标记的颜色的信息 */
    std::vector<Object*> objList;       /**< 识别到颜色位置的对象的容器 */
    unsigned int sizeThreshold;         /**< 识别到颜色位置的对象的像素点阀值 */
    int w;
    int h;

    QList<Target> m_targetList;        /**< 记录目标的容器 */

    bool m_bIsHsvSelected;
    Mat hsv_mat;
    bool m_bAddHsvFlag;
    Rect m_hsvCurrentMark;
    int m_hsvTargetNum;
    QList<HSV_Target> m_hsvTargetList;

    vector<vector<Point> > contours;

};

#endif // DISCERNCOLOR_H
