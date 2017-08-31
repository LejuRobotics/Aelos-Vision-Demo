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

/**
 * @brief     Server类的构造函数
 * @details   初始化
 */

Server::Server()
{
    isOPenSerial = false;
    m_connection_count = 0;
    mplayer = NULL;
    m_pingCount = 0;
    m_bIsConnectRounter = false;
    m_bIsReady = true;
    m_bufferReadSize = 0;
    m_bufferTotalSize = 0;

    QNetworkProxyFactory::setUseSystemConfiguration(false);
    tcpServer = new QTcpServer(this);
    m_timer = new QTimer(this);
    serialPort = new SerialPort(this);
    discernColor = new DiscernColor(this);
    videoControl = new VideoControl(this);
    mplayer = new QProcess(this);
    m_timer_2 = new QTimer(this);
    m_cmdTimer = new QTimer(this);

    connect(tcpServer,SIGNAL(newConnection()),this,SLOT(onNewConnection()));
    connect(serialPort, SIGNAL(actionFinished()), this, SLOT(onActionFinished()));
    connect(serialPort, SIGNAL(lowBattery()), this, SLOT(onLowBattery()));
    connect(videoControl,SIGNAL(sendInfo(QString)), this, SLOT(onSendInfo(QString)));
    connect(videoControl, SIGNAL(sendFrame(QImage&)), discernColor, SLOT(readFrame(QImage&)));
    connect(discernColor,SIGNAL(sendInfo(QString)), this, SLOT(onSendInfo(QString)));
    connect(discernColor,SIGNAL(directionChanged(int)),this,SLOT(onDirectionChanged(int)));
    connect(discernColor, SIGNAL(startMoveOn(int)), this, SLOT(onStartMoveOn(int)));
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    connect(m_timer_2, SIGNAL(timeout()), this, SLOT(onReadyPlayLowBattery()));
    connect(m_cmdTimer, SIGNAL(timeout()), this, SLOT(onCmdTimerout()));
}

/**
 * @brief     Server类的析构函数
 */

Server::~Server()
{
    if (mplayer != NULL)
    {
        if (mplayer->isOpen())
            mplayer->close();
        delete mplayer;
        mplayer = NULL;
    }
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
    playAudio(StartUp);
    if (!tcpServer->listen(QHostAddress::Any,g_listen_port))
    {
        qDebug()<<tcpServer->errorString();
        return;
    }
    qDebug("Start listen %d\n", g_listen_port);

    isOPenSerial = serialPort->openSerilPort(); //打开串口

    m_timer->start(5000);  //每隔5秒广播一次ip
}

/**
 * @brief     播放提示音,调用的播放器是mplayer
 * @param     type 提示音类型Server::CueTone
 */

void Server::playAudio(Server::CueTone type)
{
    QString musicName("tone/");
    switch (type)
    {
    case StartUp:
        musicName += "start.mp3";
        break;
    case LowBattery:
        musicName += "lowp.mp3";
        break;
    case ConnectSucceeful:
        musicName += "tone-1.mp3";
        break;
    case ConnectFailed:
        musicName += "tone-2.mp3";
        break;
    case ApModeAvailable:
        musicName += "tone-3.mp3";
        break;
    case ResartToWifiMode:
        musicName += "tone-4.mp3";
        break;
    case Connectting:
        musicName += "tone-5.mp3";
        break;
    case RestartToApMode:
        musicName += "tone-6.mp3";
        break;
    default:
        return;
    }

    QString program = "mplayer";
    QStringList arguments;
    arguments << musicName;

    if (mplayer->isOpen())
    {
        mplayer->waitForFinished();
        mplayer->close();
    }

    mplayer->start(program, arguments);
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
        out << qint32(0);
        out.device()->seek(0);
        out << outBlock.size() + msg.size() << msg;
        tempSocket->write(outBlock);
        tempSocket->waitForBytesWritten();
    }
}

/**
 * @brief     接收客户端的tcp消息
 * @details   客户端会发送“95f41ce1”进行身份认证，服务器返回“676f7a75"进行确认
 */

