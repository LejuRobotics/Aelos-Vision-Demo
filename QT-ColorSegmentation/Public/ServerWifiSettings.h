#ifndef SERVERWIFISETTINGS_H
#define SERVERWIFISETTINGS_H

#include <QDialog>

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
