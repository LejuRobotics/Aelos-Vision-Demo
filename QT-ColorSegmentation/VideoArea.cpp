#include "VideoArea.h"
#include "ui_VideoArea.h"

VideoArea::VideoArea(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::VideoArea)
{
    ui->setupUi(this);

    test_label = new TestLabel(ui->centralwidget);
    test_label->hide();
    mark_label = new PaintLabel(ui->centralwidget);
    connect(mark_label,SIGNAL(selectFinished(QRect)),this,SLOT(selectFinished(QRect)));

    ui->horizontalLayout->setSizeConstraint(QLayout::SetFixedSize);

    action_btn_group = new QButtonGroup(this);
    btnList << ui->action_btn_1 << ui->action_btn_2 << ui->action_btn_3
            << ui->action_btn_4 << ui->action_btn_5 << ui->action_btn_6;

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

    portSetupDialog = new PortSetupDialog(this);
    portSetupDialog->hide();

    scanIpDialog = new ScanIpDiaog(this);
    scanIpDialog->hide();
    connect(scanIpDialog, SIGNAL(startConnected(QString)), this, SLOT(onStartConnected(QString)));

    serverWifiDialog = new ServerWifiSettings(this);
    connect(serverWifiDialog, SIGNAL(wifiChanged(QString,QString)), this, SLOT(onWifiChanged(QString,QString)));

    QNetworkProxyFactory::setUseSystemConfiguration(false);
    m_long_socket = new QTcpSocket(this);
    in.setDevice(m_long_socket);
    in.setVersion(QDataStream::Qt_5_3);

    udpSocket = new QUdpSocket(this);

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    m_frame_count = 0;

    readConfigFile();
}

VideoArea::~VideoArea()
{
    delete ui;
}

void VideoArea::WriteData(const QByteArray &msg)
{
    m_long_socket->write(msg);
}

void VideoArea::on_connect_btn_clicked()
{
    if (ui->connect_btn->text() == tr("Connect"))
    {
       scanIpDialog->startScan();
    }
    else
    {
        recoverStatus();
    }
}

void VideoArea::onStartConnected(const QString &ip)
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

    connect(m_long_socket, SIGNAL(readyRead()),this,SLOT(onLongSocketReadyRead()));
    connect(m_long_socket,SIGNAL(disconnected()), this, SLOT(onSocketDisconnect()));

    udpSocket->abort();
    if (!udpSocket->bind(portSetupDialog->UdpPort()))
    {
       ui->textEdit->append(udpSocket->errorString());
       return;
    }
    ui->textEdit->append(QString("Bind %2 successful !").arg(portSetupDialog->UdpPort()));
    connect(udpSocket,SIGNAL(readyRead()), this, SLOT(onUdpSocketReadyRead()));

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

void VideoArea::onLongSocketReadyRead()
{
    QByteArray message;
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
    }
    else if (readData.contains("is connecting to the server"))
    {
        recoverStatus();
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
        }
    }
}

void VideoArea::onSocketDisconnect()
{
    recoverStatus();
}

void VideoArea::onUdpSocketReadyRead()
{  
    while (udpSocket->hasPendingDatagrams())
    {
        m_frame_count++;
        if (!m_timer->isActive())
        {
            m_timer->start(1000);
        }
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagram.data(),datagram.size());
        m_load_image.loadFromData(datagram);
        if (m_load_image.size() != ui->video_label->size())
        {
            ui->video_label->setPixmap(QPixmap::fromImage(m_load_image).scaled(ui->video_label->size(),Qt::KeepAspectRatio));
        }
        else
        {
            ui->video_label->setPixmap(QPixmap::fromImage(m_load_image));
        }
        this->update();
    }
}

void VideoArea::onTimeout()
{
    QString displayMsg = QString("FPS: %1      time: %2").arg(m_frame_count)
            .arg(QTime::currentTime().toString("hh:mm:ss.zzz"));
    this->statusBar()->showMessage(displayMsg);
    m_frame_count = 0;
}

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

void VideoArea::onActioPortClicked()
{
    if (portSetupDialog->isHidden())
    {
        portSetupDialog->show();
    }
}

void VideoArea::onActionAreaViewClicked(bool flag)
{
    test_label->setVisible(flag);
}

void VideoArea::onActionServerWifiClicked()
{
    serverWifiDialog->exec();
}

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

void VideoArea::recoverStatus()
{
    ui->connect_btn->setText(tr("Connect"));
    ui->textEdit->append(QString("disconect to %1!").arg(m_server_ip));

    m_long_socket->disconnect();
    m_long_socket->abort();

    udpSocket->disconnect();
    udpSocket->abort();

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

void VideoArea::on_record_btn_clicked()
{
    ui->record_btn->setEnabled(false);
    QString cmd("set Stop.Enable=1");
    WriteData(cmd.toUtf8());
    int curSize = m_original_rect.width()*m_original_rect.height();
    ui->textEdit->append(QString("Record size: %1\r\n").arg(curSize));
}

void VideoArea::on_reset_btn_clicked()
{
    ui->record_btn->setEnabled(true);
    QString cmd("set Stop.Enable=0");
    WriteData(cmd.toUtf8());
}

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

void VideoArea::onActionButtonGroupClicked(int btnID)
{
    QString cmd = QString("Move,%1").arg(btnID);
    WriteData(cmd.toUtf8());
}

void VideoArea::onWifiChanged(const QString &userName, const QString &password)
{
    QString msg = QString("set Wifi.Settings=%1,%2").arg(userName).arg(password);
    WriteData(msg.toUtf8());
}

void VideoArea::resizeEvent(QResizeEvent *e)
{
    test_label->setGeometry(ui->video_label->geometry());
    mark_label->setGeometry(ui->video_label->geometry());
    QMainWindow::resizeEvent(e);
}
