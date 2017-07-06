#include "PortSetupDialog.h"
#include "ui_PortSetupDialog.h"

PortSetupDialog::PortSetupDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PortSetupDialog)
{
    ui->setupUi(this);

    m_tcpPort = 7980;
    m_udpPort = 5713;
}

PortSetupDialog::~PortSetupDialog()
{
    delete ui;
}

void PortSetupDialog::setValue(PortSetupDialog::Type _type, const QString &val)
{
    switch (_type) {
    case TCP_Port:
        ui->tcp_port_edit->setText(val);
        m_tcpPort = val.toInt();
        break;
    case UDP_Port:
        ui->udp_port_edit->setText(val);
        m_udpPort = val.toInt();
        break;
    default:
        break;
    }
}

int PortSetupDialog::TcpPort() const
{
    return m_tcpPort;
}

int PortSetupDialog::UdpPort() const
{
    return m_udpPort;
}

void PortSetupDialog::on_ok_btn_clicked()
{
    this->hide();
    int lastTcpPort = ui->tcp_port_edit->text().toInt();
    int lastUdpPort = ui->udp_port_edit->text().toInt();
    if (lastTcpPort != m_tcpPort && lastUdpPort == m_udpPort)
        return;

    QSettings iniReader("config.ini",QSettings::IniFormat);
    if (lastTcpPort != m_tcpPort)
    {
        m_tcpPort = lastTcpPort;
        iniReader.setValue("Record/tcpPort",m_tcpPort);
    }
    if (lastUdpPort != m_udpPort)
    {
        m_udpPort = lastUdpPort;
        iniReader.setValue("Record/udpPort",m_udpPort);
    }
}

void PortSetupDialog::on_cancel_btn_clicked()
{
    this->hide();
    ui->tcp_port_edit->setText(QString::number(m_tcpPort));
    ui->udp_port_edit->setText(QString::number(m_udpPort));
}
