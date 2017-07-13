/**
 * @file       ServerWifiSettings.cpp
 * @version    1.0
 * @date       2017年07月08日
 * @author     C_Are
 * @copyright  Leju
 *
 * @brief      wifi设置窗口，ServerWifiSettings类的cpp文件
 * @details    向已连接的服务器发送设置的wifi账号密码，从而让服务器（机器人）连接上wifi
 */

#include "ServerWifiSettings.h"
#include "ui_serverwifisettings.h"

/**
 * @brief     ServerWifiSettings类的构造函数
 * @param     parent 父对象
 */

ServerWifiSettings::ServerWifiSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ServerWifiSettings)
{
    ui->setupUi(this);
}

/**
 * @brief     ServerWifiSettings类的析构函数
 * @param     销毁动态创建的ui指针
 */

ServerWifiSettings::~ServerWifiSettings()
{
    delete ui;
}

/**
 * @brief    点击Ok按钮执行的槽函数
 * @details  发送wifiChanged()信号
 */

void ServerWifiSettings::on_ok_btn_clicked()
{
    this->hide();
    emit wifiChanged(ui->user_edit->text(), ui->password_edit->text());
}

/**
 * @brief    点击Ok按钮执行的槽函数
 * @details  只是隐藏窗口
 */

void ServerWifiSettings::on_cancel_btn_clicked()
{
    this->hide();
}
