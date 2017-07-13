/**
 * @file       ConnectionBox.cpp
 * @version    1.0
 * @date       2017年07月011日
 * @author     C_Are
 * @copyright  Leju
 *
 * @brief      有可连接的ip地址提示框，ConnectionBox类的cpp文件
 */

#include "ConnectionBox.h"
#include "ui_ConnectionBox.h"

ConnectionBox::ConnectionBox(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectionBox)
{
    ui->setupUi(this);
    m_broadcastCount = 0;
    m_showEnable = true;

    ui->connect_btn->setEnabled(false);
}

ConnectionBox::~ConnectionBox()
{
    delete ui;
}

void ConnectionBox::addConnection(const QString &addr)
{
    m_broadcastCount++;
    if (m_broadcastCount > 9)
    {
        m_connectionList.clear();
        for (int i=ui->listWidget->count()-1; i>-1; i--)
        {
            QListWidgetItem *item = ui->listWidget->takeItem(i);
            delete item;
        }
    }
    if (!m_connectionList.contains(addr))
    {
        m_connectionList.append(addr);
        ui->listWidget->addItem(addr);
    }

    if (this->isHidden() && m_showEnable)
    {
        this->show();
    }
}

void ConnectionBox::on_connect_btn_clicked()
{
    this->hide();
    emit startConnectTo(ui->listWidget->currentItem()->text());   
}

void ConnectionBox::on_listWidget_currentRowChanged(int currentRow)
{
    if (currentRow > -1)
    {
        if (!ui->connect_btn->isEnabled())
        {
            ui->connect_btn->setEnabled(true);
        }
    }
    else
    {
        if (ui->connect_btn->isEnabled())
        {
            ui->connect_btn->setEnabled(false);
        }
    }
}

void ConnectionBox::onTimeout()
{
    m_showEnable = true;
}

void ConnectionBox::closeEvent(QCloseEvent *e)
{
    m_showEnable = false;
    QTimer::singleShot(10000, this, SLOT(onTimeout()));
    QDialog::closeEvent(e);
}
