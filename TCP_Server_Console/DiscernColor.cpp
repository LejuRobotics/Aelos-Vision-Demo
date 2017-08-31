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

int g_color_channel_Y = 128;
Scalar g_hsv_lower;
Scalar g_hsv_upper;

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
    actionStatus = Finished;
    lastActionStatus = Finished;
    moveMode = Track;
    w = g_frame_width;
    h = g_frame_height;
    sizeThreshold = (int)(100 * (g_frame_width/640.0));
    obstacleCount = 0;
    m_findCount = 0;
    m_nMoveOnTimeCount = 3000;
    m_bAddHsvFlag = false;
    m_targetType = 0;
    m_shootActionsFinishedCount = 0;
    m_bTurnRoundFinished = false;

    g_hsv_lower[0] = 0;
    g_hsv_lower[1] = 100;
    g_hsv_lower[2] = 100;
    g_hsv_upper[0] = 180;
    g_hsv_upper[1] = 255;
    g_hsv_upper[2] = 255;
}

/**
 * @brief     DiscernColor类的析构函数
 * @details   关闭线程,如果等待时间超过1s强制关闭
 */

DiscernColor::~DiscernColor()
{
    m_stopped = true;
    this->quit();
    if (!this->wait(1000))
    {
        this->terminate();
    }
}

/**
 * @brief     重置，重新初始化一些参数
 */

void DiscernColor::Reset()
{
    actionMode = 0;
    actionStatus = Finished;
    lastActionStatus = Finished;
    isReady = false;
    curPosition = Unknown;        
    moveMode = Track;
    obstacleCount = 0;
    m_bTurnRoundFinished = false;
    m_yuvTargetList.clear();
    m_hsvTargetList.clear();
}

/**
 * @brief      添加一个目标，保存该目标的名称，停止位置的宽度，类型，和转动方向
 * @param      name  目标名称
 * @param      width 停止位置识别到的目标标记框的宽度
 * @param      type  类型，目前有两种类型，一种是障碍物，一种是目标
 * @param      turn  做避障动作时的转动方向
 */

bool DiscernColor::addTarget(const QStringList &list)
{
    if (G_Image_Format == "YUV")
    {
        if (list.length() != 4)
            return false;

        YUV_Target obj;
        obj.name = list[0];
        obj.maxWidth = list[1].toInt();
        obj.type = list[2].toInt();
        obj.turn = list[3].toInt();
        obj.colorInfo = colorInfo;
        obj.state = 0;
        m_yuvTargetList << obj;
        return true;
    }
    else if (G_Image_Format == "HSV")
    {
        if (list.length() != 10)
            return false;

        HSV_Target tempObj;
        tempObj.name = list[0];
        tempObj.maxWidth = list[1].toInt();
        tempObj.type = list[2].toInt();
        tempObj.turn = list[3].toInt();
        tempObj.hsvLower[0] = list[4].toInt();
        tempObj.hsvLower[1] = list[5].toInt();
        tempObj.hsvLower[2] = list[6].toInt();
        tempObj.hsvUpper[0] = list[7].toInt();
        tempObj.hsvUpper[1] = list[8].toInt();
        tempObj.hsvUpper[2] = list[9].toInt();
        tempObj.state = 0;
        m_hsvTargetList << tempObj;
        return true;
    }
    return false;
}

/**
 * @brief     设置目标类型
 * @param     index  第几个目标
 * @param     type   目标类型,0障碍物, 1目标
 */

bool DiscernColor::setTargetType(int index, int type)
{
    if (G_Image_Format == "YUV")
    {
        if (index < m_yuvTargetList.size())
        {
            m_yuvTargetList[index].type = type;
            return true;
        }
    }
    else if (G_Image_Format == "HSV")
    {
        if (index < m_hsvTargetList.size())
        {
            m_hsvTargetList[index].type = type;
            return true;
        }
    }
    return false;
}

/**
 * @brief     设置避障动作的转动方向
 * @param     index  第几个目标
 * @param     type   转动方向,0左边, 1右边
 */

bool DiscernColor::setTargetTurn(int index, int turn)
{
    if (G_Image_Format == "YUV")
    {
        if (index < m_yuvTargetList.size())
        {
            m_yuvTargetList[index].turn = turn;
            return true;
        }
    }
    else if (G_Image_Format == "HSV")
    {
        if (index < m_hsvTargetList.size())
        {
            m_hsvTargetList[index].turn = turn;
            return true;
        }
    }
    return false;
}

/**
 * @brief     删除指定的目标
 * @param     index  第几个目标
 */

