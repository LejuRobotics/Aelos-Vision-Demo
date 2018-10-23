/**
 * @file       PortSetupDialog.cpp
 * @version    1.0
 * @date       2017年07月08日
 * @author     C_Are
 * @copyright  Leju
 *
 * @brief      端口设置窗口，PortSetupDialog类的cpp文件
 * @details    可设置连接服务器的tcp端口以及udp的监听端口
 */

#include "PortSetupDialog.h"
#include "ui_PortSetupDialog.h"

/**
 * @brief     PortSetupDialog类的构造函数
 * @param     parent 父对象
 */

PortSetupDialog::PortSetupDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PortSetupDialog)
{
    ui->setupUi(this);

    m_tcpPort = 7980;
    m_udpPort = 5713;
}

/**
 * @brief     PortSetupDialog类的析构函数
 * @details   销毁动态创建的ui指针
 */

PortSetupDialog::~PortSetupDialog()
{
    delete ui;
}

/**
 * @brief     根据参数类型设置数值，初始化函数
 * @param     _type 端口类型
 * @param     val 设置的大小
 */

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

/**
 * @return  返回正确的TCP端口
 */

int PortSetupDialog::TcpPort() const
{
    return m_tcpPort;
}

/**
 * @return  返回正确的UDP端口
 */

int PortSetupDialog::UdpPort() const
{
    return m_udpPort;
}

/**
 * @brief    点击Ok按钮执行的槽函数
 * @details  将更改的数值保存到配置文件，让程序下次启动的时候设置之前保存的数值
 */

void PortSetupDialog::on_ok_btn_clicked()
{
    this->hide();
    int lastTcpPort = ui->tcp_port_edit->text().toInt();
    int lastUdpPort = ui->udp_port_edit->text().toInt();
    if (lastTcpPort == m_tcpPort && lastUdpPort == m_udpPort)
        return;

    QSettings iniReader("config.ini",QSettings::IniFormat);
    if (lastTcpPort != m_tcpPort)
    {
        m_tcpPort = lastTcpPort;
        iniReader.setValue("Port/tcpPort",m_tcpPort);
    }
    if (lastUdpPort != m_udpPort)
    {
        m_udpPort = lastUdpPort;
        iniReader.setValue("Port/udpPort",m_udpPort);
        emit udpPortChanged();
    }
}

/**
 * @brief     点击Cancel按钮执行的槽函数
 * @details   取消之前的修改操作
 */

void PortSetupDialog::on_cancel_btn_clicked()
{
    this->hide();
    ui->tcp_port_edit->setText(QString::number(m_tcpPort));
    ui->udp_port_edit->setText(QString::number(m_udpPort));
}
