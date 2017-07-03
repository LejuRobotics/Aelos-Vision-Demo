#include "portdialog.h"
#include "ui_portdialog.h"

portDialog::portDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::portDialog)
{
    ui->setupUi(this);
    m_udpPort = 5713;
}

portDialog::~portDialog()
{
    delete ui;
}

int portDialog::udpPort() const
{
    return m_udpPort;
}

void portDialog::setUdpPort(int _port)
{
    m_udpPort = _port;
    ui->lineEdit->setText(QString::number(m_udpPort));
}

void portDialog::on_buttonBox_accepted()
{
   this->hide();
   m_udpPort = ui->lineEdit->text().toInt();
   QSettings iniReader("config.ini",QSettings::IniFormat);
   iniReader.setValue("Record/udpPort",m_udpPort);
}

void portDialog::on_buttonBox_rejected()
{
    this->hide();
    ui->lineEdit->setText(QString::number(m_udpPort));
}