bool DiscernColor::removeTarget(int index)
{
    if (G_Image_Format == "YUV")
    {
        if (index < m_yuvTargetList.size())
        {
            m_yuvTargetList.removeAt(index);
            return true;
        }
    }
    else if (G_Image_Format == "HSV")
    {
        if (index < m_hsvTargetList.size())
        {
            m_hsvTargetList.removeAt(index);
            return true;
        }
    }

    return false;
}

/**
 * @brief     设置机器人是自动还是手动执行动作
 * @param     mode 为0手动，否则自动
 */

void DiscernColor::setActionMode(int mode)
{
    actionMode = mode;
    if (mode)
    {
        actionStatus = Finished;
        isReady = true;
        m_bTurnRoundFinished = false;
    }
    else
    {
        isReady = false;
        m_bTurnRoundFinished = true;
    }
}

/**
 * @brief     设置机器人动作状态，当收到串口值确认动作完成则通过该函数设置动作完成状态
 * @param     status 动作状态
 */

void DiscernColor::setActionStatus(DiscernColor::ActionStatus status)
{
    actionStatus = status;
    if (status == Finished)
    {
        QTimer::singleShot(g_time_count, this, SLOT(setActionReady()));
    }
    else
    {
        lastActionStatus = status;
        isReady = false;
    }
}

/**
 * @brief     执行完一个动作，会添加一定时间的延迟，延迟时间到会执行该函数，让机器人可以执行下一次动作
 */

void DiscernColor::setActionReady()
{
    isReady = true;
}

void DiscernColor::startAddHsvTarget()
{
    m_bAddHsvFlag = true;
}

void DiscernColor::startAgain()
{
    if (G_Image_Format == "YUV")
    {
        for (int i=0; i<m_yuvTargetList.size(); ++i)
        {
            m_yuvTargetList[i].state = 0;
        }
    }
    else if (G_Image_Format == "HSV")
    {
        for (int i=0; i<m_hsvTargetList.size(); ++i)
        {
            m_hsvTargetList[i].state = 0;
        }
    }
    actionStatus = Finished;
    lastActionStatus = Finished;
    isReady = true;
    moveMode = Track;
    obstacleCount = 0;
    m_bTurnRoundFinished = false;
}

/**
 * @brief     接收每一帧图片放到线程中处理
 * @param     iamge 图片
 * @details   每次接收一张图片会用锁锁住，并且复制一张图片，然后放到线程中处理，确保处理好每一帧图片
 */

void DiscernColor::readFrame(QImage &image)
{
    m_mutex.lock();
    m_image = image.copy();
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
        //把QImage转为Mat，格式为RGB
        m_frame = Mat(nextImage.height(), nextImage.width(), CV_8UC3, (void*)nextImage.constBits(), nextImage.bytesPerLine());

        if (actionMode == 0) //手动
        {
            manualOperation();
        }
        else if (actionMode == 1) //自动
        {
            autoOperation();
        }
        msleep(1);
    }
}

void DiscernColor::manualOperation()
{
    if (G_Image_Format == "YUV")
    {
        QString msg("@Begin:\r\n");
        if(m_selected)  //当客户端标记一个颜色时
        {
            addColor(m_frame);
            segment(m_frame, colorInfo, 1);
            if (getCurrentMark(objList)) //当识别到物体位置
            {
                if (!m_mark_rgb.isEmpty())
                {
                    msg.append(m_mark_rgb);
                    m_mark_rgb.clear();
                }
                msg.append(QString("Mark.Rect=%1,%2,%3,%4\r\n")
                           .arg(currentMark.x)
                           .arg(currentMark.y)
                           .arg(currentMark.width)
                           .arg(currentMark.height));

            }
            else //没有识别到物体位置
            {
                msg.append("Unrecognized color\r\n");
            }
            msg.append("@End\r\n");
            emit sendInfo(msg);  //发送结果给客户端
            m_selected = false;
        }
    }
    else if (G_Image_Format == "HSV")
    {
        if (m_selected)
        {
            cvtColor(m_frame,hsv_mat,COLOR_RGB2HSV);
            cv::Mat frameROI;
            frameROI = hsv_mat(cv::Rect(m_select_rect.x(), m_select_rect.y(), m_select_rect.width(), m_select_rect.height()));
            cv::Scalar hsvMean = cv::mean(frameROI); //计算选中区域的hsv平均值
            int val_lower = qRound(hsvMean[0]) - 10;
            int val_upper = qRound(hsvMean[0]) + 10;
            if (val_lower < 0 )
                val_lower = 0;
            if (val_upper > 180)
                val_upper = 180;
            g_hsv_lower[0] = val_lower;
            g_hsv_lower[1] = 100;
            g_hsv_lower[2] = 100;
            g_hsv_upper[0] = val_upper;
            g_hsv_upper[1] = 255;
            g_hsv_upper[2] = 255;
            QString msg = QString("Return Current.HSV.Range=%1,%2,%3,%4,%5,%6")
                    .arg(g_hsv_lower[0])
                    .arg(g_hsv_lower[1])
                    .arg(g_hsv_lower[2])
                    .arg(g_hsv_upper[0])
                    .arg(g_hsv_upper[1])
                    .arg(g_hsv_upper[2]);
            emit sendInfo(msg);
            m_selected = false;
        }

        if (m_bAddHsvFlag)
        {
            //转到HSV空间
            cvtColor(m_frame,hsv_mat,COLOR_RGB2HSV);

            //根据阈值构建掩膜
            inRange(hsv_mat, g_hsv_lower, g_hsv_upper, hsv_mat);

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

                //计算重心
                mu[i] = moments(contours[i], false);
                mc[i] = Point2f(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);
            }

            int max_pos = (int)(max_element(areaVec.begin(), areaVec.end()) - areaVec.begin());

            QString msg("@Begin HSV:\r\n");
            if (max_pos < (int)contours.size())
            {
                m_hsvCurrentMark = boundingRect(contours[max_pos]);
                msg.append(QString("Max.Width=%1\r\n").arg(m_hsvCurrentMark.width));
            }
            else
            {
                curPosition = Unknown;
                msg.append("Unrecognized color\r\n");
            }
            msg.append("@End\r\n");
            emit sendInfo(msg);  //发送结果给客户端
            m_bAddHsvFlag = false;
        }
    }
}

