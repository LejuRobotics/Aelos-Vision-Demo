/**
 * @file       VideoArea.cpp
 * @version    1.0
 * @date       2017年07月08日
 * @author     C_Are
 * @copyright  Leju
 *
 * @brief      主窗口，VideoArea类的cpp文件
 * @details    VideoArea类是主窗口类，可以接收到的摄像头图像，以及与服务端进行tcp通信
 */

#include "VideoArea.h"
#include "ui_VideoArea.h"

/**
 * @brief     VideoArea类的构造函数
 * @param     parent 父对象
 * @details   界面初始化，创建udp服务器，以及tcp客户端
 */

VideoArea::VideoArea(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::VideoArea)
{
    ui->setupUi(this);

    test_label = new TestLabel(ui->centralwidget);
    test_label->hide();
    mark_label = new PaintLabel(ui->centralwidget);
    connect(mark_label,SIGNAL(selectFinished(QRect)),this,SLOT(selectFinished(QRect)));

    action_btn_group = new QButtonGroup(this);
    btnList << ui->action_btn_1 << ui->action_btn_2 << ui->action_btn_3
            << ui->action_btn_4 << ui->action_btn_5 << ui->action_btn_6
            << ui->action_btn_7 << ui->action_btn_8;

    for(int i=0; i<btnList.size(); i++)
    {
        btnList[i]->setEnabled(false);
        action_btn_group->addButton(btnList[i],(i+1));
    }
    connect(action_btn_group, SIGNAL(buttonClicked(int)), this, SLOT(onActionButtonGroupClicked(int)));

    ui->record_btn->setEnabled(false);
    ui->reset_btn->setEnabled(false);
    ui->auto_radio->setEnabled(false);
    ui->manual_radio->setEnabled(false);

    connect(ui->action_Port, SIGNAL(triggered(bool)), this, SLOT(onActioPortClicked()));
    connect(ui->action_Area, SIGNAL(toggled(bool)), this, SLOT(onActionAreaViewClicked(bool)));
    connect(ui->action_Server_wifi, SIGNAL(triggered(bool)), this, SLOT(onActionServerWifiClicked()));

    radio_btn_group = new QButtonGroup(this);
    radio_btn_group->addButton(ui->auto_radio,0);
    radio_btn_group->addButton(ui->manual_radio,1);
    connect(radio_btn_group, SIGNAL(buttonToggled(int,bool)), this, SLOT(onRadioGroupClicked(int,bool)));

    camera_with = 320;
    camera_height = 240;
    m_centerAreaRatio = 0.4;
    m_rorationRange = 0.25;
    m_isConnected = false;

    portSetupDialog = new PortSetupDialog(this);
    portSetupDialog->hide();
    connect(portSetupDialog, SIGNAL(udpPortChanged()), this, SLOT(onUdpPortChanged()));

    scanIpDialog = new ScanIpDiaog(this);
    scanIpDialog->hide();
    connect(scanIpDialog, SIGNAL(startConnectTo(QString)), this, SLOT(onStartConnectTo(QString)));

    serverWifiDialog = new ServerWifiSettings(this);
    connect(serverWifiDialog, SIGNAL(wifiChanged(QString,QString)), this, SLOT(onWifiChanged(QString,QString)));

    connectionBox = new ConnectionBox(this);
    connect(connectionBox, SIGNAL(startConnectTo(QString)), this, SLOT(onStartConnectTo(QString)));

    readConfigFile();

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    m_frame_count = 0;

    m_sliderTimer = new QTimer(this);
    connect(m_sliderTimer, SIGNAL(timeout()), this, SLOT(onSliderTimeout()));

    QNetworkProxyFactory::setUseSystemConfiguration(false);
    m_long_socket = new QTcpSocket(this);
    connect(m_long_socket, SIGNAL(readyRead()),this,SLOT(onLongSocketReadyRead()));
    connect(m_long_socket,SIGNAL(disconnected()), this, SLOT(onSocketDisconnect()));

    udpSocket = new QUdpSocket(this);
    if (!udpSocket->bind(portSetupDialog->UdpPort()))
    {
       ui->textEdit->append(udpSocket->errorString());
       return;
    }
    ui->textEdit->append(QString("Bind %2 successful !").arg(portSetupDialog->UdpPort()));
    connect(udpSocket,SIGNAL(readyRead()), this, SLOT(onUdpSocketReadyRead()));
}

/**
 * @brief     VideoArea类的析构函数
 * @details   销毁动态创建的ui指针
 */