void Server::onSocketRead()
{
    QDataStream in(tcpSocket);
//    in.setVersion(QDataStream::Qt_5_3);
    if (m_bufferReadSize == 0)
    {
        if (tcpSocket->bytesAvailable() <= sizeof(qint32))
            return;

        in >> m_bufferTotalSize;
        m_bufferReadSize += sizeof(qint32);

        QByteArray receiveByteArray;
        in >> receiveByteArray;
        m_bufferReadSize += receiveByteArray.size();
        if (m_bufferReadSize == m_bufferTotalSize)
        {
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
                retMsg.append(QString("Image.Quality=%1\r\n").arg(g_frame_quality));
                retMsg.append(QString("Arrive.Ratio=%1\r\n").arg(g_arrive_ratio));
                retMsg.append(QString("Access.Ratio=%1\r\n").arg(g_access_ratio));
                retMsg.append(QString("Delay.Count=%1\r\n").arg(g_time_count));
                retMsg.append(QString("Quick.Count=%1\r\n").arg(g_far_move_on_time));
                retMsg.append(QString("Slow.Count=%1\r\n").arg(g_near_move_on_time));
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
        m_bufferReadSize = 0;
        m_bufferTotalSize = 0;
    }
}

/**
 * @brief     解析已接收的客户端的指令
 * @param     msg 收到的消息
 */

bool Server::parseData(const QString &msg)
{
    if (msg.isEmpty())
        return false;

    m_result_msg = msg;
    if (msg.startsWith("set Image.Display"))
    {
        QString strVal = msg.mid(msg.indexOf("=")+1);
        if (strVal == "Original" || strVal == "Transform")
        {
            G_Image_Display = strVal;
            return true;
        }
    }
    else if (msg.startsWith("set Select.Rect="))
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
            return true;
        }
    }
    else if (msg.startsWith("Add Target="))
    {
        QStringList msgList = msg.mid(msg.indexOf("=")+1).split(",");
        return discernColor->addTarget(msgList);
    }
    else if(msg.startsWith("Remove Target="))
    {
        int index = msg.mid(msg.indexOf("=")+1).toInt();
        return discernColor->removeTarget(index);
    }
    else if(msg.startsWith("set Robot.Action="))
    {
        QString strVal = msg.mid(msg.indexOf("=")+1);
        if (strVal == "manual")
        {
            discernColor->setActionMode(0);
            return true;
        }
        else if (strVal == "auto")
        {
            discernColor->setActionMode(1);
            return true;
        }
    }
    else if (msg == "RESET")
    {
        discernColor->Reset();
        return true;
    }
    else if (msg.startsWith("Move"))
    {
        QStringList msgList = msg.split(",");
        if (msgList.size() == 2)
        {
            if (msgList[1] == "on") //持续快走
            {
                WriteSerial2("0xd7");
            }
            else if (msgList[1] == "stop") //停止快走
            {
                WriteSerial2("0xda");                
            }
            else
            {
                WriteSerial(msgList[1].toInt());
            }
            return true;
        }
    }
    else if (msg.startsWith("set Wifi.Settings="))
    {
        QStringList msgList = msg.mid(msg.indexOf("=")+1).split(",");
        if (msgList.size() == 2)
        {
            modifyNetworkFile(msgList[0],msgList[1]);
            return true;
        }
    }
    else if (msg.startsWith("set Image.Format="))
    {
        QString image_format = msg.mid(msg.indexOf("=")+1,3);
        if (image_format == "YUV" || image_format == "HSV")
        {
            G_Image_Format = image_format;
            if (image_format == "HSV")
            {
                QString strVal = msg.mid(msg.indexOf("=")+5);
                QStringList valList = strVal.split(",");
                if (valList.length() == 6)
                {
                    videoControl->setHsvInRange("MinH", valList[0].toInt());
                    videoControl->setHsvInRange("MinS", valList[1].toInt());
                    videoControl->setHsvInRange("MinV", valList[2].toInt());
                    videoControl->setHsvInRange("MaxH", valList[3].toInt());
                    videoControl->setHsvInRange("MaxS", valList[4].toInt());
                    videoControl->setHsvInRange("MaxV", valList[5].toInt());
                }
                else
                {
                    return false;
                }
            }
            return true;
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
            return true;
        }
    }
    else if (msg.startsWith("set Color.Brightness="))
    {
        QString brightness = msg.mid(msg.indexOf("=")+1);
        if (!brightness.isEmpty())
        {
            videoControl->setBrightness(brightness.toDouble());
            return true;
        }
    }
    else if (msg.startsWith("set Color.Contrast="))
    {
        QString contrast = msg.mid(msg.indexOf("=")+1);
        if (!contrast.isEmpty())
        {
            videoControl->setContrast(contrast.toInt());
            return true;
        }
    }
    else if (msg.startsWith("set HSV.Channel."))
    {
        QString type = msg.mid(msg.indexOf("=")-4, 4);
        int val = msg.mid(msg.indexOf("=")+1).toInt();
        videoControl->setHsvInRange(type, val);
        return true;
    }
    else if (msg.startsWith("set Target.Type="))
    {
        QStringList msgList = msg.mid(msg.indexOf("=")+1).split(",");
        if (msgList.size() == 2)
        {
            int index = msgList[0].toInt();
            int type = msgList[1].toInt();
            return discernColor->setTargetType(index, type);
        }
    }
    else if (msg.startsWith("set Target.Turn="))
    {
        QStringList msgList = msg.mid(msg.indexOf("=")+1).split(",");
        if (msgList.size() == 2)
        {
            int index = msgList[0].toInt();
            int turn = msgList[1].toInt();
            return discernColor->setTargetTurn(index, turn);
        }
    }
    else if (msg == "get Current.Electricity")
    {
        WriteSerial2("0x82");
        return true;
    }
    else if (msg.startsWith("start set param"))
    {
        QStringList msgList = msg.split("\r\n");
        foreach (const QString &item, msgList) {
            if (item.startsWith("Image.Quality"))
            {
                g_frame_quality = item.mid(item.indexOf("=")+1).toInt();
                if (g_frame_width > 320 && g_frame_quality > 90)
                {
                    g_frame_quality = -1;
                }
            }
            else if (item.startsWith("Center.Ratio"))
            {
                g_horizontal_ratio = item.mid(item.indexOf("=")+1).toDouble();
            }
            else if (item.startsWith("Turn.Ratio"))
            {
                g_rotation_range = item.mid(item.indexOf("=")+1).toDouble();
            }
            else if (item.startsWith("Arrive.Ratio"))
            {
                g_arrive_ratio = item.mid(item.indexOf("=")+1).toDouble();
            }
            else if (item.startsWith("Access.Ratio"))
            {
                g_access_ratio = item.mid(item.indexOf("=")+1).toDouble();
            }
            else if (item.startsWith("Delay.Count"))
            {
                g_time_count = item.mid(item.indexOf("=")+1).toInt();
            }
            else if (item.startsWith("Quick.Count"))
            {
                g_far_move_on_time = item.mid(item.indexOf("=")+1).toInt();
            }
            else if (item.startsWith("Slow.Count"))
            {
                g_near_move_on_time = item.mid(item.indexOf("=")+1).toInt();
            }
            else if (item.startsWith("Camera.Resolution"))
            {
                QStringList msgList = item.mid(item.indexOf("=")+1).split("x");
                if (msgList.size() == 2)
                {
                    videoControl->setCameraResolution(msgList[0].toInt(),msgList[1].toInt());
                    return true;
                }
            }
        }
        return true;
    }
    else if (msg.contains("Start Add HSV.Target"))
    {
        discernColor->startAddHsvTarget();
        return true;
    }
    else if (msg.startsWith("set Robot.Go.Back"))
    {
        G_Go_Back_Flag = msg.mid(msg.indexOf("=")+1).toInt();
        return true;
    }
    else if (msg == "Start Again")
    {
        discernColor->startAgain();
        return true;
    }
    return false;
}

