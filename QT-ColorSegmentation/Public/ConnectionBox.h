/**
 * @file       ConnectionBox.h
 * @version    1.0
 * @date       2017年07月011日
 * @author     C_Are
 * @copyright  Leju
 *
 * @brief      有可连接的ip地址提示框，ConnectionBox类的h文件
 */

#ifndef CONNECTIONBOX_H
#define CONNECTIONBOX_H

#include "precompiled.h"

/**
 * @class     ConnectionBox
 * @brief     显示局域网内可连接的ip地址
 */

namespace Ui {
class ConnectionBox;
}

class ConnectionBox : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectionBox(QWidget *parent = 0);
    ~ConnectionBox();

    void addConnection(const QString &addr, const QString &robotNo);

signals:
    void startConnectTo(const QString &);

private slots:
    void on_connect_btn_clicked();

    void onTimeout();

    void on_tableWidget_cellClicked(int row, int column);

protected:
    virtual void closeEvent(QCloseEvent *e);
    virtual void resizeEvent(QResizeEvent *e);

private:
    Ui::ConnectionBox *ui;

    QStringList m_connectionList;
    int m_broadcastCount;
    bool m_showEnable;
};

#endif // CONNECTIONBOX_H