void DiscernColor::autoOperation()
{
    if (G_Image_Format == "YUV")
    {
        autoForYUV();
    }
    else if (G_Image_Format == "HSV")
    {
        autoForHSV();
    }
}

void DiscernColor::autoForYUV()
{
    m_yuvTargetNum = currentTarget();
    if (m_yuvTargetNum != -1)
    {
        m_targetType = m_yuvTargetList[m_yuvTargetNum].type;

        if ((moveMode == Obstacle || moveMode == Shoot) && actionStatus == Finished && isReady) //到达指定位置，开始做避障动作，停止或者射门
        {
            if (moveMode == Obstacle)
            {
                obstacleAvoidance();
            }
            else if (moveMode == Shoot)
            {
                shootFootball();
            }
            return;
        }

        if (lastActionStatus == StoopDown)  //弯腰识别
        {
            recordStoopDownOfYUV();
        }
        else  //接近目标
        {
            if (actionStatus != Finished || !isReady || moveMode == Wait)
                return;

            derectOfYUV();
        }
    }

    else
    {
        if (G_Go_Back_Flag)
        {
            excuteTurnRound();
        }
    }
}

void DiscernColor::derectOfYUV()
{
    segment(m_frame, m_yuvTargetList[m_yuvTargetNum].colorInfo, 1);
    QString msg("@Begin:\r\n");
    if (getCurrentMark(objList)) //当识别到物体位置
    {
        msg.append(QString("Mark.Rect=%1,%2,%3,%4\r\n")
                   .arg(currentMark.x)
                   .arg(currentMark.y)
                   .arg(currentMark.width)
                   .arg(currentMark.height));

        calculateDirection(m_yuvCenterPoint);

        if (currentMark.width > (int)(m_yuvTargetList[m_yuvTargetNum].maxWidth*g_arrive_ratio))
        {
            setActionStatus(StoopDown);
            emit directionChanged(g_stoop_down_command);
        }
        else if (currentMark.width > (int)(m_yuvTargetList[m_yuvTargetNum].maxWidth*g_access_ratio))
        {
            moveMode = Access;
        }
        else
        {
            moveMode = Track;
        }
    }
    else //没有识别到物体位置
    {
        curPosition = Unknown;
        msg.append("Unrecognized color\r\n");
    }
    msg.append("@End\r\n");
    emit sendInfo(msg);  //发送结果给客户端

    if (lastActionStatus == StoopDown)
        return;

    if (moveMode == Track)
    {
        trackingTarget();
    }
    else if (moveMode == Access)
    {
        accessTarget();
    }
    else if (moveMode == Obstacle)
    {
        obstacleAvoidance();
    }
    else if (moveMode == Wait)
    {
        emit sendInfo(QString("Reach.Target=%1").arg(m_yuvTargetList[m_yuvTargetNum].name));
    }
}

