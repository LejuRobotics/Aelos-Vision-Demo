/**
 * @file       server.cpp
 * @version    1.0
 * @date       2017年07月08日
 * @author     C_Are
 * @copyright  Leju
 *
 * @brief      服务器，Server类的cpp文件
 * @details    服务器类，集合tcp服务器,图像处理，串口操作等
 */

#include "server.h"

/*************************************************************
 * 定义全局变量
 *************************************************************/

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
int g_connect_net_wait_time = 40000;  //机器人重启后隔40s时间ping路由器判断是否连接上路由器

/**
 * @brief     Server类的构造函数
 * @details   初始化
 */

Server::Server()
{
    isOPenSerial = false;
    m_connection_count = 0;

    readConfigFile();

    QNetworkProxyFactory::setUseSystemConfiguration(false);
    tcpServer = new QTcpServer(this);
    m_timer = new QTimer(this);
    serialPort = new SerialPort(this);
    discernColor = new DiscernColor(this);
    videoControl = new VideoControl(this);

    connect(tcpServer,SIGNAL(newConnection()),this,SLOT(onNewConnection()));
    connect(serialPort, SIGNAL(actionFinished()), this, SLOT(onActionFinished()));
    connect(videoControl,SIGNAL(sendInfo(QString)), this, SLOT(onSendInfo(QString)));
    connect(videoControl, SIGNAL(sendFrame(QImage*)), discernColor, SLOT(readFrame(QImage*)));
    connect(discernColor,SIGNAL(sendInfo(QString)), this, SLOT(onSendInfo(QString)));
    connect(discernColor,SIGNAL(directionChanged(int)),this,SLOT(onDirectionChanged(int)));
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
}

/**
 * @brief     Server类的析构函数
 */

Server::~Server()
{
    if (tcpSocket->isOpen())
    {
        tcpSocket->abort();
    }
}

/**
 * @brief     tcp服务器开始监听，并尝试打开串口
 */

void Server::startListen()
{
    if (!tcpServer->listen(QHostAddress::Any,g_listen_port))
    {
        qDebug()<<tcpServer->errorString();
        return;
    }
    qDebug("Start listen %d\n", g_listen_port);

    isOPenSerial = serialPort->openSerilPort(); //打开串口

    QTimer::singleShot(g_connect_net_wait_time, this, SLOT(pingRouter())); //g_connect_net_wait_time 毫秒后获取ip

    onTimeout(); //先广播一次的ip
    m_timer->start(5000);  //每隔5秒广播一次
}

/**
 * @brief     当tcp服务器有新的连接的执行的槽函数
 * @details   通过m_connection_count控制连接个数，让服务器只连接一个客户端进行操作,当服务器已经有连接的时候会通知其它客户端
 */

void Server::onNewConnection()
{
    m_connection_count++;
    if (m_connection_count == 1) //同时只连接一个客户端
    {
        tcpSocket = tcpServer->nextPendingConnection();
        connect(tcpSocket,SIGNAL(readyRead()),this,SLOT(onSocketRead()));
        connect(tcpSocket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(disPlayError(QAbstractSocket::SocketError)));

        m_client_ip = tcpSocket->peerAddress().toString();
        WriteMsg(QString("Connect to server successful\r\n").toUtf8()); //连接成功，返回信息
    }
    else //如果有多个客户端连接，告诉这些客户端已连接的客户端ip，不为它们服务
    {
        QTcpSocket *tempSocket = tcpServer->nextPendingConnection();
        connect(tempSocket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(disPlayError(QAbstractSocket::SocketError)));

        QString msg = QString("%1 is connecting to the server\r\n").arg(m_client_ip);
        QByteArray outBlock;
        QDataStream out(&outBlock,QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_5_3);
        out<<msg.toUtf8();
        tempSocket->write(outBlock);
    }
}

/**
 * @brief     接收客户端的tcp消息
 * @details   客户端会发送“95f41ce1”进行身份认证，服务器返回“676f7a75"进行确认
 */

