#ifndef PORTSETUPDIALOG_H
#define PORTSETUPDIALOG_H

#include "precompiled.h"

namespace Ui {
class PortSetupDialog;
}

class PortSetupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PortSetupDialog(QWidget *parent = 0);
    ~PortSetupDialog();

    enum Type { TCP_Port, UDP_Port };

    void setValue(Type _type, const QString &val);

    int TcpPort() const;
    int UdpPort() const;

private slots:
    void on_ok_btn_clicked();

    void on_cancel_btn_clicked();

private:
    Ui::PortSetupDialog *ui;

    int m_tcpPort;
    int m_udpPort;
};

#endif // PORTSETUPDIALOG_H