void DiscernColor::recordStoopDownOfYUV()
{
    if (actionStatus != Finished || !isReady) //正在做弯腰动作,需要记录弯腰过程中识别到的目标位置
    {
        segment(m_frame, m_yuvTargetList[m_yuvTargetNum].colorInfo, 1);
        if (getCurrentMark(objList)) //当识别到物体位置
        {
            int area = currentMark.width * currentMark.height;
            m_area_list.append(area);
            m_mark_list.append(currentMark);
            m_center_point_list.append(m_yuvCenterPoint);
        }
    }
    else if (actionStatus == Finished && isReady)  //弯腰动作完成
    {
        QString msg("@Begin:\r\n");
        if (!m_area_list.isEmpty() && !m_mark_list.isEmpty() && !m_center_point_list.isEmpty())
        {
            QList<int> tempList = m_area_list;
            //从小到大排序
            std::sort(m_area_list.begin(), m_area_list.end());
            //选择中值
            int p = 0;
            if (m_area_list.size()%2)
            {
                p = m_area_list.size()/2;
            }
            else
            {
                p = (m_area_list.size()+1)/2;
            }
            int index = tempList.indexOf(m_area_list[p]);
//            qDebug()<< "p: "<< p << "\n"
//                    << "index: " << index << "\n"
//                    << "tempList: "<< tempList << "\n"
//                    << "m_area_list: " << m_area_list;

            currentMark = m_mark_list[index];
            msg.append(QString("Mark.Rect=%1,%2,%3,%4\r\n")
                       .arg(currentMark.x)
                       .arg(currentMark.y)
                       .arg(currentMark.width)
                       .arg(currentMark.height));

            m_yuvCenterPoint.setX(m_center_point_list[index].x());
            m_yuvCenterPoint.setY(m_center_point_list[index].y());
            calculateDirection(m_center_point_list[index]);

            if (currentMark.width > (int)(m_yuvTargetList[m_yuvTargetNum].maxWidth*g_arrive_ratio))
            {
                if (m_targetType == 0)
                {
                    moveMode = Obstacle;
                }
                else if (m_targetType == 1)
                {
                    moveMode = Wait;
                    m_yuvTargetList[m_yuvTargetNum].state = 1;
                }
                else if (m_targetType == 2)
                {
                    moveMode = ShootAdjust;
                }
            }
            else if (currentMark.width > (int)(m_yuvTargetList[m_yuvTargetNum].maxWidth*g_access_ratio))
            {
                moveMode = Access;
            }
            else
            {
                moveMode = Track;
            }

            m_area_list.clear();
            m_mark_list.clear();
            m_center_point_list.clear();
        }
        else
        {
            curPosition = Unknown;
            msg.append("Unrecognized color\r\n");
        }

        msg.append("@End\r\n");
        emit sendInfo(msg);  //发送结果给客户端

        if (moveMode == Track)
        {
            trackingTarget();
        }
        else if (moveMode == Access)
        {
            accessTarget();
        }
        else if (moveMode == Wait)
        {
            emit sendInfo(QString("Reach.Target=%1").arg(m_yuvTargetList[m_yuvTargetNum].name));
        }
        else if (moveMode == ShootAdjust)
        {
            adjustShootFootball();
        }
    }
}

void DiscernColor::autoForHSV()
{
    m_hsvTargetNum = currentTarget();
    if (m_hsvTargetNum != -1)
    {
        m_targetType = m_hsvTargetList[m_hsvTargetNum].type;

        if ((moveMode == Obstacle || moveMode == Shoot) && actionStatus == Finished && isReady) //到达指定位置，开始做避障动作，停止或者射门
        {
            if (moveMode == Obstacle)
            {
                obstacleAvoidance();
            }
            else if (moveMode == Shoot)
            {
                shootFootball();
            }
            return;
        }

        if (lastActionStatus == StoopDown)  //弯腰识别
        {
            recordStoopDownOfHSV();
        }
        else  //接近目标
        {
            if (actionStatus != Finished || !isReady || moveMode == Wait)
                return;

            derectOfHSV();
        }
    }

    else
    {
        if (G_Go_Back_Flag)
        {
            excuteTurnRound();
        }
    }
}

