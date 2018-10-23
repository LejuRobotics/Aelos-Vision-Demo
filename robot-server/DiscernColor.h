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

    enum ActionStatus {
        Finished,      /**< 初始状态和动作完成以后的状态 */
        QuickWalk,     /**< 持续快走 */
        QuickBack,     /**< 快退 */
        TurnLeft_S,    /**< 小幅度左转 */
        TurnRight_S,   /**< 小幅度右转 */
        TurnLeft_L,    /**< 大幅度左转 */
        TurnRight_L,   /**< 大幅度右转 */
        StoopDown,     /**< 弯腰 */
        LeftShift,     /**< 左移 */
        RightShift,    /**< 右移 */
        ShootFootBall  /**< 射门 */
    };

    /**
     * @brief 动作模式
     */

    enum MoveMode {
        Track,       /**< 快速靠近模式 */
        Access,      /**< 慢慢靠近模式 */
        Obstacle,    /**< 避障模式 */       
        Wait,        /**< 等待模式 */
        Shoot,       /**< 开始射门模式 */
        ShootAdjust  /**< 调整射门位置模式 */
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

    /**
     * @brief YUV目标
     */
    struct YUV_Target {
        QString name;           /**< 目标名称 */
        int type;               /**< 目标类型，0障碍物，1目标, 2足球*/
        int maxWidth;           /**< 在停止靠近目标的位置识别到的标记框宽度 */
        int turn;               /**< 避障转动方向，0左边，1右边 */
        ColorInfo colorInfo;    /**< 目标的颜色信息 */
        int state;              /**< 状态，0未完成，1已完成 */
    };

    /**
     * @brief HSV目标
     */
    struct HSV_Target {
        QString name;           /**< 目标名称 */
        int type;               /**< 目标类型，0障碍物，1目标, 2足球*/
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
    void setColorChannelY(int val);
    void startAddHsvTarget();
    void startAgain();

public slots:
    void readFrame(QImage &image);
    void setActionReady();

protected:
    virtual void run();

signals:
    void directionChanged(int);
    void sendInfo(const QString &);
    void startMoveOn(int msec);

private:
    void manualOperation();
    void autoOperation();

    void autoForYUV();
    void derectOfYUV();
    void recordStoopDownOfYUV();

    void autoForHSV();
    void derectOfHSV();
    void recordStoopDownOfHSV();

    void excuteTurnRound();

    bool getCurrentMark(const vector<Object*> &objList);
    void calculateDirection(const QPoint &pos);
    int  currentTarget() const;

    void findTarget();

    void trackingTarget();
    void accessTarget();
    void obstacleAvoidance();
    void shootFootball();
    void adjustShootFootball();

    void addColor(Mat &frame);
    void segment(Mat &frame, ColorInfo &info, bool mask);
    unsigned char GetLabel(ColorInfo &info, unsigned char *source, int pIndex);

private:
    bool m_stopped;
    QMutex m_mutex;                 /**< 读取图片的锁 */
    QQueue<QImage> m_image_queue;   /**< 保存每一帧图片的队列 */
    QImage m_image;
    Mat m_frame;

    bool m_selected;
    QRect m_select_rect;
    Position curPosition;          /**< 当前目标所在的方向 */


    int actionMode;                /**< 动作模式: 0,手动(测试阶段默认)  1,自动 */
    bool isReady;                  /**< 是否做好动作准备 */

    ActionStatus actionStatus;     /**< 记录当前机器人动作状态 */
    ActionStatus lastActionStatus;  /**< 记录上一次机器人动作状态 */

    MoveMode moveMode;

    QString m_mark_rgb;                 /**< 标记框颜色 */
    ColorInfo colorInfo;                /**< 记录标记的颜色的信息 */
    std::vector<Object*> objList;       /**< 识别到颜色位置的对象的容器 */
    unsigned int sizeThreshold;         /**< 识别到颜色位置的对象的像素点阀值 */
    int w;
    int h;

    int obstacleCount;
    int m_findCount;
    int m_nMoveOnTimeCount;

    int m_yuvTargetNum;
    Rect currentMark;                   /**< YUV识别物体的标记框位置 */
    QPoint m_yuvCenterPoint;
    QList<YUV_Target> m_yuvTargetList;  /**< 记录YUV目标的容器 */

    Mat hsv_mat;
    bool m_bAddHsvFlag;
    Rect m_hsvCurrentMark;              /**< HSV识别物体的标记框位置 */
    int m_hsvTargetNum;
    QPoint m_hsvCenterPoint;
    QList<HSV_Target> m_hsvTargetList;  /**< 记录HSV目标的容器 */

    int m_targetType;
    int m_shootActionsFinishedCount;

    QList<int> m_area_list;
    QList<Rect> m_mark_list;
    QList<QPoint> m_center_point_list;

    QList<YUV_Target> m_yuv_return_list;
    QList<HSV_Target> m_hsv_return_list;

    bool m_bTurnRoundFinished;
};

#endif // DISCERNCOLOR_H