VideoArea::~VideoArea()
{
    delete ui;
}

/**
 * @brief     通过已连接的tcp socket发送信息
 */

void VideoArea::WriteData(const QByteArray &msg)
{
    QByteArray outBlock;
    QDataStream out(&outBlock,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_3);
    out<<msg;
    m_long_socket->write(outBlock);
}

/**
 * @brief    点击主窗口上的Connect/Disconect按钮执行的槽函数
 * @details  点击Connect，调取scanIpDialog对话框
 */

void VideoArea::on_connect_btn_clicked()
{
    if (ui->connect_btn->text() == tr("Connect"))
    {
       scanIpDialog->exec();
    }
    else
    {
        m_long_socket->abort();
    }
}

/**
 * @brief    点击scanIpDialog窗口Connect按钮执行的槽函数
 * @details  连接成功后，服务器会通过udp发送图像，断开连接，服务器（机器人）只是不发送图像，动作继续执行
 */

void VideoArea::onStartConnectTo(const QString &ip)
{
    m_server_ip = ip;
    QString msg;
    m_long_socket->abort();
    m_long_socket->connectToHost(ip, portSetupDialog->TcpPort());
    if (!m_long_socket->waitForConnected(3000))
    {
        msg = QString("connect to %1 failed !").arg(ip);
        ui->textEdit->append(msg);
        return;
    }

    ui->connect_btn->setText(tr("Disconnect"));
    ui->record_btn->setEnabled(true);
    ui->reset_btn->setEnabled(true);
    ui->auto_radio->setEnabled(true);
    ui->manual_radio->setEnabled(true);
    for (int i=0; i<btnList.size(); ++i)
    {
        btnList[i]->setEnabled(true);
    }

    mark_label->cleanRect(); //清除标记框
    saveConfigFile();
}

/**
 * @brief    接收处理tcp消息
 */

void VideoArea::onLongSocketReadyRead()
{
    QByteArray message;
    QDataStream in(m_long_socket);
    in.setVersion(QDataStream::Qt_5_3);
    in >> message;
    QString readData = QString::fromUtf8(message);
    ui->textEdit->append(readData);

    if (readData.startsWith("Connect to server successful"))
    {
        WriteData(QString("95f41ce1").toUtf8());
    }
    else if (readData == "676f7a75")
    {
        WriteData(QString("Start.Running\r\n").toUtf8());
        m_isConnected = true;
    }
    else if (readData.contains("is connecting to the server"))
    {
        m_long_socket->abort();
    }
    else if (readData.startsWith("From server:"))
    {
        QStringList msgList = readData.split("\r\n");
        if (msgList.size() > 2)
        {
            if (msgList[1].startsWith("Camera.Resolution"))
            {
              QStringList mList = msgList[1].mid(msgList[1].indexOf("=")+1).split("x");
              if (mList.size() == 2)
              {
                  camera_with = mList[0].toInt();
                  camera_height = mList[1].toInt();
              }
            }
            if (msgList[2].startsWith("Area.Position"))
            {
               QStringList nList = msgList[2].mid(msgList[2].indexOf("=")+1).split(",");
               if (nList.size() == 2)
               {
                   m_centerAreaRatio = nList[0].toDouble();
                   m_rorationRange = nList[1].toDouble();
                   test_label->drawArea(m_centerAreaRatio, m_rorationRange);
               }
            }
        }
    }

    else if (readData.startsWith("@Begin:"))
    {
        QStringList list = readData.split("\r\n");
        foreach (QString item, list) {
           if (item.startsWith("Mark.Rect="))
           {
               QStringList msgList = item.mid(item.indexOf("=")+1).split(",");
               if (msgList.size() == 4)
               {
                   m_original_rect.setX(msgList[0].toInt());
                   m_original_rect.setY(msgList[1].toInt());
                   m_original_rect.setWidth(msgList[2].toInt());
                   m_original_rect.setHeight(msgList[3].toInt());

                   int realX = qRound((double)m_original_rect.x() / camera_with * ui->video_label->width());
                   int realY = qRound((double)m_original_rect.y() / camera_height * ui->video_label->height());
                   int realW = qRound((double)m_original_rect.width() / camera_with * ui->video_label->width());
                   int realH = qRound((double)m_original_rect.height() / camera_height * ui->video_label->height());

                   QRect realRect = QRect(realX, realY, realW, realH);

                   mark_label->drawRect(realRect); //标记画框
               }
           }
           else if (item.startsWith("Reach.Target="))
           {
               if (!ui->manual_radio->isChecked())
               {
                   ui->manual_radio->setChecked(true);
               }
           }
           else if (item.startsWith("Unrecognized color"))
           {
               mark_label->cleanRect(); //清除标记框
           }
           else if (item.startsWith("Mark.RGB="))
           {
               m_mark_rgb = item.mid(item.indexOf("=")+1);
               mark_label->setMarkColor(m_mark_rgb);
           }
        }
    }
}

