/**
 * @file       ScanIpDiaog.cpp
 * @version    1.0
 * @date       2017年07月08日
 * @author     C_Are
 * @copyright  Leju
 *
 * @brief      连接机器人窗口，ScanIpDiaog类的cpp文件
 */

#include "ScanIpDiaog.h"
#include "ui_ScanIpDiaog.h"

static int g_thread_count = 4; //目前开启4条线程同时扫描局域网内连接的机器人

/**
 * @brief     ScanIpDiaog类的构造函数
 * @param     parent 父对象
 * @details   初始化
 */

ScanIpDiaog::ScanIpDiaog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ScanIpDiaog)
{
    ui->setupUi(this);

    m_current_status = Inital;
    m_scan_finish_count = 0;
    m_connect_finish_count = 0;
    m_tcp_port = 0;
    m_bIsFirstChanged = false;

    ui->mainLayout->setSizeConstraint(QLayout::SetFixedSize);

    radio_button_group = new QButtonGroup(this);
    radio_button_group->addButton(ui->scan_radio, 0);
    radio_button_group->addButton(ui->input_radio, 1);
    radio_button_group->addButton(ui->history_ratio, 2);
    connect(radio_button_group, SIGNAL(buttonToggled(int,bool)), this, SLOT(onRadioButtonGroupToggled(int,bool)));

    connect(ui->detail_btn, SIGNAL(toggled(bool)), this, SLOT(onDetailBtnClicked(bool)));
    ui->detail_btn->setChecked(false);

    QRegExp ipRegExp("((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])[\\.]){3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])");
    ui->input_edit->setValidator(new QRegExpValidator(ipRegExp,this));

    ui->progressBar->setMaximum(253);

    for (int i=0; i<g_thread_count; ++i)
    {
        threadList << new ScanIpThread(this);
        connect(threadList[i], SIGNAL(connectSuccessed(QString)), this, SLOT(onConnectSuccessed(QString)));
        connect(threadList[i], SIGNAL(connectFailed(QString)), this, SLOT(onConnectFailed(QString)));
        connect(threadList[i], SIGNAL(scanFinished(int)), this, SLOT(onScanFinished(int)));
    }
}

/**
 * @brief     ScanIpDiaog类的析构函数
 * @details   销毁ui指针
 */

ScanIpDiaog::~ScanIpDiaog()
{
    delete ui;
}

/**
 * @brief     设置tcp端口
 * @param     port 端口号
 */

void ScanIpDiaog::setTcpPort(int port)
{
    for (int i=0; i<g_thread_count; ++i)
    {
        threadList[i]->setPort(port);
    }
}

/**
 * @brief     获取本地ip4
 */

QString ScanIpDiaog::localIP4()
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

/**
 * @brief     初始化，设定每个线程扫描的ip范围
 */

void ScanIpDiaog::init()
{
    QString local_ip = localIP4();
//    qDebug()<< "local ip: "<<local_ip;
    QStringList ip_splite_list = local_ip.split(".");
    if (ip_splite_list.size() < 4)
        return;

    emit sendInfo("local ip: "+local_ip);
    int ip_third_val = ip_splite_list[2].toInt();
//    qDebug()<< "ip_third_val: "<<ip_third_val;
    threadList[0]->setScanRange(ip_third_val,2,64);
    threadList[1]->setScanRange(ip_third_val,65,128);
    threadList[2]->setScanRange(ip_third_val,129,191);
    threadList[3]->setScanRange(ip_third_val,192,254);
}

/**
 * @brief     开始扫描
 */

void ScanIpDiaog::startScan()
{
    if (m_current_status == Inital)
    {
        init();
        for (int i=0; i<g_thread_count; ++i)
        {
            threadList[i]->startScan();
        }
        m_current_status = Scanning;
        ui->scan_btn->setText(tr("Stop"));
        ui->scan_btn->setEnabled(true);
        ui->connect_btn->setEnabled(false);
    }
}

/**
 * @brief     读取配置文件中的历史连接记录
 * @param     list 历史连接记录
 */

void ScanIpDiaog::loadHistotyAddress(const QStringList &list)
{
    ui->comboBox->addItems(list);
}

/**
 * @brief     连接connectSuccessed(QString)信号的槽函数
 * @param     addr ip地址
 * @details   扫描到可连接的ip则添加到ui->listWidget中
 */

void ScanIpDiaog::onConnectSuccessed(const QString &addr)
{
    ui->listWidget->addItem(addr);
    updateProgress();

    if (ui->listWidget->currentRow() != 0)
    {
        ui->listWidget->setCurrentRow(0);
    }

    if (!ui->connect_btn->isEnabled())
    {
        ui->connect_btn->setEnabled(true);
    }
}

/**
 * @brief     连接connectFailed(QString)信号的槽函数
 * @param     msg 连接失败的信息
 * @details   会将连接失败的信息加到ui->textBrowser中
 */

