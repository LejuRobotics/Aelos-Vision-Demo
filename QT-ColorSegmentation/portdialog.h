#ifndef PORTDIALOG_H
#define PORTDIALOG_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class portDialog;
}

class portDialog : public QDialog
{
    Q_OBJECT

public:
    explicit portDialog(QWidget *parent = 0);
    ~portDialog();

    int udpPort() const ;
    void setUdpPort(int _port);

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::portDialog *ui;
    int m_udpPort;
};

#endif // PORTDIALOG_H