void Server::onSocketRead()
{
    QByteArray receiveByteArray;
    QDataStream in(tcpSocket);
    in.setVersion(QDataStream::Qt_5_3);
    in >> receiveByteArray;
    QString receive_data = QString::fromUtf8(receiveByteArray);
    qDebug()<< "receive: "<< receive_data;
    if (receive_data == "95f41ce1")  //身份验证
    {
        WriteMsg(QString("676f7a75").toUtf8());
    }
    else if (receive_data.startsWith("Start.Running"))  //确认完毕
    {
        QString retMsg("From server: \r\n");
        retMsg.append(QString("Camera.Resolution=%1x%2\r\n").arg(g_frame_width).arg(g_frame_height));
        retMsg.append(QString("Area.Position=%1,%2\r\n").arg(g_horizontal_ratio).arg(g_rotation_range));
        retMsg.append(QString("Open %1 %2 !\r\n").arg(g_serial_name).arg(isOPenSerial ? "successfully" : "failed"));
        retMsg.append("End\r\n");
        WriteMsg(retMsg.toUtf8()); //连接成功，返回信息

        videoControl->openUrl(tcpSocket->peerAddress().toString()); //开启线程，读取摄像头
    }
    else
    {
        if (parseData(receive_data))
        {
            m_result_msg = QString("send ok (\"%1\")\r\n").arg(m_result_msg);
        }
        else
        {
            m_result_msg = QString("send error (\"%1\")\r\n").arg(m_result_msg);
        }
        //接收成功，返回结果
        WriteMsg(m_result_msg.toUtf8());
    }
}

/**
 * @brief     解析已接收的客户端的指令
 * @param     msg 收到的消息
 */

bool Server::parseData(const QString &msg)
{
    bool isOk = false;
    if (msg.isEmpty())
        return isOk;

    m_result_msg = msg;
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
            discernColor->setSelectRect(temp);
            isOk = true;
        }
    }
    else if (msg.startsWith("set Stop.Enable="))
    {
        int ret = msg.mid(msg.indexOf("=")+1).toInt();
        discernColor->setStopEnable(ret);
        isOk = true;
    }
    else if(msg.startsWith("set Robot.Action="))
    {
        QString strVal = msg.mid(msg.indexOf("=")+1);
        if (strVal == "manual")
        {
            discernColor->setActionMode(0);
            isOk = true;
        }
        else if (strVal == "auto")
        {
            discernColor->setActionMode(1);
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
//            else if (msgList[1] == "forward")
//            {
//                unsigned char serialnum[] = {0xd1};
//                char* buf = (char*)(&serialnum);
//                serialPort->sendMsg(buf,1);
//            }
//            else if (msgList[1] == "stop")
//            {
//                unsigned char serialnum[] = {0xd0};
//                char* buf = (char*)(&serialnum);
//                serialPort->sendMsg(buf,1);
//            }
        }
    }
    else if (msg.startsWith("set Wifi.Settings="))
    {
        QStringList msgList = msg.mid(msg.indexOf("=")+1).split(",");
        if (msgList.size() == 2)
        {
            modifyNetworkFile(msgList[0],msgList[1]);
            isOk = true;
        }
    }
    else if (msg.startsWith("set Image.Format="))
    {
        QString image_format = msg.mid(msg.indexOf("=")+1);
        if (image_format == "RGB" || image_format == "YUV")
        {
            videoControl->setImageFormat(image_format);
            isOk = true;
        }
    }
    else if (msg.startsWith("set Color.Channel.Y="))
    {
        QString channelY = msg.mid(msg.indexOf("=")+1);
        if (!channelY.isEmpty())
        {
            int val = channelY.toInt();
            if (val < 0 || val > 255)
                return false;

            g_color_channel_Y = val;
            isOk = true;
        }
    }
    else if (msg.startsWith("set Color.Brightness="))
    {
        QString brightness = msg.mid(msg.indexOf("=")+1);
        if (!brightness.isEmpty())
        {
            videoControl->setBrightness(brightness.toDouble());
        }
    }
    else if (msg.startsWith("set Color.Contrast="))
    {
        QString contrast = msg.mid(msg.indexOf("=")+1);
        if (!contrast.isEmpty())
        {
            videoControl->setContrast(contrast.toInt());
        }
    }

    return isOk;
}

/**
 * @brief     向客户端发送消息
 * @param     msg 需要发送的消息
 */

void Server::WriteMsg(const QByteArray &msg)
{
    QByteArray outBlock;
    QDataStream out(&outBlock,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_3);
    out<<msg;
    tcpSocket->write(outBlock);
}