void ScanIpDiaog::onConnectFailed(const QString &msg)
{
    ui->textBrowser->append(msg);
    updateProgress();
}

/**
 * @brief     连接scanFinished(int)信号的槽函数
 * @param     sec 扫描完成用掉的时间
 * @details   会将扫描完成用掉的时间加到ui->textBrowser中
 */

void ScanIpDiaog::onScanFinished(int sec)
{
    m_scan_finish_count++;
    if (m_scan_finish_count < g_thread_count)
        return;

    ui->textBrowser->append(QString("The time elapsed %1 s\n").arg(sec));
    m_scan_finish_count = 0;
    m_current_status = Finished;
    ui->scan_btn->setText("ReScan");
    ui->scan_btn->setEnabled(true);
}

/**
 * @brief     点击scan按钮，开始，重新或停止扫描
 */

void ScanIpDiaog::on_scan_btn_clicked()
{
    if (ui->scan_btn->text() == tr("Scan"))
    {
        ui->scan_btn->setText("Stop");
        startScan();
    }
    else if (ui->scan_btn->text() == tr("Stop"))
    {
        ui->scan_btn->setText("ReScan");
        for (int i=0; i<g_thread_count; ++i)
        {
            threadList[i]->stop();
        }
    }
    else if (ui->scan_btn->text() == tr("ReScan"))
    {
        ui->scan_btn->setText("Stop");
        m_current_status = Inital;
        m_connect_finish_count = 0;
        ui->progressBar->reset();
        ui->textBrowser->clear();
        for (int i=ui->listWidget->count()-1; i>-1; i--)
        {
            QListWidgetItem *item = ui->listWidget->takeItem(i);
            delete item;
        }
        startScan();
    }
}

/**
 * @brief     点击Quit按钮，隐藏窗口
 */

void ScanIpDiaog::on_quit_btn_clicked()
{
    this->hide();
}

/**
 * @brief     点击Connect按钮，发送startConnected(QString)信号
 */

void ScanIpDiaog::on_connect_btn_clicked()
{
    QString current_ip;
    switch (radio_button_group->checkedId()) {
    case 0:
        current_ip = ui->listWidget->currentItem()->text();
        break;
    case 1:
        current_ip = ui->input_edit->text();
        break;
    case 2:
        current_ip = ui->comboBox->currentText();
        break;
    default:
        break;
    }

    emit startConnectTo(current_ip);
    this->hide();
}

/**
 * @brief     点击ui->listWidget的item执行的槽函数
 */

void ScanIpDiaog::on_listWidget_clicked(const QModelIndex &index)
{
    Q_UNUSED(index);

    if (!ui->scan_radio->isChecked())
    {
        ui->scan_radio->setChecked(true);
    }

    if (!ui->connect_btn->isEnabled())
    {
        ui->connect_btn->setEnabled(true);
    }
}

/**
 * @brief     显示或隐藏ui->textBrowser
 * @param     checked true显示，false隐藏
 */

void ScanIpDiaog::onDetailBtnClicked(bool checked)
{
    ui->textBrowser->setVisible(checked);
}

/**
 * @brief     选择指定方式连接ip
 */

void ScanIpDiaog::onRadioButtonGroupToggled(int id, bool checked)
{
    if (checked)
    {
        switch (id) {
        case 0:
            ui->connect_btn->setEnabled(ui->listWidget->currentRow() > -1);
            break;
        case 1:
            ui->connect_btn->setEnabled(!ui->input_edit->text().isEmpty());
            break;
        case 2:
            ui->connect_btn->setEnabled(ui->comboBox->count() > 0);
            break;
        default:
            break;
        }
    }
}

/**
 * @brief     更新进度条
 */

void ScanIpDiaog::updateProgress()
{
    m_connect_finish_count++;
    ui->progressBar->setValue(m_connect_finish_count);
}

/**
 * @brief     当ip输入框有变化执行的槽函数
 * @param     arg1  输入框内容
 */

void ScanIpDiaog::on_input_edit_textChanged(const QString &arg1)
{
    if (!ui->input_radio->isChecked())
    {
        ui->input_radio->setChecked(true);
    }

    if (!arg1.isEmpty())
    {
        if (!ui->connect_btn->isEnabled())
            ui->connect_btn->setEnabled(true);
    }
    else
    {
        ui->connect_btn->setEnabled(false);
    }
}

/**
 * @brief     当ui->comboBox的选项变化执行的槽函数
 * @param     index 当前的选项
 */

void ScanIpDiaog::on_comboBox_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    if (!m_bIsFirstChanged)
    {
        m_bIsFirstChanged = true;
        return;
    }

    if (!ui->history_ratio->isChecked())
    {
        ui->history_ratio->setChecked(true);
    }
}