void DiscernColor::derectOfHSV()
{
    g_hsv_lower = m_hsvTargetList[m_hsvTargetNum].hsvLower;
    g_hsv_upper = m_hsvTargetList[m_hsvTargetNum].hsvUpper;

    //转到HSV空间
    cvtColor(m_frame,hsv_mat,COLOR_RGB2HSV);

    //根据阈值构建掩膜
    inRange(hsv_mat, m_hsvTargetList[m_hsvTargetNum].hsvLower, m_hsvTargetList[m_hsvTargetNum].hsvUpper, hsv_mat);

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

        //计算重心
        mu[i] = moments(contours[i], false);
        mc[i] = Point2f(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);
    }

    int max_pos = (int)(max_element(areaVec.begin(), areaVec.end()) - areaVec.begin());

    QString msg("@Begin HSV:\r\n");
    if (max_pos < (int)contours.size())
    {
        m_hsvCurrentMark = boundingRect(contours[max_pos]);
        msg.append(QString("Mark.Rect=%1,%2,%3,%4\r\n")
                   .arg(m_hsvCurrentMark.x)
                   .arg(m_hsvCurrentMark.y)
                   .arg(m_hsvCurrentMark.width)
                   .arg(m_hsvCurrentMark.height));

        m_hsvCenterPoint.setX(mc[max_pos].x);
        m_hsvCenterPoint.setY(mc[max_pos].y);
        calculateDirection(m_hsvCenterPoint);

        if (m_hsvCurrentMark.width > (int)(m_hsvTargetList[m_hsvTargetNum].maxWidth*g_arrive_ratio))
        {
            setActionStatus(StoopDown);
            emit directionChanged(g_stoop_down_command);
        }
        else if (m_hsvCurrentMark.width > (int)(m_hsvTargetList[m_hsvTargetNum].maxWidth*g_access_ratio))
        {
            moveMode = Access;
        }
        else
        {
            moveMode = Track;
        }
    }
    else
    {
        curPosition = Unknown;
        msg.append("Unrecognized color\r\n");
    }
    msg.append("@End\r\n");
    emit sendInfo(msg);  //发送结果给客户端

    if (lastActionStatus == StoopDown)
        return;

    if (moveMode == Track)
    {
        trackingTarget();
    }
    else if (moveMode == Access)
    {
        accessTarget();
    }
    else if (moveMode == Wait)
    {
        emit sendInfo(QString("Reach.Target=%1").arg(m_hsvTargetList[m_hsvTargetNum].name));
    }

}

void DiscernColor::recordStoopDownOfHSV()
{
    if (actionStatus != Finished || !isReady) //正在做弯腰动作,需要记录弯腰过程中识别到的目标位置
    {
        g_hsv_lower = m_hsvTargetList[m_hsvTargetNum].hsvLower;
        g_hsv_upper = m_hsvTargetList[m_hsvTargetNum].hsvUpper;

        //转到HSV空间
        cvtColor(m_frame,hsv_mat,COLOR_RGB2HSV);

        //根据阈值构建掩膜
        inRange(hsv_mat, m_hsvTargetList[m_hsvTargetNum].hsvLower, m_hsvTargetList[m_hsvTargetNum].hsvUpper, hsv_mat);

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

            //计算重心
            mu[i] = moments(contours[i], false);
            mc[i] = Point2f(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);
        }

        int max_pos = (int)(max_element(areaVec.begin(), areaVec.end()) - areaVec.begin());
        if (max_pos < (int)contours.size())
        {
            m_area_list.append(areaVec[max_pos]);
            m_hsvCurrentMark = boundingRect(contours[max_pos]);
            m_mark_list.append(m_hsvCurrentMark);
            QPoint pos(mc[max_pos].x, mc[max_pos].y);
            m_center_point_list.append(pos);
        }
    }
    else if (actionStatus == Finished && isReady)  //弯腰动作完成
    {
        QString msg("@Begin HSV:\r\n");
        if (!m_area_list.isEmpty() && !m_mark_list.isEmpty() && !m_center_point_list.isEmpty())
        {
            QList<int> tempList = m_area_list;
            //从小到大排序
            std::sort(m_area_list.begin(), m_area_list.end());
            //选择中值
            int p = 0;
            if (m_area_list.size()%2)
            {
                p = m_area_list.size()/2;
            }
            else
            {
                p = (m_area_list.size()+1)/2;
            }
            int index = tempList.indexOf(m_area_list[p]);
//            qDebug()<< "p: "<< p << "\n"
//                    << "index: " << index << "\n"
//                    << "tempList: "<< tempList << "\n"
//                    << "m_area_list: " << m_area_list;

            m_hsvCurrentMark = m_mark_list[index];
            msg.append(QString("Mark.Rect=%1,%2,%3,%4\r\n")
                       .arg(m_hsvCurrentMark.x)
                       .arg(m_hsvCurrentMark.y)
                       .arg(m_hsvCurrentMark.width)
                       .arg(m_hsvCurrentMark.height));

            m_hsvCenterPoint.setX(m_center_point_list[index].x());
            m_hsvCenterPoint.setY(m_center_point_list[index].y());
            calculateDirection(m_hsvCenterPoint);

            if (m_hsvCurrentMark.width > (int)(m_hsvTargetList[m_hsvTargetNum].maxWidth*g_arrive_ratio))
            {
                if (m_targetType == 0)
                {
                    moveMode = Obstacle;
                }
                else if (m_targetType == 1)
                {
                    moveMode = Wait;
                    m_hsvTargetList[m_hsvTargetNum].state = 1;
                }
                else if (m_targetType == 2)
                {
                    moveMode = ShootAdjust;
                }
            }
            else if (m_hsvCurrentMark.width > (int)(m_hsvTargetList[m_hsvTargetNum].maxWidth*g_access_ratio))
            {
                moveMode = Access;
            }
            else
            {
                moveMode = Track;
            }

            m_area_list.clear();
            m_mark_list.clear();
            m_center_point_list.clear();
        }
        else
        {
            curPosition = Unknown;
            msg.append("Unrecognized color\r\n");
        }

        msg.append("@End\r\n");
        emit sendInfo(msg);  //发送结果给客户端

        if (moveMode == Track)
        {
            trackingTarget();
        }
        else if (moveMode == Access)
        {
            accessTarget();
        }
        else if (moveMode == Wait)
        {
            emit sendInfo(QString("Reach.Target=%1").arg(m_hsvTargetList[m_hsvTargetNum].name));
        }
        else if (moveMode == ShootAdjust)
        {
            adjustShootFootball();
        }
    }
}

