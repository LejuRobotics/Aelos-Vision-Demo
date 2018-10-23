/**
 * @file       ServerWifiSettings.h
 * @version    1.0
 * @date       2017年07月08日
 * @author     C_Are
 * @copyright  Leju
 *
 * @brief      wifi设置窗口，ServerWifiSettings类的h文件
 * @details    向已连接的服务器发送设置的wifi账号密码，从而让服务器（机器人）连接上wifi
 */

#ifndef SERVERWIFISETTINGS_H
#define SERVERWIFISETTINGS_H

#include "precompiled.h"

/**
 * @class     ServerWifiSettings
 * @brief     继续QDialog类的wifi设置界面
 * @details   向已连接的服务器发送设置的wifi账号密码，从而让服务器（机器人）连接上wifi
 */

namespace Ui {
class ServerWifiSettings;
}

class ServerWifiSettings : public QDialog
{
    Q_OBJECT

public:
    explicit ServerWifiSettings(QWidget *parent = 0);
    ~ServerWifiSettings();

signals:
    void wifiChanged(const QString &userName, const QString &password);

private slots:
    void on_ok_btn_clicked();

    void on_cancel_btn_clicked();

private:
    Ui::ServerWifiSettings *ui;
};

#endif // SERVERWIFISETTINGS_H