/**
 * @brief     与客户端断开连接后，停止发送图像
 */

void Server::disPlayError(QAbstractSocket::SocketError)
{
    QTcpSocket *socketObj = qobject_cast<QTcpSocket*>(sender());
    qDebug()<<socketObj->errorString()<<"("<<socketObj->peerAddress().toString()<<")";
    socketObj->abort();
    if (socketObj == tcpSocket)
    {
        videoControl->stop();
    }
    m_connection_count--;
}

/**
 * @brief     连接directionChanged(int)信号的槽函数，根据val向串口发送消息，控制机器人前进
 * @param     val 指定机器人执行的动作文件,比如3指的是3.act,我们在3.act文件里面执行的是小幅度左转动作
 */

void Server::onDirectionChanged(int val)
{
    WriteSerial(val);
}

/**
 * @brief     连接actionFinished()信号的槽函数,更新机器人动作状态，通过定时器进行下一次动作前的延迟
 */

void Server::onActionFinished()
{   
    discernColor->setActionStatus(DiscernColor::Finished);
    QTimer::singleShot(g_time_count, this, SLOT(readyNextAction()));
}


/**
 * @brief     定时器超时，机器人可以进行下一次动作
 */

void Server::readyNextAction()
{
    discernColor->setActionReady();
}

/**
 * @brief     连接sendInfo(QString)信号的槽函数,向客户端发送消息,比如标记框位置，一些错误提示等
 */

void Server::onSendInfo(const QString &msg)
{
    WriteMsg(msg.toUtf8());
}

/**
 * @brief     定时器超时，广播服务器IP地址
 */

void Server::onTimeout()
{
    QString localIp = getLocalIP4Address();
    if (!localIp.isEmpty() && !m_connection_count)
    {
        QUdpSocket udpSocket;
        QByteArray datagram;
        QDataStream out(&datagram, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_5_3);
        out << QString("IP").toUtf8() << localIp.toUtf8();
        udpSocket.writeDatagram(datagram,QHostAddress::Broadcast,g_broadcast_port);
    }
}

/**
 * @brief     读取配置文件
 */

void Server::readConfigFile()
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
    g_connect_net_wait_time = iniReader.value("Debug/waitConnectNetTime").toInt();

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
            << "g_right_l_command: " << g_right_l_command << "\n"
            << "g_connect_net_wait_time: " << g_connect_net_wait_time << "\n";
}

/**
 * @brief     修改nanopi上/etc/network/interfaces文件，连接指定的wifi账号密码
 * @param     id wifi账号
 * @param     password wifi密码
 */

void Server::modifyNetworkFile(const QString &id, const QString &password)
{
    QFile file("./network/wifi/interfaces");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qDebug("open file failed !");
        return;
    }

    QString newData = QString("# interfaces(5) file used by ifup(8) and ifdown(8)\n"
                              "# Include files from /etc/network/interfaces.d:\n"
                              "source-directory /etc/network/interfaces.d\n"
                              "\n"
                              "auto lo\n"
                              "iface lo inet loopback\n"
                              "\n"
                              "allow-hotplug wlan0\n"
                              "iface wlan0 inet dhcp\n"
                              "wpa-ssid \"%1\"\n"
                              "wpa-psk \"%2\"\n").arg(id).arg(password);

    file.write(newData.toUtf8());
    file.close();

    QSettings iniReader("network/wifi.ini",QSettings::IniFormat);
    iniReader.setValue("Configure/mode",0);

    qDebug()<<"--===start reset network===---";
    system("network/wifi/wifi_setup.sh");
}

/**
 * @brief     通过SerialPort类的实例向串口发送消息
 * @param     val 16进制的ASCII码，比如1，机器人会通过1.act动作文件执行对应动作,其中：
 *            1.act--------->快走
 *            2.act--------->快退
 *            3.act--------->左转（小）
 *            4.act--------->右转（小）
 *            5.act--------->左转（大）
 *            6.act--------->右转（大）
 *            7、8，9保留，后面用来执行其他动作
 */

