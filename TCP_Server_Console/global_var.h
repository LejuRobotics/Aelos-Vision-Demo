#ifndef GLOBAL_VAR_H
#define GLOBAL_VAR_H

/********************************
 * 声明一些全局变量，便于后面调试
 ********************************/

#include <QString>

//以下参数可以在setup.ini文件里面设置
extern QString g_serial_name; //串口名称
extern int g_baud_rate;     //波特率
extern int g_listen_port;   //接收指令的TCP端口号(服务端，监听)
extern int g_broadcast_port; //发送图片的UDP端口号
extern int g_frame_width;    //图片宽度
extern int g_frame_height;   //图片高度
extern int g_frame_quality;  //图片质量
extern double g_horizontal_ratio; //中间区域的划分比例，值越大，区域越小
extern double g_object_ratio;  //物体标记框占中间区域的比例,值越大，越靠近中间区域
extern double g_rotation_range; //控制标记框的x坐标位置在这个比例位置内用小幅度旋转，否则用大幅度
extern int g_time_count;    //完成一个动作以后的延迟时间
extern int g_forward_command;  //快走的指令
extern int g_left_s_command;   //左转指令(小幅度)
extern int g_right_s_command;  //右转指令(小幅度)
extern int g_left_l_command;   //左转指令(大幅度)
extern int g_right_l_command;  //右转指令(大幅度)

#endif // GLOBAL_VAR_H
