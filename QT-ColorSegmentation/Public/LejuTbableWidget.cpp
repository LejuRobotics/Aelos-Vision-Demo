/**
 * @file       LejuTbableWidget.cpp
 * @version    1.0
 * @date       2017年07月22日
 * @author     C_Are
 * @copyright  Leju
 *
 * @brief      记录标记颜色的列表，LejuTbableWidget类的cpp文件
 */

#include "LejuTbableWidget.h"

LejuTbableWidget::LejuTbableWidget(QWidget *parent) : QTableWidget(parent)
{
    //设置表头
    setColumnCount(7);
    m_headLabelList << tr("Name") << tr("Time") << tr("Color")
                    << tr("Width") << tr("Type") << tr("Turn")
                    << tr("State");
    setHorizontalHeaderLabels(m_headLabelList);

    horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);

    //创建右键菜单
    menu = new QMenu(this);
    QAction *action = new QAction(this);
    action->setText(tr("删除"));
    connect(action,SIGNAL(triggered()),this,SLOT(onDeleteItem()));
    menu->addAction(action);
}

LejuTbableWidget::~LejuTbableWidget()
{

}

void LejuTbableWidget::addItem(const QString &pTime, const QString &pColor, int pWidth)
{
    insertRow(rowCount());
    QComboBox *combox = new QComboBox(this);
    combox->setProperty("row", rowCount()-1);
    combox->addItem(tr("障碍物"));
    combox->addItem(tr("目标"));
    connect(combox, SIGNAL(currentIndexChanged(int)), this, SLOT(onTypeChanged(int)));

    QComboBox *combox_2 = new QComboBox(this);
    combox_2->setProperty("row", rowCount()-1);
    combox_2->addItem(tr("Left"));
    combox_2->addItem(tr("Right"));
    connect(combox_2, SIGNAL(currentIndexChanged(int)), this, SLOT(onTurnChanged(int)));

    LejuColorLabel *colorLabel = new LejuColorLabel(this);
    colorLabel->setColor(pColor);

    setItem(rowCount()-1,0,new QTableWidgetItem(QString(tr("目标%1")).arg(rowCount())));
    setItem(rowCount()-1,1,new QTableWidgetItem(pTime));
    setCellWidget(rowCount()-1, 2, colorLabel);
    setItem(rowCount()-1,3,new QTableWidgetItem(QString::number(pWidth)));
    setCellWidget(rowCount()-1, 4, combox);
    setCellWidget(rowCount()-1, 5, combox_2);
    setItem(rowCount()-1,6,new QTableWidgetItem(tr("未完成")));

    item(rowCount()-1,1)->setTextAlignment(Qt::AlignCenter);
    item(rowCount()-1,3)->setTextAlignment(Qt::AlignCenter);
    item(rowCount()-1,6)->setTextAlignment(Qt::AlignCenter);
}

QString LejuTbableWidget::lastName() const
{
    return item(rowCount()-1, 0)->text();
}

int LejuTbableWidget::lastType() const
{
    QComboBox *box = (QComboBox*)cellWidget(rowCount()-1,4);
    return box->currentIndex();
}

int LejuTbableWidget::lastTurn() const
{
    QComboBox *box = (QComboBox*)cellWidget(rowCount()-1,5);
    return box->currentIndex();
}

bool LejuTbableWidget::isAllFinished() const
{
    for (int i=0; i<rowCount(); ++i)
    {
        if (item(i, 6)->text() == tr("未完成"))
        {
            return false;
        }
    }
    return true;
}

void LejuTbableWidget::onDeleteItem()
{
    QString msg;
    msg = QString("Remove Target=%1").arg(currentRow());
    emit sendData(msg.toUtf8());
    removeRow(currentRow());
}

void LejuTbableWidget::onTypeChanged(int index)
{
    int nRow = sender()->property("row").toInt();
    QString msg;
    msg = QString("set Target.Type=%1,%2").arg(nRow).arg(index);
    emit sendData(msg.toUtf8());
}

void LejuTbableWidget::onTurnChanged(int index)
{
    int nRow = sender()->property("row").toInt();
    QString msg;
    msg = QString("set Target.Turn=%1,%2").arg(nRow).arg(index);
    emit sendData(msg.toUtf8());
}

void LejuTbableWidget::contextMenuEvent(QContextMenuEvent *e)
{
    menu->exec(QCursor::pos());
    QTableWidget::contextMenuEvent(e);
}

