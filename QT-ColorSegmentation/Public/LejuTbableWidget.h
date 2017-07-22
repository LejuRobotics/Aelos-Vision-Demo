/**
 * @file       LejuTbableWidget.h
 * @version    1.0
 * @date       2017年07月22日
 * @author     C_Are
 * @copyright  Leju
 *
 * @brief      记录标记颜色的列表，LejuTbableWidget类的h文件
 */

#ifndef LEJUTBABLEWIDGET_H
#define LEJUTBABLEWIDGET_H

#include "precompiled.h"

class LejuColorLabel : public QWidget
{
    Q_OBJECT
public:
    explicit LejuColorLabel(QWidget *parent = 0) : QWidget(parent)
    {
        colorLabel  = new QLabel(this);
    }

    ~LejuColorLabel() {}

    void setColor(const QString &color)
    {
       colorLabel->setStyleSheet(QString("background-color: rgb(%1)").arg(color));
    }

protected:
    virtual void resizeEvent(QResizeEvent *e)
    {
        colorLabel->setGeometry(this->width()*0.15, this->height()*0.1,
                                this->width()*0.7, this->height()*0.8);
        QWidget::resizeEvent(e);
    }

private:
    QLabel *colorLabel;
};

class LejuTbableWidget : public QTableWidget
{
    Q_OBJECT
public:
    explicit LejuTbableWidget(QWidget *parent = 0);
    ~LejuTbableWidget();

    void addItem(const QString &pTime, const QString &pColor, int pWidth);
    QString lastName() const;
    int lastType() const;
    int lastTurn() const;

protected:
    virtual void contextMenuEvent(QContextMenuEvent *e);

signals:
    void sendData(const QByteArray &msg);  /**< 通过tcp发送指令的信号 */

private slots:
    void onDeleteItem();
    void onTypeChanged(int index);
    void onTurnChanged(int index);

private:
    QStringList m_headLabelList;
    QMenu *menu;
};

#endif // LEJUTBABLEWIDGET_H