void DiscernColor::excuteTurnRound()
{
    if (actionStatus == Finished && isReady && !m_bTurnRoundFinished)  //转身(持续右转直至180度左右)
    {
        static int turnCount = 0;
        if (turnCount < g_turn_round_count)
        {
            setActionStatus(TurnRight_L);
            emit directionChanged(g_right_l_command);
            turnCount++;
            qDebug()<<"start turn round !!! "<< turnCount;
        }
        else
        {
            if (G_Image_Format == "YUV")
            {
                m_yuv_return_list.clear();
                for (int i=m_yuvTargetList.size()-2; i>-1; --i)
                {
                    YUV_Target obj = m_yuvTargetList[i];
                    obj.state = 0;
                    if (i == 0)
                    {
                        obj.type = 1;
                    }
                    m_yuv_return_list << obj;
                }
                m_yuvTargetList = m_yuv_return_list;
            }
            else if (G_Image_Format == "HSV")
            {
                m_hsv_return_list.clear();
                for (int i=m_hsvTargetList.size()-2; i>-1; --i)
                {
                    HSV_Target obj = m_hsvTargetList[i];
                    obj.state = 0;
                    if (i == 0)
                    {
                        obj.type = 1;
                    }
                    m_hsv_return_list << obj;
                }
                m_hsvTargetList = m_hsv_return_list;
            }
            m_bTurnRoundFinished = true;
            turnCount = 0;
            moveMode = Track;
        }
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
    int curIndex;
    if (objList.size() == 1) //当只有一个标记框
    {
        rectX = objList[0]->minX;
        rectY = objList[0]->minY;
        rectW = objList[0]->maxX - rectX;
        rectH = objList[0]->maxY - rectY;
        curIndex = 0;
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
        curIndex = areaVec.size()-1;
    }

    //更新标记框
    currentMark.x = rectX;
    currentMark.y = rectY;
    currentMark.width = rectW;
    currentMark.height = rectH;
    m_yuvCenterPoint.setX(objList[curIndex]->centerX);
    m_yuvCenterPoint.setY(objList[curIndex]->centerY);

    return true;
}

/**
 * @brief     通过标记框的重心位置或者重心判断物体方向
 * @param     pos 中心点或重心坐标
 */

void DiscernColor::calculateDirection(const QPoint &pos)
{
    if ( pos.x() < qRound(g_frame_width*g_horizontal_ratio*g_rotation_range) )
    {
        curPosition = LeftFar;
    }

    else if ( pos.x() >= qRound(g_frame_width*g_horizontal_ratio*g_rotation_range)  &&
              pos.x() <  g_frame_width*g_horizontal_ratio )
    {
        curPosition = LeftNear;
    }

    else if ( pos.x() > g_frame_width*(1.0-g_horizontal_ratio) &&
              pos.x() <=  (g_frame_width - qRound(g_frame_width*g_horizontal_ratio*g_rotation_range)) )
    {
        curPosition = RightNear;
    }
    else if (pos.x() > (g_frame_width - qRound(g_frame_width*g_horizontal_ratio*g_rotation_range)) )
    {
        curPosition = RightFar;
    }
    else
    {
        curPosition = Center;
    }
}

/**
 * @brief     获取当前识别的目标
 * @return    返回当前识别的目标，如果目标都已找到，则返回-1
 */

int DiscernColor::currentTarget() const
{
    if (G_Image_Format == "YUV")
    {
        for (int i=0; i<m_yuvTargetList.size(); ++i)
        {
            if (m_yuvTargetList[i].state == 0)
            {
                return i;
            }
        }
    }
    else if (G_Image_Format == "HSV")
    {
        for (int i=0; i<m_hsvTargetList.size(); ++i)
        {
            if (m_hsvTargetList[i].state == 0)
            {
                return i;
            }
        }
    }

    return -1;
}

/**
 * @brief     根据目标位置，选择动作靠近目标
 */

void DiscernColor::findTarget()
{    
    switch (curPosition)
    {
    case Center: //前进
        setActionStatus(QuickWalk);
        emit startMoveOn(m_nMoveOnTimeCount);
        m_findCount = 0;
        break;
    case LeftFar:  //左转(大)
        setActionStatus(TurnLeft_L);
        emit directionChanged(g_left_l_command);
        m_findCount = 0;
        break;
    case LeftNear: //左转(小)
        setActionStatus(TurnLeft_S);
        emit directionChanged(g_left_s_command);
        m_findCount = 0;
        break;
    case RightNear: //右转(小)
        setActionStatus(TurnRight_S);
        emit directionChanged(g_right_s_command);
        m_findCount = 0;
        break;
    case RightFar:  //右转(大)
        setActionStatus(TurnRight_L);
        emit directionChanged(g_right_l_command);
        m_findCount = 0;
        break;
    case Unknown: //没有识别到颜色,先右转，如果向右转动3次还是找不到就持续左转
        if (m_targetType == 2 && lastActionStatus != StoopDown)
        {
            setActionStatus(StoopDown);
            emit directionChanged(g_stoop_down_command);
            return;
        }
        m_findCount++;
        if (m_findCount > 3)
        {
            setActionStatus(TurnLeft_L);
            emit directionChanged(g_left_l_command);
        }
        else
        {
            setActionStatus(TurnRight_L);
            emit directionChanged(g_right_l_command);
        }
        if (m_findCount > 1000)
            m_findCount = 4;
        break;
    default:
        break;
    }
}

/**
 * @brief     快速靠近目标
 */

void DiscernColor::trackingTarget()
{
    m_nMoveOnTimeCount = g_far_move_on_time;
    findTarget();
}

/**
 * @brief     慢慢靠近目标
 */

void DiscernColor::accessTarget()
{
    m_nMoveOnTimeCount  = g_near_move_on_time;
    findTarget();
}

/**
 * @brief     执行避障动作
 */

void DiscernColor::obstacleAvoidance()
{
    int turn = 0;
    if (G_Image_Format == "YUV")
    {
        turn = m_yuvTargetList[m_yuvTargetNum].turn;
    }
    else if (G_Image_Format == "HSV")
    {
        turn = m_hsvTargetList[m_hsvTargetNum].turn;
    }

    obstacleCount++;
    qDebug()<< "----======start obstacle avoidance: " << obstacleCount;
    if (obstacleCount <= 3) //避障执行的前两次动作，转向
    {
        if(turn == 0) //左边
        {
            setActionStatus(TurnLeft_L);
            emit directionChanged(g_left_l_command);
        }
        else          //右边
        {
            setActionStatus(TurnRight_L);
            emit directionChanged(g_right_l_command);
        }
    }
    else if (obstacleCount == 4) //避障执行的第三次动作,前进
    {
        setActionStatus(QuickWalk);
        emit startMoveOn(g_far_move_on_time);
    }
    else if (obstacleCount > 4 && obstacleCount <= 7) //恢复转向
    {
        if(turn == 0)
        {
            setActionStatus(TurnRight_L);
            emit directionChanged(g_right_l_command);
        }
        else
        {
            setActionStatus(TurnLeft_L);
            emit directionChanged(g_left_l_command);
        }

        if (obstacleCount == 7)
        {
            obstacleCount = 0;
            moveMode = Track;
            if (G_Image_Format == "YUV")
            {
                m_yuvTargetList[m_yuvTargetNum].state = 1;
                emit sendInfo(QString("Reach.Target=%1").arg(m_yuvTargetList[m_yuvTargetNum].name));
            }
            else if (G_Image_Format == "HSV")
            {
                m_hsvTargetList[m_hsvTargetNum].state = 1;
                emit sendInfo(QString("Reach.Target=%1").arg(m_hsvTargetList[m_hsvTargetNum].name));
            }
        }
    }
}

void DiscernColor::shootFootball()
{    
    m_shootActionsFinishedCount++;
    qDebug()<< "----------------=============start shoot !!!"<<m_shootActionsFinishedCount;
    if (m_shootActionsFinishedCount  == 1)
    {
        setActionStatus(QuickWalk);
        emit startMoveOn(g_access_football_time);
    }
    else if (m_shootActionsFinishedCount == 2)
    {
        setActionStatus(ShootFootBall);
        emit directionChanged(g_right_kick_command);
        moveMode = Wait;
        m_shootActionsFinishedCount = 0;
        QString targetName;
        if (G_Image_Format == "YUV")
        {
            targetName = m_yuvTargetList[m_yuvTargetNum].name;
            m_yuvTargetList[m_yuvTargetNum].state = 1;
        }
        else if (G_Image_Format == "HSV")
        {
            targetName = m_hsvTargetList[m_hsvTargetNum].name;
            m_hsvTargetList[m_hsvTargetNum].state = 1;
        }
        emit sendInfo(QString("Reach.Target=%1").arg(targetName));
    }
}

void DiscernColor::adjustShootFootball()
{
    int centerX = 0;
    if (G_Image_Format == "YUV")
    {
        centerX = m_yuvCenterPoint.x();
    }
    else if (G_Image_Format == "HSV")
    {
        centerX = m_hsvCenterPoint.x();
    }
    else
    {
        return;
    }

    if (centerX < qRound(g_frame_width*0.62))
    {
        setActionStatus(LeftShift);
        emit directionChanged(g_left_shift_command);
        moveMode = Access;
    }
    else if (centerX > qRound(g_frame_width*0.72))
    {
        setActionStatus(RightShift);
        emit directionChanged(g_right_shift_command);
        moveMode = Access;
    }
    else
    {
        moveMode = Shoot;
    }
}

/**
 * @brief     记录标记的颜色信息
 * @param     frame  一帧图片
 */

void DiscernColor::addColor(Mat &frame)
{
    int rgbMean[3];
    unsigned char uvRange[2][2];

    cv::Mat frameROI;
    frameROI = frame(cv::Rect(m_select_rect.x(), m_select_rect.y(), m_select_rect.width(), m_select_rect.height()));
    cv::Scalar rgbScalar = cv::mean(frameROI); //计算选中区域的RGB平均值
    rgbMean[0] = rgbScalar[0];
    rgbMean[1] = rgbScalar[1];
    rgbMean[2] = rgbScalar[2];

    m_mark_rgb = QString("Mark.RGB=%1,%2,%3\r\n").arg(rgbMean[1]).arg(rgbMean[2]).arg(rgbMean[0]);

    cv::Mat yuvROI;
    cv::cvtColor(frameROI, yuvROI,CV_RGB2YUV);
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

    memset(colorInfo.channelLUT,0,2*256*sizeof(unsigned int));
    int colorIndex = colorInfo.counter;
    colorIndex = 0;

    colorInfo.meanColor[colorIndex][0] = rgbMean[0];
    colorInfo.meanColor[colorIndex][1] = rgbMean[1];
    colorInfo.meanColor[colorIndex][2] = rgbMean[2];

    colorInfo.channelRange[colorIndex][0][0] = uvRange[0][0];
    colorInfo.channelRange[colorIndex][0][1] = uvRange[0][1];
    colorInfo.channelRange[colorIndex][1][0] = uvRange[1][0];
    colorInfo.channelRange[colorIndex][1][1] = uvRange[1][1];

    for(int i=uvRange[0][0]; i<uvRange[0][1]+1; i++)
        colorInfo.channelLUT[0][i] |= (1<<colorIndex);
    for(int i=uvRange[1][0]; i<uvRange[1][1]+1; i++)
        colorInfo.channelLUT[1][i] |= (1<<colorIndex);
    colorInfo.counter++;
}

/**
 * @brief     计算识别颜色核心函数
 * @param     source  指向图片的内存
 * @param     mask 为true则让该像素点变成平均色
 */

void DiscernColor::segment(Mat &frame, ColorInfo &info, bool mask)
{
    Mat yuv_mat;
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

    unsigned char *source = yuv_mat.data;

    static Object *tmpObj = NULL;
    static std::stack<int> pixelStack;
    static short int *isVisited = new short int[w*h];

    objList.clear();
    memset(isVisited,-1,w*h*sizeof(short int));

    for (int i = 0; i < w*h; i ++) {  //for each big pixel
        if(isVisited[i] == -1){        //-1 means it does not belong to any object yet(including the "not interested" object, which is labeled 0)
            int tmpLabel = GetLabel(info, source, i); //i*3 inside getColor
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
             if(GetLabel(info, source, index) == tmpObj->colorID) {   //has same color as current scanning object
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
//                        source[3*index+1] = colorInfo.meanColor[colorIndex][1];
//                        source[3*index+2] = colorInfo.meanColor[colorIndex][2];
                        source[3*index+1] = info.meanColor[colorIndex][1];
                        source[3*index+2] = info.meanColor[colorIndex][2];
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

unsigned char DiscernColor::GetLabel(ColorInfo &info, unsigned char *source, int pIndex)
{
   return info.channelLUT[0][source[3*pIndex+1]] & info.channelLUT[1][source[3*pIndex+2]];
}
