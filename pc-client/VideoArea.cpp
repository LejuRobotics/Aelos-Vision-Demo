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
//    ui->centralwidget->setStyleSheet("background-color:#797979;");
    test_label = new TestLabel(ui->centralwidget);
    test_label->hide();
    mark_label = new PaintLabel(ui->centralwidget);
    connect(mark_label,SIGNAL(selectFinished(QRect)),this,SLOT(selectFinished(QRect)));

    action_btn_group = new QButtonGroup(this);
    btnList << ui->action_btn_1 << ui->action_btn_2 << ui->action_btn_3
            << ui->action_btn_4 << ui->action_btn_5 << ui->action_btn_6
            << ui->action_btn_7 << ui->action_btn_8 << ui->action_btn_9
            << ui->action_btn_10 << ui->action_btn_MoveOn << ui->action_btn_MoveStop;

    for(int i=0; i<btnList.size(); i++)
    {
        if (btnList[i]->text() == "Move on")
        {
            action_btn_group->addButton(btnList[i],100);
        }
        else if (btnList[i]->text() == "Move stop")
        {
            action_btn_group->addButton(btnList[i],101);
        }
        else
        {
            action_btn_group->addButton(btnList[i],(i+1));
        }
    }
    connect(action_btn_group, SIGNAL(buttonClicked(int)), this, SLOT(onActionButtonGroupClicked(int)));

    ui->groupBox_3->setEnabled(false);

    connect(ui->action_Port, SIGNAL(triggered(bool)), this, SLOT(onActioPortClicked()));
    connect(ui->action_Area, SIGNAL(toggled(bool)), this, SLOT(onActionAreaViewClicked(bool)));
    connect(ui->action_Server_wifi, SIGNAL(triggered(bool)), this, SLOT(onActionServerWifiClicked()));
    connect(ui->actionParam_setting, SIGNAL(triggered(bool)), this, SLOT(onActionParamSettingClicked()));

    camera_with = 320;
    camera_height = 240;
    m_centerAreaRatio = 0.4;
    m_rorationRange = 0.25;
    m_isConnected = false;
    m_bufferReadSize = 0;
    m_bufferTotalSize = 0;

    portSetupDialog = new PortSetupDialog(this);
    portSetupDialog->hide();
    connect(portSetupDialog, SIGNAL(udpPortChanged()), this, SLOT(onUdpPortChanged()));

    scanIpDialog = new ScanIpDiaog(this);
    scanIpDialog->hide();
    connect(scanIpDialog, SIGNAL(sendInfo(QString)), this, SLOT(addInfomation(QString)));
    connect(scanIpDialog, SIGNAL(startConnectTo(QString)), this, SLOT(onStartConnectTo(QString)));

    serverWifiDialog = new ServerWifiSettings(this);
    connect(serverWifiDialog, SIGNAL(wifiChanged(QString,QString)), this, SLOT(onWifiChanged(QString,QString)));

    connectionBox = new ConnectionBox(this);
    connect(connectionBox, SIGNAL(startConnectTo(QString)), this, SLOT(onStartConnectTo(QString)));

    sliderNameList << "Bright" << "Contr." << "YUV_Y" << "MinH"
                      << "MinS" << "MinV" << "MaxH" << "MaxS" << "MaxV";

    for (int i=0; i<sliderNameList.size(); ++i)
    {
        sliderGroupBoxList << new SliderGroupBox(this);
        sliderGroupBoxList[i]->setName(sliderNameList[i]);
        if (i == 0)
        {
            sliderGroupBoxList[i]->setType("float");
            sliderGroupBoxList[i]->setRange(10,30);
        }
        else if (i == 1)
        {
            sliderGroupBoxList[i]->setRange(0,120);
        }
        else if (i == 3 || i == 6)
        {
            sliderGroupBoxList[i]->setRange(0,180);
        }
        else
        {
            sliderGroupBoxList[i]->setRange(0,255);
        }
        connect(sliderGroupBoxList[i], SIGNAL(sliderValueChanged(int)), this, SLOT(onSliderValueChanged(int)));
    }
    ui->verticalLayout->addWidget(sliderGroupBoxList[0]);
    ui->verticalLayout->addWidget(sliderGroupBoxList[1]);
    ui->verticalLayout_2->addWidget(sliderGroupBoxList[2]);

    ui->hsv_slider_layout->addWidget(sliderGroupBoxList[3],0,0,1,1);
    ui->hsv_slider_layout->addWidget(sliderGroupBoxList[6],0,1,1,1);
    ui->hsv_slider_layout->addWidget(sliderGroupBoxList[4],1,0,1,1);
    ui->hsv_slider_layout->addWidget(sliderGroupBoxList[7],1,1,1,1);
    ui->hsv_slider_layout->addWidget(sliderGroupBoxList[5],2,0,1,1);
    ui->hsv_slider_layout->addWidget(sliderGroupBoxList[8],2,1,1,1);

    sliderGroupBoxList[2]->setValue(128);
    sliderGroupBoxList[3]->setValue(0);
    sliderGroupBoxList[4]->setValue(0);
    sliderGroupBoxList[5]->setValue(0);
    sliderGroupBoxList[6]->setValue(255);
    sliderGroupBoxList[7]->setValue(255);
    sliderGroupBoxList[8]->setValue(255);

    sliderGroupBoxList[2]->setEnabled(false);

    parameterSettingDialog = new ParameterSettingDialog(this);
    parameterSettingDialog->hide();
    connect(parameterSettingDialog, SIGNAL(sendData(QByteArray)), this, SLOT(WriteData(QByteArray)));
    connect(parameterSettingDialog, SIGNAL(debugViewChanged(double,double)), test_label, SLOT(drawArea(double,double)));

    connect(ui->tableWidget, SIGNAL(sendData(QByteArray)), this, SLOT(WriteData(QByteArray)));

    readConfigFile();

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    m_frame_count = 0;

    m_sliderTimer = new QTimer(this);
    connect(m_sliderTimer, SIGNAL(timeout()), this, SLOT(onSliderTimeout()));

    QNetworkProxyFactory::setUseSystemConfiguration(false);
    m_long_socket = new QTcpSocket(this);
    m_long_socket->setProxy(QNetworkProxy::NoProxy);
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
    if (m_long_socket->isOpen())
        m_long_socket->abort();
    delete ui;
}

