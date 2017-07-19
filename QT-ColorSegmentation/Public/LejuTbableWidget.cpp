#include "LejuTbableWidget.h"

LejuTbableWidget::LejuTbableWidget(QWidget *parent) : QTableWidget(parent)
{
    //设置表头
    this->setColumnCount(5);
    m_headLabelList << tr("Name") << tr("Time") << tr("Color") << tr("Size") << tr("Type");
    this->setHorizontalHeaderLabels(m_headLabelList);

    this->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    this->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    this->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);

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

void LejuTbableWidget::addItem(const QString &pTime, const QString &pColor, int &pSize)
{
    this->insertRow(this->rowCount());
    QComboBox *combox = new QComboBox(this);
    combox->addItem(tr("目标"));
    combox->addItem(tr("障碍物"));

    LejuColorLabel *colorLabel = new LejuColorLabel(this);
    colorLabel->setColor(pColor);

    this->setItem(this->rowCount()-1,0,new QTableWidgetItem(QString(tr("目标%1")).arg(this->rowCount())));
    this->setItem(this->rowCount()-1,1,new QTableWidgetItem(pTime));
    this->setCellWidget(this->rowCount()-1, 2, colorLabel);
    this->setItem(this->rowCount()-1,3,new QTableWidgetItem(QString::number(pSize)));
    this->setCellWidget(this->rowCount()-1, 4, combox);

    this->item(this->rowCount()-1,1)->setTextAlignment(Qt::AlignCenter);
    this->item(this->rowCount()-1,3)->setTextAlignment(Qt::AlignCenter);
}


void LejuTbableWidget::onDeleteItem()
{
    emit deleteItem(this->currentRow());
    removeRow(this->currentRow());
}

void LejuTbableWidget::contextMenuEvent(QContextMenuEvent *e)
{
    menu->exec(QCursor::pos());
    QTableWidget::contextMenuEvent(e);
}