/**
 * @brief    当tcp socket断开后执行的槽函数
 */

void VideoArea::onSocketDisconnect()
{
    ui->connect_btn->setText(tr("Connect"));
    ui->textEdit->append(QString("disconect to %1!").arg(m_server_ip));
    m_isConnected = false;

    if (m_timer->isActive())
    {
        m_timer->stop();
    }
    if (!ui->manual_radio->isChecked())
    {
        ui->manual_radio->setChecked(true);
    }
    for (int i=0; i<btnList.size(); ++i)
    {
        btnList[i]->setEnabled(false);
    }
    ui->record_btn->setEnabled(false);
    ui->reset_btn->setEnabled(false);
    ui->auto_radio->setEnabled(false);
    ui->manual_radio->setEnabled(false);
}

/**
 * @brief    接收处理udp消息，显示摄像头图像
 */

void VideoArea:: onUdpSocketReadyRead()
{  
    while (udpSocket->hasPendingDatagrams())
    {
        QByteArray m_udpReadData;
        m_udpReadData.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(m_udpReadData.data(),m_udpReadData.size());

        QByteArray dataType, dataContent;
        QDataStream m_udpReadStream(&m_udpReadData, QIODevice::ReadOnly);
        m_udpReadStream.setVersion(QDataStream::Qt_5_3);
        m_udpReadStream >> dataType >> dataContent;
        QString dataTypeStr = QString::fromUtf8(dataType);
        if (dataTypeStr == "IMAGE")
        {
            m_load_image.loadFromData(dataContent);
            if (m_load_image.size() != ui->video_label->size())
            {
                ui->video_label->setPixmap(QPixmap::fromImage(m_load_image).scaled(ui->video_label->size(),Qt::KeepAspectRatio));
            }
            else
            {
                ui->video_label->setPixmap(QPixmap::fromImage(m_load_image));
            }
            m_frame_count++;
            if (!m_timer->isActive())
            {
                m_timer->start(1000);
            }
        }
        else if (dataTypeStr.startsWith("Leju@"))
        {
            QByteArray infoMd5 = dataType.mid(dataType.indexOf("@")+1).append("-leju");
            QByteArray byteMd5 = QCryptographicHash::hash(infoMd5, QCryptographicHash::Md5);
            if (byteMd5.toHex() == dataContent) //校验md5
            {
                QStringList infoList = dataTypeStr.mid(dataTypeStr.indexOf("@")+1).split("-");
                if (infoList.length() == 2)
                {
                    if (!m_isConnected) //如果已经连接上了就不弹出
                    {
                       connectionBox->addConnection(infoList[0], infoList[1]);
                    }
                }
            }
        }
    }
}

/**
 * @brief    定时器超时执行的槽函数
 * @details  显示每秒接收到的图片帧数
 */

void VideoArea::onTimeout()
{
    QString displayMsg = QString("FPS: %1      time: %2").arg(m_frame_count)
            .arg(QTime::currentTime().toString("hh:mm:ss.zzz"));
    this->statusBar()->showMessage(displayMsg);
    m_frame_count = 0;
}

/**
 * @brief    标记物体后执行的槽函数
 * @warning  opencv的Rect取值只能是正数，如果传入的_rect是宽高小于0，需要转为正数
 */

void VideoArea::selectFinished(const QRect &_rect)
{
    QRect curRect;
    if (_rect.width() < 0 || _rect.height() < 0)
    {
        curRect = QRect(_rect.bottomRight(), _rect.topLeft());
    }
    else
    {
        curRect = _rect;
    }

    int realX = qRound((double)curRect.x() / ui->video_label->width() * camera_with);
    int realY = qRound((double)curRect.y() / ui->video_label->height() * camera_height);
    int realW = qRound((double)curRect.width() / ui->video_label->width() * camera_with);
    int realH = qRound((double)curRect.height() / ui->video_label->height() * camera_height);
    curRect = QRect(realX, realY, realW, realH);

    if (m_long_socket->isOpen() && realW > 0 && realH > 0)
    {
        QString msg = QString("set Select.Rect=%1,%2,%3,%4").
                arg(curRect.x()).arg(curRect.y()).
                arg(curRect.width()).arg(curRect.height());
        WriteData(msg.toUtf8());
        ui->textEdit->append(msg);
    }
}