/**
 * @brief     通过已连接的tcp socket发送信息
 */

void VideoArea::WriteData(const QByteArray &msg)
{
    if (msg.isEmpty() || !m_long_socket->isOpen())
        return;

    QByteArray outBlock;
    QDataStream out(&outBlock,QIODevice::WriteOnly);
    out << qint32(0);
    out.device()->seek(0);
    out << outBlock.size() + msg.size() << msg;
    m_long_socket->write(outBlock);
    m_long_socket->waitForBytesWritten();
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
    ui->groupBox_3->setEnabled(true);
    ui->record_btn->setEnabled(true);
    ui->reset_btn->setEnabled(true);
    ui->again_btn->setEnabled(true);
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
    QDataStream in(m_long_socket);
    if (m_bufferReadSize == 0)
    {
        if (m_long_socket->bytesAvailable() <= sizeof(qint32))
            return;

        in >> m_bufferTotalSize;
        m_bufferReadSize += sizeof(qint32);

        QByteArray receiveByteArray;
        in >> receiveByteArray;
        m_bufferReadSize += receiveByteArray.size();
        if (m_bufferReadSize == m_bufferTotalSize)
        {
            QString readData = QString::fromUtf8(receiveByteArray);
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
                foreach (const QString &item, msgList)
                {
                    if (item.startsWith("Camera.Resolution"))
                    {
                        QString strVal = item.mid(item.indexOf("=")+1);
                        QStringList mList = strVal.split("x");
                        if (mList.size() == 2)
                        {
                            camera_with = mList[0].toInt();
                            camera_height = mList[1].toInt();
                            parameterSettingDialog->setValue(ParameterSettingDialog::Resolution, strVal);
                        }
                    }
                    else if (item.startsWith("Image.Quality"))
                    {
                        int val = item.mid(item.indexOf("=")+1).toInt();
                        parameterSettingDialog->setValue(ParameterSettingDialog::ImageQuality, val);
                    }
                    else if (item.startsWith("Area.Position"))
                    {
                        QStringList nList = item.mid(item.indexOf("=")+1).split(",");
                        if (nList.size() == 2)
                        {
                            m_centerAreaRatio = nList[0].toDouble();
                            m_rorationRange = nList[1].toDouble();
                            test_label->drawArea(m_centerAreaRatio, m_rorationRange);
                            parameterSettingDialog->setValue(ParameterSettingDialog::CenterRatio, m_centerAreaRatio);
                            parameterSettingDialog->setValue(ParameterSettingDialog::TurnRatio, m_rorationRange);
                        }
                    }
                    else if (item.startsWith("Arrive.Ratio"))
                    {
                        double val = item.mid(item.indexOf("=")+1).toDouble();
                        parameterSettingDialog->setValue(ParameterSettingDialog::ArriveRatio, val);
                    }
                    else if (item.startsWith("Access.Ratio"))
                    {
                        double val = item.mid(item.indexOf("=")+1).toDouble();
                        parameterSettingDialog->setValue(ParameterSettingDialog::AccessRatio, val);
                    }
                    else if (item.startsWith("Delay.Count"))
                    {
                        int val = item.mid(item.indexOf("=")+1).toInt();
                        parameterSettingDialog->setValue(ParameterSettingDialog::DelayCount, val);
                    }
                    else if (item.startsWith("Quick.Count"))
                    {
                        int val = item.mid(item.indexOf("=")+1).toInt();
                        parameterSettingDialog->setValue(ParameterSettingDialog::QuickCount, val);
                    }
                    else if (item.startsWith("Slow.Count"))
                    {
                        int val = item.mid(item.indexOf("=")+1).toInt();
                        parameterSettingDialog->setValue(ParameterSettingDialog::SlowCount, val);
                    }
                    else if (item.startsWith("Obstacle.Turn.Count"))
                    {
                        int val = item.mid(item.indexOf("=")+1).toInt();
                        parameterSettingDialog->setValue(ParameterSettingDialog::ObstacleTurnCount, val);
                    }
                    else if (item.startsWith("Goback.Turn.Count"))
                    {
                        int val = item.mid(item.indexOf("=")+1).toInt();
                        parameterSettingDialog->setValue(ParameterSettingDialog::GobackTurnCount, val);
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
                   else if (item.startsWith("Unrecognized color"))
                   {
                       mark_label->cleanRect(); //清除标记框
                   }
                   else if (item.startsWith("Mark.RGB="))
                   {
                       m_mark_rgb = item.mid(item.indexOf("=")+1);
                       mark_label->setMarkColor(m_mark_rgb);
                       if (!ui->record_btn->isEnabled())
                       {
                           ui->record_btn->setEnabled(true);
                       }
                   }
                }
            }
            else if (readData.startsWith("@Begin HSV:"))
            {
                QStringList list = readData.split("\r\n");
                foreach (QString item, list) {
                   if (item.startsWith("Mark.Rect="))
                   {

                   }
                   else if (item.startsWith("Unrecognized color"))
                   {

                   }
                   else if (item.startsWith("Max.Width"))
                   {
                       int maxWidth = item.mid(item.indexOf("=")+1).toInt();
                       QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
                       ui->tableWidget->addItem(time,"0,255,0",maxWidth);

                       QString cmd = QString("Add Target=%1,%2,%3,%4,%5,%6,%7,%8,%9,%10")
                               .arg(ui->tableWidget->lastName())
                               .arg(maxWidth)
                               .arg(ui->tableWidget->lastType())
                               .arg(ui->tableWidget->lastTurn())
                               .arg(sliderGroupBoxList[3]->value())
                               .arg(sliderGroupBoxList[4]->value())
                               .arg(sliderGroupBoxList[5]->value())
                               .arg(sliderGroupBoxList[6]->value())
                               .arg(sliderGroupBoxList[7]->value())
                               .arg(sliderGroupBoxList[8]->value());
                       WriteData(cmd.toUtf8());
                   }
                }
            }
            else if (readData.startsWith("Return Current.HSV.Range"))
            {
                QString strVal = readData.mid(readData.indexOf("=")+1);
                QStringList valList = strVal.split(",");
                if (valList.length() == 6)
                {
                    sliderGroupBoxList[3]->setValue(valList[0].toInt(), true);
                    sliderGroupBoxList[4]->setValue(valList[1].toInt(), true);
                    sliderGroupBoxList[5]->setValue(valList[2].toInt(), true);
                    sliderGroupBoxList[6]->setValue(valList[3].toInt(), true);
                    sliderGroupBoxList[7]->setValue(valList[4].toInt(), true);
                    sliderGroupBoxList[8]->setValue(valList[5].toInt(), true);
                }
            }
            else if (readData.startsWith("Reach.Target"))
            {
                QString strVal = readData.mid(readData.indexOf("=")+1);
                for (int i=0; i<ui->tableWidget->rowCount(); ++i)
                {
                    QString name = ui->tableWidget->item(i, 0)->text();
                    if (name == strVal)
                    {
                        ui->tableWidget->item(i, 6)->setText(tr("已完成"));
                        break;
                    }
                }

//                if (ui->tableWidget->isAllFinished())
//                {
//                    ui->manual_radio->setChecked(true);
//                    on_manual_radio_clicked(true);
//                }
            }
        }
        m_bufferReadSize = 0;
        m_bufferTotalSize = 0;
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

    m_timer->stop();
    ui->groupBox_3->setEnabled(false);
    ui->again_btn->setEnabled(false);
    ui->manual_radio->setChecked(true);
    for (int i=0; i<btnList.size(); ++i)
    {
        btnList[i]->setEnabled(false);
    }
    ui->record_btn->setEnabled(false);
    ui->reset_btn->setEnabled(false);
    ui->auto_radio->setEnabled(false);
    ui->manual_radio->setEnabled(false);
    for (int i=ui->tableWidget->rowCount()-1; i>-1; --i)
    {
        ui->tableWidget->removeRow(i);
    }
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
            if (!m_timer->isActive() && m_isConnected)
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

void VideoArea::onActionParamSettingClicked()
{
    parameterSettingDialog->exec();
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
    if (ui->tableWidget->rowCount() > 5)
    {
        QMessageBox::information(0, tr("提示"), tr("目前最多只能添加6个"),QMessageBox::Ok);
        return;
    }
    QString cmd;
    if (!ui->hsv_radio->isChecked()) //RGB,YUV格式
    {
        ui->record_btn->setEnabled(false);
        QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        ui->tableWidget->addItem(time,m_mark_rgb,m_original_rect.width());

        cmd = QString("Add Target=%1,%2,%3,%4")
                .arg(ui->tableWidget->lastName())
                .arg(m_original_rect.width())
                .arg(ui->tableWidget->lastType())
                .arg(ui->tableWidget->lastTurn());
    }
    else  // HSV格式
    {
        cmd = QString("Start Add HSV.Target");
    }

    WriteData(cmd.toUtf8());
}

/**
 * @brief    点击Reset按钮执行的槽函数,重置服务端
 */

void VideoArea::on_reset_btn_clicked()
{    
    QString cmd("RESET");
    WriteData(cmd.toUtf8());

    ui->record_btn->setEnabled(true);
    ui->again_btn->setEnabled(true);
    ui->manual_radio->setChecked(true);
    for (int i=0; i<btnList.size(); ++i)
    {
        btnList[i]->setEnabled(true);
    }
    for (int i=ui->tableWidget->rowCount()-1; i>-1; --i)
    {
        ui->tableWidget->removeRow(i);
    }
}

void VideoArea::on_again_btn_clicked()
{
    ui->tableWidget->resetState();
    QString cmd("Start Again");
    WriteData(cmd.toUtf8());
}

/**
 * @brief    点击auto执行的槽函数,如果已经选中好目标，则开始自动接近目标
 * @param    checked  选中状态
 */

void VideoArea::on_auto_radio_clicked(bool checked)
{
    if (checked)
    {
        for (int i=0; i<btnList.size(); ++i)
        {
            btnList[i]->setEnabled(false);
        }
        QString cmd("set Robot.Action=auto");
        WriteData(cmd.toUtf8());
    }
}

/**
 * @brief    点击manual执行的槽函数,手动控制机器人动作，如果机器人正在自动行走，将会暂停动作
 * @param    checked  选中状态
 */

void VideoArea::on_manual_radio_clicked(bool checked)
{
    if (checked)
    {
        for (int i=0; i<btnList.size(); ++i)
        {
            btnList[i]->setEnabled(true);
        }
        QString cmd("set Robot.Action=manual");
        WriteData(cmd.toUtf8());
    }
}


/**
 * @brief    点击Quick walk,Quick back等按钮控制机器人动作
 */

void VideoArea::onActionButtonGroupClicked(int btnID)
{
    QString cmd;
    if (btnID < 100)
    {
        cmd = QString("Move,%1").arg(btnID);
    }
    else if (btnID == 100)
    {
        cmd = "Move,on";
    }
    else if (btnID == 101)
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
 * @brief    显示原图
 */

void VideoArea::on_original_radio_toggled(bool checked)
{
    if (checked)
    {
        QString msg("set Image.Display=Original");
        WriteData(msg.toUtf8());
    }
}

/**
 * @brief    显示转换后的图片
 */

void VideoArea::on_transform_radio_toggled(bool checked)
{
    if (checked)
    {
        QString msg("set Image.Display=Transform");
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
        ui->record_btn->setEnabled(false);
        sliderGroupBoxList[2]->setEnabled(true);
        ui->hsvGroupBox->setEnabled(false);

        QString msg("set Image.Format=YUV");
        WriteData(msg.toUtf8());
    }
}

/**
 * @brief    当选择HSV按钮的时候，设置图片格式为HSV
 */

void VideoArea::on_hsv_radio_toggled(bool checked)
{
    if (checked)
    {
        ui->record_btn->setEnabled(true);
        sliderGroupBoxList[2]->setEnabled(false);
        ui->hsvGroupBox->setEnabled(true);
        mark_label->cleanRect();

        QString msg = QString("set Image.Format=HSV,%1,%2,%3,%4,%5,%6")
                .arg(sliderGroupBoxList[3]->value())
                .arg(sliderGroupBoxList[4]->value())
                .arg(sliderGroupBoxList[5]->value())
                .arg(sliderGroupBoxList[6]->value())
                .arg(sliderGroupBoxList[7]->value())
                .arg(sliderGroupBoxList[8]->value());
        WriteData(msg.toUtf8());
    }
}

void VideoArea::onSliderValueChanged(int value)
{
    SliderGroupBox *obj = qobject_cast<SliderGroupBox*>(sender());
    QString name = obj->getName();
    if (name == "Bright")
    {
        double brightness = value/10.0;
        m_command = QString("set Color.Bright=%1").arg(brightness);
    }
    else if (name == "Contr.")
    {
        m_command = QString("set Color.Contr=%1").arg(value);
    }
    else if (name == "YUV_Y")
    {
        m_command = QString("set Color.Channel.Y=%1").arg(value);
    }
    else if (name == "MinH")
    {
        m_command = QString("set HSV.Channel.MinH=%1").arg(value);
    }
    else if (name == "MinS")
    {
        m_command = QString("set HSV.Channel.MinS=%1").arg(value);
    }
    else if (name == "MinV")
    {
        m_command = QString("set HSV.Channel.MinV=%1").arg(value);
    }
    else if (name == "MaxH")
    {
        m_command = QString("set HSV.Channel.MaxH=%1").arg(value);
    }
    else if (name == "MaxS")
    {
        m_command = QString("set HSV.Channel.MaxS=%1").arg(value);
    }
    else if (name == "MaxV")
    {
        m_command = QString("set HSV.Channel.MaxV=%1").arg(value);
    }

    if (!m_sliderTimer->isActive())
        m_sliderTimer->start(300);
}

void VideoArea::onSliderTimeout()
{
    WriteData(m_command.toUtf8());
    m_sliderTimer->stop();
}

void VideoArea::addInfomation(const QString &msg)
{
    ui->textEdit->append(msg);
}

void VideoArea::on_goback_checkBox_clicked(bool checked)
{
    QString msg;
    if (checked)
    {
        msg = "set Robot.Go.Back=1";
    }
    else
    {
        msg = "set Robot.Go.Back=0";
    }
    WriteData(msg.toUtf8());
}
