#include "ScanIpDiaog.h"
#include "ui_ScanIpDiaog.h"

static int g_thread_count = 4;

ScanIpDiaog::ScanIpDiaog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ScanIpDiaog)
{
    ui->setupUi(this);

    m_current_status = Inital;
    m_scan_finish_count = 0;
    m_connect_finish_count = 0;
    m_tcp_port = 0;

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

ScanIpDiaog::~ScanIpDiaog()
{
    delete ui;
}

void ScanIpDiaog::setTcpPort(int port)
{
    for (int i=0; i<g_thread_count; ++i)
    {
        threadList[i]->setPort(port);
    }
}

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

void ScanIpDiaog::init()
{
    QString local_ip = localIP4();
    qDebug()<< "local ip: "<<local_ip;
    QStringList ip_splite_list = local_ip.split(".");
    if (ip_splite_list.size() < 4)
        return;

    int ip_third_val = ip_splite_list[2].toInt();
    qDebug()<< "ip_third_val: "<<ip_third_val;
    threadList[0]->setScanRange(ip_third_val,2,64);
    threadList[1]->setScanRange(ip_third_val,65,128);
    threadList[2]->setScanRange(ip_third_val,129,191);
    threadList[3]->setScanRange(ip_third_val,192,254);
}

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
        ui->reStart_btn->setText(tr("Stop"));
        ui->reStart_btn->setEnabled(true);
        ui->connect_btn->setEnabled(false);
    }

    if (this->isHidden())
        this->exec();
}

void ScanIpDiaog::loadHistotyAddress(const QStringList &list)
{
    ui->comboBox->addItems(list);
}

void ScanIpDiaog::onConnectSuccessed(const QString &addr)
{
    ui->listWidget->addItem(addr);
    updateProgress();
}

void ScanIpDiaog::onConnectFailed(const QString &msg)
{
    ui->textBrowser->append(msg);
    updateProgress();
}

void ScanIpDiaog::onScanFinished(int sec)
{
    m_scan_finish_count++;
    if (m_scan_finish_count < g_thread_count)
        return;

    ui->textBrowser->append(QString("The time elapsed %1 s\n").arg(sec));
    m_scan_finish_count = 0;
    m_current_status = Finished;
    ui->reStart_btn->setText("ReStart");
    ui->reStart_btn->setEnabled(true);
}

void ScanIpDiaog::on_reStart_btn_clicked()
{
    if (ui->reStart_btn->text() == tr("ReStart"))
    {
        ui->reStart_btn->setText("Stop");
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
    else
    {
        ui->reStart_btn->setText("ReStart");
        for (int i=0; i<g_thread_count; ++i)
        {
            threadList[i]->stop();
        }
    }
}

void ScanIpDiaog::on_quit_btn_clicked()
{
    this->hide();
}

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

    emit startConnected(current_ip);
    this->hide();
}

void ScanIpDiaog::on_listWidget_clicked(const QModelIndex &index)
{
    Q_UNUSED(index);

    if (!ui->connect_btn->isEnabled())
    {
        ui->connect_btn->setEnabled(true);
    }
}

void ScanIpDiaog::onDetailBtnClicked(bool checked)
{
    ui->textBrowser->setVisible(checked);
}

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

void ScanIpDiaog::updateProgress()
{
    m_connect_finish_count++;
    ui->progressBar->setValue(m_connect_finish_count);
}

void ScanIpDiaog::on_input_edit_textChanged(const QString &arg1)
{
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
