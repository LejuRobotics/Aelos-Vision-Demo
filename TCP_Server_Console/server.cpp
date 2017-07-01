#include "server.h"
#include <QDebug>
#include <QNetworkProxyFactory>

QString g_serial_name = "/dev/ttyS1"; //串口名称
int g_baud_rate = 9600;     //波特率
int g_listen_port = 7980;   //接收指令的端口号(服务端，监听)
int g_broadcast_port = 5713; //发送图片的UDP端口号
int g_frame_width = 640;    //图片宽度
int g_frame_height = 480;   //图片高度
int g_frame_quality = 50;   //图片质量
double g_horizontal_ratio = 0.4; //中间区域的划分比例，值越大，区域越小
double g_object_ratio = 0.6;  //物体标记框占中间区域的比例,值越大，越靠近中间区域
double g_rotation_range = 0.25; //控制标记框的x坐标位置在这个比例位置内用小幅度旋转，否则用大幅度
int g_time_count = 1500;    //完成一个动作以后的延迟时间
int g_forward_command = 1;  //前进的指令
int g_left_s_command = 3;   //左转指令(小幅度)
int g_right_s_command = 4;  //右转指令(小幅度)
int g_left_l_command = 5;   //左转指令(大幅度)
int g_right_l_command = 6;  //右转指令(大幅度)

server::server()
{
    isOPenSerial = false;

    readConfigFile();

    serialPort = new SerialPort(this);

    videoControl = new VideoControl(this);
    connect(videoControl,SIGNAL(markChanged(bool)),this,SLOT(onMarkChanged(bool)));
    connect(videoControl,SIGNAL(directionChanged(int)),this,SLOT(onDirectionChanged(int)));
    connect(videoControl,SIGNAL(sendInfo(QString)), this, SLOT(onSendInfo(QString)));

    QNetworkProxyFactory::setUseSystemConfiguration(false);
    tcpServer = new QTcpServer(this);
    connect(tcpServer,SIGNAL(newConnection()),this,SLOT(onNewConnection()));

    connect(serialPort, SIGNAL(actionFinished()), this, SLOT(onActionFinished()));

    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
}

server::~server()
{
    if (tcpSocket->isOpen())
    {
        tcpSocket->abort();
    }
}

//开始监听
void server::startListen()
{
    if (!tcpServer->listen(QHostAddress::Any,g_listen_port))
    {
        qDebug()<<tcpServer->errorString();
        return;
    }
    qDebug("Start listen %d\n", g_listen_port);

    isOPenSerial = serialPort->openSerilPort(); //打开串口
}

bool server::parseData(const QString &msg)
{
    bool isOk = false;
    if (msg.isEmpty())
        return isOk;

    if (msg.startsWith("set Select.Rect="))
    {
        QStringList posList = msg.mid(msg.indexOf("=")+1).split(",");
        if (posList.size() == 4)
        {
            QRect temp;
            temp.setX(posList[0].toInt());
            temp.setY(posList[1].toInt());
            temp.setWidth(posList[2].toInt());
            temp.setHeight(posList[3].toInt());
            videoControl->setSelectRect(temp);
            isOk = true;
        }
    }
    else if(msg.startsWith("set Robot.Action="))
    {
        QString strVal = msg.mid(msg.indexOf("=")+1);
        if (strVal == "manual")
        {
            videoControl->actionMode = 0;
            videoControl->isReady = false;
            isOk = true;
        }
        else if (strVal == "auto")
        {
            videoControl->actionMode = 1;
            videoControl->isReady = true;
            isOk = true;
        }
    }
    else if (msg.startsWith("Move"))
    {
        QStringList msgList = msg.split(",");
        if (msgList.size() == 2)
        {
            if (msgList[1] == "1" || msgList[1] == "2" ||
                    msgList[1] == "3" || msgList[1] == "4" ||
                    msgList[1] == "5" || msgList[1] == "6" ||
                    msgList[1] == "7" || msgList[1] == "8" ||
                    msgList[1] == "9")
            {
                onDirectionChanged(msgList[1].toInt());
                isOk = true;
            }
        }
    }

    return isOk;
}

void server::WriteMsg(const QByteArray &msg)
{
    QByteArray outBlock;
    QDataStream out(&outBlock,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_3);

    out<<(qint64)0;
    out<<msg;
    out.device()->seek(0);
    out<<(qint64)(outBlock.size()-sizeof(qint64));

    tcpSocket->write(outBlock);
}

