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

/*************************************************************
 * 声明全局变量，在server.cpp文件中定义，这些变量可以在配置文件中更改
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
extern int g_left_s_command;              /**< 左转指令(小幅度) */
extern int g_right_s_command;             /**< 右转指令(小幅度) */
extern int g_left_l_command;              /**< 左转指令(大幅度) */
extern int g_right_l_command;             /**< 右转指令(大幅度) */
extern int g_connect_net_wait_time;       /**< 程序启动后ping路由器的延迟时长 */
extern QString g_robot_number;            /**< 机器人编号 */
extern int g_stop_move_on_time_count;     /**< 发送停止持续前进指令的延时时长 */

#endif // GLOBAL_VAR_H