void Server::WriteSerial(int val)
{
    int bufferSize = 9;
    unsigned char serialnum[] = {0xd3,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x31};
//    unsigned char serialnum[] = {0xd3,0x00,0x00,0x00,0x00,0x05,0x00,0x00,0x31,0x21,0x61,0x63,0x74};
    bool isOk = false;
    switch (val)
    {
    case 1:  //快走
    {
        serialnum[8] = 0x31;
        isOk = true;
    }
        break;
    case 2:  //快退
    {
        serialnum[8] = 0x32;
        isOk = true;
    }
        break;
    case 3:  //左转（小）
    {
        serialnum[8] = 0x33;
        isOk = true;
    }
        break;
    case 4:  //右转（小）
    {
        serialnum[8] = 0x34;
        isOk = true;
    }
        break;
    case 5:  //左转（大）
    {
        serialnum[8] = 0x35;
        isOk = true;
    }
        break;
    case 6:  //右转（大）
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
//        QTimer::singleShot(2000, this, SLOT(onActionFinished())); //测试，判断动作timeCount毫秒后完成
    }
}

/**
 * @brief     通过ping路由器ip,判断机器人是否连接上路由器
 * @details   如果获取不到ip4地址，则设置机器人上的网卡为热点模式,如果连接获取到ip4但不能ping通路由器，也设置为热点模式，
 *            热点模式下，机器人的ip固定为192.168.8.1，目前(2017-07-08)热点为机器人编号，密码为12345678
 */

void Server::pingRouter()
{
    QString localIp = getLocalIP4Address();
    qDebug()<< "localIp: " << localIp;
    QSettings iniReader("network/wifi.ini",QSettings::IniFormat);
    QString wifi_mode = iniReader.value("Configure/mode").toString();
    if (localIp.isEmpty())
    {
        if (wifi_mode == "0")
        {
            qDebug()<< "Cannot get ip address"
                    << "\n"
                    << "---===start reset to Wifi AP mode===---";
            iniReader.setValue("Configure/mode",1);
            system("network/wifi_ap/ap_setup.sh"); //excute shell, replace network file (/etc/network/interfaces) and reboot,
                                                  //if successful, nanopi will set to Wifi-Ap mode
        }
    }
    else
    {
        QString routerIp = localIp.mid(0,localIp.lastIndexOf(".")+1) + "1";
        qDebug()<< "routerIp: "<< routerIp;
        QProcess *cmd = new QProcess;
#ifdef Q_OS_WIN
        QString strArg = "ping " + routerIp + " -n 1 -w 200";
#else
        QString strArg = "ping -s 1 -c 1 " + routerIp;
#endif
        cmd->start(strArg);
        cmd->waitForReadyRead();
        cmd->waitForFinished();
        QByteArray ba = cmd->readAll();
        qDebug()<<"--==>Ret: " << QString::fromLocal8Bit(ba);

        delete cmd;
        cmd = NULL;

        if (ba.isEmpty())
        {
            if (wifi_mode == "0")
            {
                qDebug()<< "Cannot Ping through router"
                        << "\n"
                        << "---===start reset to Wifi AP mode===---";
                iniReader.setValue("Configure/mode",1);
                system("network/wifi_ap/ap_setup.sh");
            }
        }
    }
}

/**
 * @brief     获取机器人nanopi上的ip
 * @return    如果已分配到ip，则返回该ip，否则返回一个空的QString()
 */

QString Server::getLocalIP4Address() const
{
    QList<QNetworkInterface> interfaceList = QNetworkInterface::allInterfaces();
    foreach(QNetworkInterface interfaceItem, interfaceList)
    {
        if(interfaceItem.flags().testFlag(QNetworkInterface::IsUp)
                &&interfaceItem.flags().testFlag(QNetworkInterface::IsRunning)
                &&interfaceItem.flags().testFlag(QNetworkInterface::CanBroadcast)
                &&interfaceItem.flags().testFlag(QNetworkInterface::CanMulticast)
                &&!interfaceItem.flags().testFlag(QNetworkInterface::IsLoopBack)
                &&interfaceItem.hardwareAddress()!="00:50:56:C0:00:01"
                &&interfaceItem.hardwareAddress()!="00:50:56:C0:00:08")
        {
            QList<QNetworkAddressEntry> addressEntryList=interfaceItem.addressEntries();
            foreach(QNetworkAddressEntry addressEntryItem, addressEntryList)
            {
                if(addressEntryItem.ip().protocol()==QAbstractSocket::IPv4Protocol)
                {
                    return addressEntryItem.ip().toString();
                }
            }
        }
    }
    return QString();
}
