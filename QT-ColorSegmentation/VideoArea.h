#ifndef VIDEOAREA_H
#define VIDEOAREA_H

#include "precompiled.h"
#include "painterlabel.h"
#include "PortSetupDialog.h"
#include "ScanIpDiaog.h"
#include "ServerWifiSettings.h"

namespace Ui {
class VideoArea;
}

class VideoArea : public QMainWindow
{
    Q_OBJECT

public:
    explicit VideoArea(QWidget *parent = 0);
    ~VideoArea();

    void WriteData(const QByteArray &msg);

private slots:    
    void on_connect_btn_clicked();
    void onStartConnected(const QString &ip);

    void onLongSocketReadyRead();
    void onSocketDisconnect();

    void onUdpSocketReadyRead();
    void onTimeout();

    void selectFinished(const QRect &_rect);

    //clicked on menubar
    void onActioPortClicked();
    void onActionAreaViewClicked(bool flag);
    void onActionServerWifiClicked();

    void on_record_btn_clicked();
    void on_reset_btn_clicked();
    void onRadioGroupClicked(int btnID, bool checked); //check acuto or manual
    void onActionButtonGroupClicked(int btnID);  //clicked on action buttons

    void onWifiChanged(const QString &userName, const QString &password);

protected:
    virtual void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;

private:
    Ui::VideoArea *ui;

    PaintLabel *mark_label;
    TestLabel *test_label;

    int camera_with;
    int camera_height;
    double m_centerAreaRatio;
    double m_rorationRange;
    QRect m_original_rect;

    QList<QPushButton*> btnList;
    QButtonGroup *action_btn_group;
    QButtonGroup *radio_btn_group;

    PortSetupDialog *portSetupDialog;
    ScanIpDiaog *scanIpDialog;
    ServerWifiSettings *serverWifiDialog;

    QString m_server_ip;

    QTcpSocket *m_long_socket;
    QDataStream in;

    QUdpSocket *udpSocket;
    QImage m_load_image;

    QTimer *m_timer;
    int m_frame_count;     

    void readConfigFile();
    void saveConfigFile();
    void recoverStatus();
};

#endif // VIDEOAREA_H