//当有新的socket连接
void server::onNewConnection()
{
    tcpSocket = tcpServer->nextPendingConnection();
    connect(tcpSocket,SIGNAL(readyRead()),this,SLOT(onSocketRead()));
    connect(tcpSocket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(disPlayError(QAbstractSocket::SocketError)));

    QString retMsg("From server: \r\n");
    retMsg.append(QString("Camera.Resolution=%1x%2\r\n").arg(g_frame_width).arg(g_frame_height));
    retMsg.append(QString("Area.Position=%1,%2\r\n").arg(g_horizontal_ratio).arg(g_rotation_range));
    retMsg.append(QString("Open %1 %2 !\r\n").arg(g_serial_name).arg(isOPenSerial ? "successfully" : "failed"));
    retMsg.append("End\r\n");
    WriteMsg(retMsg.toUtf8()); //连接成功，返回信息

    videoControl->openUrl(tcpSocket->peerAddress().toString()); //开启线程，读取摄像头
}

//接收数据
void server::onSocketRead()
{   
    QString receive_data,result_data;
    receive_data = QString(tcpSocket->readAll());
    qDebug()<< "receive: "<< receive_data;
    if (parseData(receive_data))
    {
        result_data = QString("send successfully !");
    }
    else
    {
        result_data = QString("Error: invalid commands !");
    }
    //接收成功，返回结果
    WriteMsg(result_data.toUtf8());
}

//打印错误信息
void server::disPlayError(QAbstractSocket::SocketError)
{
    qDebug()<<tcpSocket->errorString();
    tcpSocket->close();
    videoControl->stop();
}

void server::onMarkChanged(bool flag)
{    
    QString result_data;
    if (flag)
    {
        result_data = QString("get Mark.Rect=%1,%2,%3,%4")
                .arg(videoControl->currentMark.x())
                .arg(videoControl->currentMark.y())
                .arg(videoControl->currentMark.width())
                .arg(videoControl->currentMark.height());
    }
    else
    {
        result_data = QString("cannot find Mark");
    }

    WriteMsg(result_data.toUtf8());
}

void server::onDirectionChanged(int val)
{
    int bufferSize = 9;
    unsigned char serialnum[] = {0xd3,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x31};
//    unsigned char serialnum[] = {0xd3,0x00,0x00,0x00,0x00,0x05,0x00,0x00,0x31,0x21,0x61,0x63,0x74};
    bool isOk = false;
    switch (val)
    {
    case 1:
    {
        serialnum[8] = 0x31;
        isOk = true;
    }
        break;
    case 2:
    {
        serialnum[8] = 0x32;
        isOk = true;
    }
        break;
    case 3:
    {
        serialnum[8] = 0x33;
        isOk = true;
    }
        break;
    case 4:
    {
        serialnum[8] = 0x34;
        isOk = true;
    }
        break;
    case 5:
    {
        serialnum[8] = 0x35;
        isOk = true;
    }
        break;
    case 6:
    {
        serialnum[8] = 0x36;
        isOk = true;
    }
        break;
    case 7:
    {
        serialnum[8] = 0x37;
        isOk = true;
    }
        break;
    case 8:
    {
        serialnum[8] = 0x38;
        isOk = true;
    }
        break;
    case 9:
    {
        serialnum[8] = 0x39;
        isOk = true;
    }
        break;
    default:
        break;
    }

    if (isOk)
    {
        qDebug("start action: move,%d",val);
        char* buf = (char*)(&serialnum);
        serialPort->sendMsg(buf,bufferSize);
//        QTimer::singleShot(g_time_count, this, SLOT(onActionFinished())); //测试，判断动作timeCount毫秒后完成
    }
}

void server::onActionFinished()
{   
    videoControl->setRebotStatus(VideoControl::Finished);
    m_timer->start(g_time_count);
}

void server::onTimeout()
{
    videoControl->isReady = true;
}

void server::onSendInfo(const QString &msg)
{
    WriteMsg(msg.toUtf8());
}

void server::readConfigFile()
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
    g_left_s_command = iniReader.value("Debug/sLeftCommand").toInt();
    g_right_s_command = iniReader.value("Debug/sRightCommand").toInt();
    g_left_l_command = iniReader.value("Debug/lLeftCommand").toInt();
    g_right_l_command = iniReader.value("Debug/lRightCommand").toInt();

    qDebug()<< " serialName: "<< g_serial_name << "\n"
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
            << "g_left_s_command: " << g_left_s_command << "\n"
            << "g_right_s_command: " << g_right_s_command << "\n"
            << "g_left_l_command: " << g_left_l_command << "\n"
            << "g_right_l_command: " << g_right_l_command << "\n";
}
