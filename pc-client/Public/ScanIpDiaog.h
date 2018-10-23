/**
 * @file       ScanIpDiaog.h
 * @version    1.0
 * @date       2017年07月08日
 * @author     C_Are
 * @copyright  Leju
 *
 * @brief      连接机器人窗口，ScanIpDiaog类的h文件
 */

#ifndef SCANIPDIAOG_H
#define SCANIPDIAOG_H

#include "precompiled.h"
#include "ScanIpThread.h"

/**
 * @class     ScanIpDiaog
 * @brief     连接机器人的窗口
 * @details   可以通过扫描、输入,和历史记录尝试连接机器人
 */

namespace Ui {
class ScanIpDiaog;
}

class ScanIpDiaog : public QDialog
{
    Q_OBJECT

public:
    explicit ScanIpDiaog(QWidget *parent = 0);
    ~ScanIpDiaog();

    /**
     * @brief     扫描状态
     */

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

    void on_scan_btn_clicked();

    void on_quit_btn_clicked();

    void on_listWidget_clicked(const QModelIndex &index);

    void on_connect_btn_clicked();

    void onDetailBtnClicked(bool checked);

    void onRadioButtonGroupToggled(int id, bool checked);

    void on_input_edit_textChanged(const QString &arg1);

    void on_comboBox_currentIndexChanged(int index);

signals:
    void startConnectTo(const QString &); //send this signal when conect_btn is clicked
    void sendInfo(const QString &);

private:
    Ui::ScanIpDiaog *ui;

    QButtonGroup *radio_button_group;

    QList<ScanIpThread*> threadList;

    Status m_current_status;

    int m_tcp_port;
    int m_scan_finish_count;
    int m_connect_finish_count;
    bool m_bIsFirstChanged;

    void updateProgress();

};

#endif // SCANIPDIAOG_H