/**
 * @brief     向客户端发送消息
 * @param     msg 需要发送的消息
 */

void Server::WriteMsg(const QByteArray &msg)
{
    m_cmdQueue.append(msg);
    if (!m_cmdTimer->isActive())
        m_cmdTimer->start(100);
}

void Server::onCmdTimerout()
{
    if (!m_cmdQueue.isEmpty())
    {
        QByteArray msg = m_cmdQueue.dequeue();
        QByteArray outBlock;
        QDataStream out(&outBlock,QIODevice::WriteOnly);
        out << qint32(0);
        out.device()->seek(0);
        out << outBlock.size() + msg.size() << msg;
        tcpSocket->write(outBlock);
        tcpSocket->waitForBytesWritten();
    }

    if (m_cmdQueue.isEmpty() && m_cmdTimer->isActive())
    {
        m_cmdTimer->stop();
    }
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
}

/**
 * @brief     连接sendInfo(QString)信号的槽函数,向客户端发送消息,比如标记框位置，一些错误提示等
 */

void Server::onSendInfo(const QString &msg)
{
    WriteMsg(msg.toUtf8());
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

    playAudio(ResartToWifiMode);

    qDebug()<<"--===start reset network===---";
    m_shellName = "network/wifi/wifi_setup.sh";
    QTimer::singleShot(5000, this, SLOT(startExcuteShell()));
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
//    int bufferSize = 9;
//    unsigned char serialnum[] = {0xd3,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x31};
////    unsigned char serialnum[] = {0xd3,0x00,0x00,0x00,0x00,0x05,0x00,0x00,0x31,0x21,0x61,0x63,0x74};
//    bool isOk = false;
//    switch (val)
//    {
//    case 1:  //快走
//    {
//        serialnum[8] = 0x31;
//        isOk = true;
//    }
//        break;
//    case 2:  //快退
//    {
//        serialnum[8] = 0x32;
//        isOk = true;
//    }
//        break;
//    case 3:  //左转（小）
//    {
//        serialnum[8] = 0x33;
//        isOk = true;
//    }
//        break;
//    case 4:  //右转（小）
//    {
//        serialnum[8] = 0x34;
//        isOk = true;
//    }
//        break;
//    case 5:  //左转（大）
//    {
//        serialnum[8] = 0x35;
//        isOk = true;
//    }
//        break;
//    case 6:  //右转（大）
//    {
//        serialnum[8] = 0x36;
//        isOk = true;
//    }
//        break;
//    case 7:
//    {
//        serialnum[8] = 0x37;
//        isOk = true;
//    }
//        break;
//    case 8:
//    {
//        serialnum[8] = 0x38;
//        isOk = true;
//    }
//        break;
//    case 9:
//    {
//        serialnum[8] = 0x39;
//        isOk = true;
//    }
//        break;
//    default:
//        break;
//    }

//    if (isOk)
//    {
//        qDebug("start action: move,%d",val);
//        char* buf = (char*)(&serialnum);
//        serialPort->sendMsg(buf,bufferSize);
//#ifdef Q_OS_WIN
//        QTimer::singleShot(2000, this, SLOT(onActionFinished())); //测试，判断动作timeCount毫秒后完成
//#endif
//    }

    QString strVal = QString::number(val);
    if (strVal.length() > SCM_MAX_BUFFER_SIZE)
    {
        qDebug()<< "error: buffer size out of range, the max buffer size is 65535 !" << __FILE__ << __LINE__;
        return;
    }

    QByteArray hexVal = strVal.toLatin1().toHex();
    QString strHighSize, strLowSize, cmd;
    if (strVal.length() < 256)
    {
        convertIntToHex(strLowSize, strVal.length());
        cmd = QString("d3 00 00 00 00 %1 00 00 %2").arg(strLowSize).arg(QString(hexVal));
    }
    else
    {
        int highSize = strVal.length() / 255;
        int lowSize = strVal.length() % 255;
        convertIntToHex(strHighSize, highSize);
        convertIntToHex(strLowSize, lowSize);
        cmd = QString("d3 00 00 00 %1 %2 00 00 %3").arg(strHighSize).arg(strLowSize).arg(QString(hexVal));
    }

    serialPort->sendMsg(cmd);
#ifdef Q_OS_WIN
    QTimer::singleShot(2000, this, SLOT(onActionFinished())); //测试，判断动作timeCount毫秒后完成
#endif
}

