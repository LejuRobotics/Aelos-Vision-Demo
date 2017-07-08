#ifndef SCANIPDIAOG_H
#define SCANIPDIAOG_H

#include "precompiled.h"
#include "ScanIpThread.h"

namespace Ui {
class ScanIpDiaog;
}

class ScanIpDiaog : public QDialog
{
    Q_OBJECT

public:
    explicit ScanIpDiaog(QWidget *parent = 0);
    ~ScanIpDiaog();

    enum Status{ Inital, Scanning, Finished };

    void setTcpPort(int port);

    QString localIP4(); //get the local IP4 address

    void init(); // initialize function

    void startScan(); // start scanning and try to connect to the IP address

    void loadHistotyAddress(const QStringList &list);

private slots:
    void onConnectSuccessed(const QString &addr);

    void onConnectFailed(const QString &msg);

    void onScanFinished(int sec);

    void on_reStart_btn_clicked();

    void on_quit_btn_clicked();

    void on_listWidget_clicked(const QModelIndex &index);

    void on_connect_btn_clicked();

    void onDetailBtnClicked(bool checked);

    void onRadioButtonGroupToggled(int id, bool checked);

    void on_input_edit_textChanged(const QString &arg1);

signals:
    void startConnected(const QString &); //send this signal when conect_btn is clicked

private:
    Ui::ScanIpDiaog *ui;

    QButtonGroup *radio_button_group;

    QList<ScanIpThread*> threadList;

    Status m_current_status;

    int m_tcp_port;
    int m_scan_finish_count;
    int m_connect_finish_count;

    void updateProgress();

};

#endif // SCANIPDIAOG_H
