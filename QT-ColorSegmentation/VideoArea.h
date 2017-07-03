#ifndef VIDEOAREA_H
#define VIDEOAREA_H

#include <QMainWindow>
#include <QtNetwork>
#include <QTimer>
#include <QTime>
#include <QStatusBar>
#include <QButtonGroup>
#include "painterlabel.h"
#include "portdialog.h"

namespace Ui {
class VideoArea;
}

class VideoArea : public QMainWindow
{
    Q_OBJECT

public:
    explicit VideoArea(QWidget *parent = 0);
    ~VideoArea();

private slots:    
    void on_connect_btn_clicked(); //点击连接按钮执行函数

    void onLongSocketReadyRead();
    void onSocketDisconnect();

    void onUdpSocketReadyRead();
    void onTimeout();

    void selectFinished(const QRect &_rect);

    void onActioPortClicked();    //菜单栏
    void onActionAreaViewClicked(bool flag);

    void on_record_btn_clicked();
    void on_reset_btn_clicked();
    void onRadioGroupClicked(int btnID, bool checked);
    void onActionButtonGroupClicked(int btnID);  //动作按钮

private:
    void readConfigFile();
    void saveConfigFile();
    void recoverStatus();

private:
    Ui::VideoArea *ui;

    int camera_with;
    int camera_height;
    double m_centerAreaRatio;
    double m_rorationRange;
    QRect m_original_rect;

    QList<QPushButton*> btnList;
    QButtonGroup *action_btn_group;
    QButtonGroup *radio_btn_group;

    portDialog *dialog;

    QTcpSocket *m_long_socket;
    QDataStream in;

    QUdpSocket *udpSocket;
    QImage m_load_image;

    QTimer *m_timer;
    int m_frame_count;     
};

#endif // VIDEOAREA_H