/**
 * @brief    点击菜单栏Port action执行的槽函数，显示portSetupDialog窗口
 */

void VideoArea::onActioPortClicked()
{
    if (portSetupDialog->isHidden())
    {
        portSetupDialog->show();
    }
}

/**
 * @brief    点击菜单栏Debug view action执行的槽函数，显示或隐藏test_label
 * @param    flag true显示，false隐藏
 */

void VideoArea::onActionAreaViewClicked(bool flag)
{
    test_label->setVisible(flag);
}

/**
 * @brief    点击菜单栏Wifi settings action执行的槽函数，显示serverWifiDialog
 */

void VideoArea::onActionServerWifiClicked()
{
    serverWifiDialog->exec();
}

/**
 * @brief    读取配置文件
 * @details  初始化设置之前一次登录的的参数
 */

void VideoArea::readConfigFile()
{
    QSettings iniReader("config.ini",QSettings::IniFormat);

    QString tcpPort = iniReader.value("Port/tcpPort").toString();
    portSetupDialog->setValue(PortSetupDialog::TCP_Port, tcpPort);
    scanIpDialog->setTcpPort(tcpPort.toInt());

    QString udpPort = iniReader.value("Port/udpPort").toString();
    portSetupDialog->setValue(PortSetupDialog::UDP_Port, udpPort);

    iniReader.beginGroup("Address");
    QStringList addr_key_list, addr_value_list;
    addr_key_list = iniReader.childKeys();
    for(int i=0;i<addr_key_list.length();++i)
    {
        addr_value_list << iniReader.value(addr_key_list[i]).toString();
    }
    iniReader.endGroup();
    scanIpDialog->loadHistotyAddress(addr_value_list);
}

/**
 * @brief    保存配置文件
 */

void VideoArea::saveConfigFile()
{
    QSettings iniReader("config.ini",QSettings::IniFormat);
    iniReader.beginGroup("Address");
    QStringList addr_key_list, addr_value_list;
    addr_key_list = iniReader.childKeys();
    for(int i=0;i<addr_key_list.size();++i)
    {
        addr_value_list << iniReader.value(addr_key_list[i]).toString();
    }
    if (!addr_value_list.contains(m_server_ip))
    {
        if (addr_value_list.size() == 8)
        {
           addr_value_list.removeLast();
        }
        addr_value_list.insert(0,m_server_ip);
    }
    else
    {
        for (int i=0; i<addr_value_list.size(); ++i)
        {
            if (addr_value_list[i] == m_server_ip)
            {
                addr_value_list.move(i,0);
                break;
            }
        }
    }

    for (int i=0; i<addr_value_list.size(); ++i)
    {
        QString nextKey = QString("addr_%1").arg(i+1);
        iniReader.setValue(nextKey, addr_value_list[i]);
    }
}

/**
 * @brief    点击Record按钮执行的槽函数
 * @details  记录机器人停止位置的标记框的大小
 */

void VideoArea::on_record_btn_clicked()
{
    ui->record_btn->setEnabled(false);
    QString cmd("set Stop.Enable=1");
    WriteData(cmd.toUtf8());
    int curSize = m_original_rect.width()*m_original_rect.height();
    ui->textEdit->append(QString("Record size: %1\r\n").arg(curSize));

    ui->tableWidget->addItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"),m_mark_rgb,curSize);
}

/**
 * @brief    点击Reset按钮执行的槽函数
 * @details  当点击过Record按钮后，需要执行下一次测试，需要点击Reset按钮进行重置
 */

void VideoArea::on_reset_btn_clicked()
{
    ui->record_btn->setEnabled(true);
    QString cmd("set Stop.Enable=0");
    WriteData(cmd.toUtf8());
}

/**
 * @brief    点击manual或auto执行的槽函数
 * @details  nanual状态下，可以通过Quick walk等按钮直接手动控制机器人动作，
 *           如果已经标记过物体，点击aoto，机器人会根据识别到的标记框位置自动前进
 */