void Server::WriteSerial2(const QString &val)
{
    unsigned char serialnum[] = {0xd7};
    if (val == "0xd7")          //持续快走
    {
        serialnum[0] = 0xd7;
    }
    else if (val == "0xda")     //停止快走
    {
        serialnum[0] = 0xda;
    }
    else if (val == "0x82")    //获取当前剩余电量
    {
        serialnum[0] = 0x82;
    }
    else
    {
        return;
    }
    char* buf = (char*)(&serialnum);
    serialPort->sendMsg(buf,1);
#ifdef Q_OS_WIN
    QTimer::singleShot(2000, this, SLOT(onActionFinished())); //测试，判断动作timeCount毫秒后完成
#endif
}

QString Server::convertIntToHex(QString &outStr,int num)
{
    outStr = QString::number(num,16);
    if (outStr.length() == 1)
        outStr.insert(0,"0");

    return outStr;
}

/**
 * @brief     定时器超时，广播服务器IP地址
 */

void Server::onTimeout()
{
    QString localIp = getLocalIP4Address();

    if (!localIp.isEmpty() && !m_connection_count)
    {
        if (m_local_ip != localIp) //使用md5加密
        {
            m_local_ip = localIp;
            m_byteIpAndNo = QString(m_local_ip + "-" + g_robot_number + "-" + "leju").toUtf8();
            m_byteMd5 = QCryptographicHash::hash(m_byteIpAndNo, QCryptographicHash::Md5);
        }

        QUdpSocket udpSocket;
        QByteArray datagram;
        QDataStream out(&datagram, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_5_3);
        out << QString("Leju@%1-%2").arg(m_local_ip).arg(g_robot_number).toUtf8() << m_byteMd5.toHex();
        udpSocket.writeDatagram(datagram,QHostAddress::Broadcast,g_broadcast_port);
    }

    if (m_bIsConnectRounter)
        return;

    if (!localIp.isEmpty())
    {
        if (localIp == "192.168.8.1")
        {
            m_bIsConnectRounter = true;
            playAudio(ApModeAvailable);
            return;
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

            if (!ba.isEmpty())
            {
                m_bIsConnectRounter = true;
                playAudio(ConnectSucceeful);
                return;
            }
        }
    }

    m_pingCount++;
    if (m_pingCount == 1)
    {
       playAudio(Connectting);
    }
    if (m_pingCount == g_ping_router_count)
    {
        QSettings iniReader("network/wifi.ini",QSettings::IniFormat);
        QString wifi_mode = iniReader.value("Configure/mode").toString();
        if (wifi_mode == "0")
        {
            qDebug()<< "Cannot Ping through router"
                    << "\n"
                    << "---===start reset to Wifi AP mode===---";
            iniReader.setValue("Configure/mode",1);

            playAudio(ConnectFailed);
            m_shellName = "network/wifi_ap/ap_setup.sh";
            QTimer::singleShot(5000, this, SLOT(startExcuteShell()));
        }
    }
}

/**
 * @brief     向串口发送持续快走的指令,同时启动一个定时器，超时停止前进
 */

void Server::onStartMoveOn(int msec)
{
    WriteSerial2("0xd7");
    QTimer::singleShot(msec, this, SLOT(stopMoveOn()));
}

/**
 * @brief     定时器超时，停止前进
 */

void Server::stopMoveOn()
{
    WriteSerial2("0xda");
}

void Server::startExcuteShell()
{
    char *shell = m_shellName.toLatin1().data();
    system(shell);
}

void Server::onLowBattery()
{
    if (m_bIsReady)
    {
        playAudio(LowBattery);
        m_bIsReady = false;
    }
    if (!m_timer_2->isActive())
    {
        m_timer_2->start(5000);
    }
}

void Server::onReadyPlayLowBattery()
{
    m_bIsReady = true;
    m_timer_2->stop();
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
