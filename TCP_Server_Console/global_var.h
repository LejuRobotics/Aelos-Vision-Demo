/**
 * @file       global_var.h
 * @version    1.0
 * @date       2017年07月08日
 * @author     C_Are
 * @copyright  Leju
 *
 * @brief      全局变量的声明类，预编译文件
 * @details    这些全局变量可在setup.ini配置文件中更改，用来方便程序的调试
 */

#ifndef GLOBAL_VAR_H
#define GLOBAL_VAR_H

/*************************************************************
 * 以下头文件都是预编译
 *************************************************************/

#include <QNetworkProxyFactory>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QNetworkInterface>
#include <QThread>
#include <QImage>
#include <QBuffer>
#include <QMutex>
#include <QQueue>
#include <QTimer>
#include <QTime>
#include <QSettings>
#include <QFile>
#include <QProcess>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QCryptographicHash>
#include <stack>

/*************************************************************
 * 声明全局变量，在main.cpp文件中定义，这些变量可以在配置文件中更改
 *************************************************************/

extern QString g_serial_name;             /**< 串口名称 */
extern int g_baud_rate;                   /**< 波特率 */
extern int g_listen_port;                 /**< TCP监听端口号 */
extern int g_broadcast_port;              /**< 发送图片的UDP端口号 */
extern int g_frame_width;                 /**< 图片宽度 */
extern int g_frame_height;                /**< 图片高度 */
extern int g_frame_quality;               /**< 图片质量 */
extern double g_horizontal_ratio;         /**< 中间区域的划分比例，值越大，区域越小 */
extern double g_object_ratio;             /**< 物体标记框占中间区域的比例,值越大，越靠近中间区域 */
extern double g_rotation_range;           /**< 控制标记框的x坐标位置在这个比例位置内用小幅度旋转，否则用大幅度 */
extern int g_time_count;                  /**< 完成一个动作以后的延迟时间 */
extern int g_forward_command;             /**< 快走的指令 */
extern int g_quick_back_command;          /**< 快退的指令 */
extern int g_left_s_command;              /**< 左转指令(小幅度) */
extern int g_right_s_command;             /**< 右转指令(小幅度) */
extern int g_left_l_command;              /**< 左转指令(大幅度) */
extern int g_right_l_command;             /**< 右转指令(大幅度) */
extern int g_stoop_down_command;          /**< 弯腰 */
extern int g_ping_router_count;           /**< 程序启动后ping路由器的次数，每次间隔5秒，如果ping8次（也就是40s）,无法读到数据，则重启设为热点模式 */
extern QString g_robot_number;            /**< 机器人编号 */
extern int g_far_move_on_time;            /**< 离目标较远的时候，快走的时长 */
extern int g_near_move_on_time;           /**< 离目标较近的时候，快走的时长 */
extern double g_arrive_ratio;             /**< 判读到达指定目标位置的标记框的宽的比例 */
extern double g_access_ratio;             /**< 当前标记框的宽度大于记录的标记框宽度乘以这个比例，则执行慢慢靠近的动作模式 */
extern int g_log_file_switch;             /**< 是否保存程序打印日志，0否，1是 */


/*************************************************************
 * 声明全局变量，方便多个文件中使用
 *************************************************************/
extern QString G_Image_Display;           /**< 图片显示类型 */
extern QString G_Image_Format;            /**< 识别颜色的图片格式 */
extern QString G_Object_Type;             /**< 物体类型 */
extern int G_Go_Back_Flag;                /**< 完成所有目标后返回到初始目标的标志 0否，1是*/
extern int G_Shoot_Flag;                  /**< 是否让机器人寻找足球并进行射门动作 0否，1是*/
extern int G_Football_Radius;             /**< 可以进行射门位置的足球半径 */

#endif // GLOBAL_VAR_H