void VideoArea::onRadioGroupClicked(int btnID, bool checked)
{
    if (checked)
    {
        QString cmd;
        switch (btnID)
        {
        case 0: //自动
            for (int i=0; i<btnList.size(); ++i)
            {
                btnList[i]->setEnabled(false);
            }
            cmd = "set Robot.Action=auto";
            break;
        case 1: //手动
            for (int i=0; i<btnList.size(); ++i)
            {
                btnList[i]->setEnabled(true);
            }
            cmd = "set Robot.Action=manual";
            break;
        default:
            break;
        }
        if (!cmd.isEmpty())
           WriteData(cmd.toUtf8());
    }
}

/**
 * @brief    点击Quick walk,Quick back等按钮控制机器人动作
 */

void VideoArea::onActionButtonGroupClicked(int btnID)
{
    QString cmd;
    if (btnID < 7)
    {
       cmd = QString("Move,%1").arg(btnID);
    }
    else if (btnID == 7)
    {
        cmd = "Move,on";
    }
    else if (btnID == 8)
    {
        cmd = "Move,stop";
    }
    WriteData(cmd.toUtf8());
}

/**
 * @brief    当更改udp端口后,需要重新绑定更改后的端口
 */
void VideoArea::onUdpPortChanged()
{
    udpSocket->abort();
    if (!udpSocket->bind(portSetupDialog->UdpPort()))
    {
       ui->textEdit->append(udpSocket->errorString());
       return;
    }
    ui->textEdit->append(QString("Bind %2 successful !").arg(portSetupDialog->UdpPort()));
}

/**
 * @brief    向服务端发送设置的wifi账号密码
 */

void VideoArea::onWifiChanged(const QString &userName, const QString &password)
{
    QString msg = QString("set Wifi.Settings=%1,%2").arg(userName).arg(password);
    WriteData(msg.toUtf8());
}

/**
 * @brief    重写resizeEvent事件
 * @details  设置test_label，mark_label和显示摄像头的video_label大小一样
 */

void VideoArea::resizeEvent(QResizeEvent *e)
{
    test_label->setGeometry(ui->video_label->geometry());
    mark_label->setGeometry(ui->video_label->geometry());
    QMainWindow::resizeEvent(e);
}

/**
 * @brief    当滑块的值变化的时候，通过定时器发送指令
 */

void VideoArea::startSliderTimer()
{
    if (!m_sliderTimer->isActive())
    {
        m_sliderTimer->start(200);
    }
}

/**
 * @brief    定时器超时，发送指令
 */

void VideoArea::onSliderTimeout()
{
    WriteData(m_command.toUtf8());
    m_sliderTimer->stop();
}

/**
 * @brief    当选择RGB按钮的时候，设置图片格式为RGB
 */

void VideoArea::on_rgb_radio_toggled(bool checked)
{
    if (checked)
    {
       QString msg("set Image.Format=RGB");
       WriteData(msg.toUtf8());
    }
}

/**
 * @brief    当选择YUV按钮的时候，设置图片格式为YUV
 */

void VideoArea::on_yuv_radio_toggled(bool checked)
{
    if (checked)
    {
       QString msg("set Image.Format=YUV");
       WriteData(msg.toUtf8());
    }
}

/**
 * @brief    拖动Y滑块，设置Y值，YUV格式中的Y，也就是亮度，这个是把
 *           图片的每个像素的Y通道(亮度)都改为一个统一的值
 * @note     只更改YUV格式中的Y通道，并且强制统一为一个值
 * @param    value 亮度
 */

void VideoArea::on_colorY_slider_valueChanged(int value)
{
    ui->colorY_label->setText(QString("Y: %1").arg(value));
    m_command = QString("set Color.Channel.Y=%1").arg(value);
    startSliderTimer();
}

/**
 * @brief    拖动Brightness滑块，设置亮度，这个和Y滑块不一样，这个是
 *           整体改变亮度，比如RGB格式下，把R,G,B 3个通道都改变
 * @param    value 亮度
 */

void VideoArea::on_brightness_slider_valueChanged(int value)
{
    double brightness = value/100.0;
    ui->brightness_label->setText(QString("Brightness: %1").arg(brightness));
    m_command = QString("set Color.Brightness=%1").arg(brightness);
    startSliderTimer();
}

/**
 * @brief    拖动Contrast滑块，设置对比度，配合Brightness使用
 * @param    value 对比度
 */

void VideoArea::on_contrast_slider_valueChanged(int value)
{
    ui->contrast_label->setText(QString("Contrast: %1").arg(value));
    m_command = QString("set Color.Contrast=%1").arg(value);
    startSliderTimer();
}
