/**
 * @file       PortSetupDialog.h
 * @version    1.0
 * @date       2017年07月08日
 * @author     C_Are
 * @copyright  Leju
 *
 * @brief      端口设置窗口，PortSetupDialog类的h文件
 * @details    可设置连接服务器的tcp端口以及udp的监听端口
 */

#ifndef PORTSETUPDIALOG_H
#define PORTSETUPDIALOG_H

#include "precompiled.h"

/**
 * @class     PortSetupDialog
 * @brief     继续QDialog类的端口设置界面
 * @details   可设置连接服务器的tcp端口以及udp的监听端口
 */

namespace Ui {
class PortSetupDialog;
}

class PortSetupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PortSetupDialog(QWidget *parent = 0);
    ~PortSetupDialog();

    /**
     * @brief 端口类型
     */

    enum Type { TCP_Port, UDP_Port };

    void setValue(Type _type, const QString &val);

    int TcpPort() const;
    int UdpPort() const;

signals:
    void udpPortChanged();

private slots:
    void on_ok_btn_clicked();

    void on_cancel_btn_clicked();

private:
    Ui::PortSetupDialog *ui;

    int m_tcpPort;    /**< TCP端口 */
    int m_udpPort;    /**< UDP端口 */
};

#endif // PORTSETUPDIALOG_H
