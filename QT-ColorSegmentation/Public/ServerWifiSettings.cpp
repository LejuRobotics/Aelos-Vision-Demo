#include "ServerWifiSettings.h"
#include "ui_serverwifisettings.h"

ServerWifiSettings::ServerWifiSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ServerWifiSettings)
{
    ui->setupUi(this);
}

ServerWifiSettings::~ServerWifiSettings()
{
    delete ui;
}

void ServerWifiSettings::on_ok_btn_clicked()
{
    this->hide();
    emit wifiChanged(ui->user_edit->text(), ui->password_edit->text());
}

void ServerWifiSettings::on_cancel_btn_clicked()
{
    this->hide();
}
