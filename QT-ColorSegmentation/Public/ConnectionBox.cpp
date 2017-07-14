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

    ui->tableWidget->horizontalHeader()->setHighlightSections(false); //点击表时不对表头行光亮（获取焦点）
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);  //设置充满表宽度
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);  //设置选择行为时每次选择一行
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection); //设置单选模式
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers); //设置不可编辑
}

ConnectionBox::~ConnectionBox()
{
    delete ui;
}

void ConnectionBox::addConnection(const QString &addr, const QString &robotNo)
{
    m_broadcastCount++;
    if (m_broadcastCount > 9)
    {
        m_connectionList.clear();
        for (int i=ui->tableWidget->rowCount()-1; i>-1; --i)
        {
            ui->tableWidget->removeRow(i);
        }
        m_broadcastCount = 0;
    }
    if (!m_connectionList.contains(addr))
    {
        m_connectionList.append(addr);
        ui->tableWidget->insertRow(ui->tableWidget->rowCount());
        ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 0, new QTableWidgetItem(addr));
        ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 1, new QTableWidgetItem(robotNo));
        ui->tableWidget->item(ui->tableWidget->rowCount()-1,0)->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->item(ui->tableWidget->rowCount()-1,1)->setTextAlignment(Qt::AlignCenter);
        if (ui->tableWidget->currentRow() < 0)
        {
            ui->tableWidget->setCurrentCell(0,0);
            ui->connect_btn->setEnabled(true);
        }
    }

    if (this->isHidden() && m_showEnable)
    {
        this->show();
    }
}

void ConnectionBox::on_connect_btn_clicked()
{
    this->hide();
    emit startConnectTo(ui->tableWidget->item(ui->tableWidget->currentRow(),0)->text());
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

void ConnectionBox::resizeEvent(QResizeEvent *e)
{
    int tableW = ui->tableWidget->width()-30;
    ui->tableWidget->horizontalHeader()->resizeSection(0,tableW/2); //设置列宽
    ui->tableWidget->horizontalHeader()->resizeSection(1,tableW/2);
    QDialog::resizeEvent(e);
}

void ConnectionBox::on_tableWidget_cellClicked(int row, int column)
{
    Q_UNUSED(row);
    Q_UNUSED(column);
    if (!ui->connect_btn->isEnabled())
    {
        ui->connect_btn->setEnabled(true);
    }
}
