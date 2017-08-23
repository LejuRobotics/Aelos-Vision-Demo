#include "server.h"
#include <QCoreApplication>

/*************************************************************
 * 定义全局变量,这些变量在global.var.h文件中声明
 *************************************************************/

QString g_serial_name = "/dev/ttyS1";
int g_baud_rate = 9600;
int g_listen_port = 7980;
int g_broadcast_port = 5713;
int g_frame_width = 640;
int g_frame_height = 480;
int g_frame_quality = 50;
double g_horizontal_ratio = 0.4;
double g_object_ratio = 0.6;
double g_rotation_range = 0.25;
int g_time_count = 1500;
int g_forward_command = 1;
int g_quick_back_command = 2;
int g_left_s_command = 3;
int g_right_s_command = 4;
int g_left_l_command = 5;
int g_right_l_command = 6;
int g_stoop_down_command = 7;
int g_ping_router_count = 8;
QString g_robot_number = "AELOS150C00D";
int g_far_move_on_time = 3500;
int g_near_move_on_time = 1500;
double g_arrive_ratio = 0.9;
double g_access_ratio = 0.4;
int g_log_file_switch = 0;

QString G_Image_Display = "Original";
QString G_Image_Format = "HSV";
QString G_Object_Type = "Null";
int G_Go_Back_Flag = 0;
int G_Shoot_Flag = 0;
int G_Football_Radius = 0;

const qint64 LOG_FILE_MAX_SIZE = 5*1024*1024;  //日志文件大小的最大值

/**
 * @brief 打印输出保存到日志文件
 */

void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    static QMutex mutex;
    mutex.lock();

    QString text;
    switch(type)
    {
    case QtDebugMsg:
        text = QString("Debug:");
        break;

    case QtWarningMsg:
        text = QString("Warning:");
        break;

    case QtCriticalMsg:
        text = QString("Critical:");
        break;

//    case QtInfoMsg:
//        text = QString("Information:");
//        break;

    case QtFatalMsg:
        text = QString("Fatal:");
    }

    QString context_info = QString("File:(%1) Line:(%2)").arg(QString(context.file)).arg(context.line);
    QString current_date_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd");
    QString current_date = QString("(%1)").arg(current_date_time);
    QString message = QString("%1 %2 %3 %4").arg(text).arg(context_info).arg(msg).arg(current_date);

    QFile file("log.txt");
    file.open(QIODevice::WriteOnly | QIODevice::Append);
    if (file.size() > LOG_FILE_MAX_SIZE)  //日志文件大小超过指定大小，则清除文件内容重新写入
    {
        file.close();
        file.open(QIODevice::WriteOnly);
        file.close();
        file.open(QIODevice::WriteOnly | QIODevice::Append);
    }
    QTextStream text_stream(&file);
    text_stream << message << "\r\n";
    file.flush();
    file.close();

    mutex.unlock();
}


/**
 * @brief     读取配置文件
 */

void readConfigFile()
{
    QSettings iniReader("setup.ini",QSettings::IniFormat);
    g_serial_name = iniReader.value("Serial/name").toString();
    g_baud_rate = iniReader.value("Serial/baudRate").toInt();
    g_frame_width = iniReader.value("Resolution/width").toInt();
    g_frame_height = iniReader.value("Resolution/height").toInt();
    g_listen_port = iniReader.value("Port/tcpPort").toInt();
    g_broadcast_port = iniReader.value("Port/udpPort").toInt();
    g_frame_quality = iniReader.value("Debug/frameQuality").toInt();
    g_horizontal_ratio = iniReader.value("Debug/horizontalRatio").toDouble();
    g_object_ratio = iniReader.value("Debug/objectRatio").toDouble();
    g_rotation_range = iniReader.value("Debug/rotationRange").toDouble();
    g_time_count = iniReader.value("Debug/timeCount").toInt();
    g_forward_command = iniReader.value("Debug/forwardCommand").toInt();
    g_quick_back_command = iniReader.value("Debug/quickBackCommand").toInt();
    g_left_s_command = iniReader.value("Debug/sLeftCommand").toInt();
    g_right_s_command = iniReader.value("Debug/sRightCommand").toInt();
    g_left_l_command = iniReader.value("Debug/lLeftCommand").toInt();
    g_right_l_command = iniReader.value("Debug/lRightCommand").toInt();
    g_stoop_down_command = iniReader.value("Debug/stoopDownCommand").toInt();
    g_ping_router_count = iniReader.value("Debug/pingRouterCount").toInt();
    g_robot_number = iniReader.value("Robot/No").toString();
    g_far_move_on_time = iniReader.value("Debug/moveOnFarTime").toInt();
    g_near_move_on_time = iniReader.value("Debug/moveOnNearTime").toInt();
    g_arrive_ratio = iniReader.value("Debug/arriveRatio").toDouble();
    g_access_ratio = iniReader.value("Debug/accessRatio").toDouble();
    g_log_file_switch = iniReader.value("Debug/logFileSwitch").toInt();

    qDebug()<< "serialName: "<< g_serial_name << "\n"
            << "baudRate: " << g_baud_rate << "\n"
            << QString("resolution: %1x%2").arg(g_frame_width).arg(g_frame_height) << "\n"
            << "g_listen_port: " << g_listen_port << "\n"
            << "g_broadcast_port: " << g_broadcast_port << "\n"
            << "g_frame_quality: " << g_frame_quality << "\n"
            << "g_horizontal_ratio: " << g_horizontal_ratio << "\n"
            << "g_object_ratio: " << g_object_ratio << "\n"
            << "g_rotation_range: " << g_rotation_range << "\n"
            << "g_time_count: " << g_time_count << "\n"
            << "g_forward_command: " << g_forward_command << "\n"
            << "g_quick_back_command: " << g_quick_back_command << "\n"
            << "g_left_s_command: " << g_left_s_command << "\n"
            << "g_right_s_command: " << g_right_s_command << "\n"
            << "g_left_l_command: " << g_left_l_command << "\n"
            << "g_right_l_command: " << g_right_l_command << "\n"
            << "g_stoop_down_command: " << g_stoop_down_command << "\n"
            << "g_ping_router_count: " << g_ping_router_count << "\n"
            << "g_robot_number: " << g_robot_number << "\n"
            << "g_far_move_on_time: " << g_far_move_on_time << "\n"
            << "g_near_move_on_time: " << g_near_move_on_time << "\n"
            << "g_arrive_ratio: " << g_arrive_ratio << "\n"
            << "g_access_ratio: " << g_access_ratio << "\n"
            << "g_log_file_switch: " << g_log_file_switch << "\n";
}


/**
 * @brief 主函数
 */

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    readConfigFile();

    //注册MessageHandler,监听打印输出保存到日志文件
    if (g_log_file_switch)
    {
        qInstallMessageHandler(outputMessage);
    }

    Server s;
    s.startListen();

    return a.exec();
}
