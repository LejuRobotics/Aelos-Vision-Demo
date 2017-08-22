/**
 * @file       VideoArea.h
 * @version    1.0
 * @date       2017年07月08日
 * @author     C_Are
 * @copyright  Leju
 *
 * @brief      主窗口，VideoArea类的h文件
 * @details    VideoArea类是主窗口类，可以接收到的摄像头图像，以及与服务端进行tcp通信
 */

#ifndef VIDEOAREA_H
#define VIDEOAREA_H

#include "precompiled.h"
#include "painterlabel.h"
#include "PortSetupDialog.h"
#include "ScanIpDiaog.h"
#include "ServerWifiSettings.h"
#include "ConnectionBox.h"
#include "ParameterSettingDialog.h"

/**
 * @class     VideoArea
 * @brief     主窗口
 * @details   通过udp接收摄像头图像以及与服务端进行tcp通信
 */

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
    void WriteData(const QByteArray &msg); //发送TCP消息

    void on_connect_btn_clicked();  //点击"Connect"按钮
    void onStartConnectTo(const QString &ip);

    void onLongSocketReadyRead();   //接收TCP消息
    void onSocketDisconnect();

    void onUdpSocketReadyRead();    //接收UDP消息，包括摄像头图像和ip地址
    void onTimeout();

    void selectFinished(const QRect &_rect);
    void onUdpPortChanged();
    void onWifiChanged(const QString &userName, const QString &password);
    void addInfomation(const QString &msg);

    //点击菜单栏按钮
    void onActioPortClicked();
    void onActionAreaViewClicked(bool flag);
    void onActionServerWifiClicked();
    void onActionParamSettingClicked();

    void on_record_btn_clicked();  //点击Record按钮
    void on_reset_btn_clicked();   //点击Reset按钮
    void on_auto_radio_clicked(bool checked);  //点击auto按钮
    void on_manual_radio_clicked(bool checked); //点击manual按钮
    void onActionButtonGroupClicked(int btnID);  //点击动作按钮，如Quick walk, Quick back等

    void on_original_radio_toggled(bool checked);  //显示原图
    void on_transform_radio_toggled(bool checked); //显示转换后的图
    void on_yuv_radio_toggled(bool checked);  //选中YUV格式
    void on_hsv_radio_toggled(bool checked);  //选中HSV格式
    void on_goback_checkBox_clicked(bool checked);  //设置机器人完成所有目标后返回到第一个目标
    void on_football_checkBox_clicked(bool checked);  //识别足球

    //亮度，对比度，Y值, HSV值滚动条
    void onSliderValueChanged(int value);

    void on_shooot_checkBox_clicked(bool checked);

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
    QString m_mark_rgb;

    QList<QPushButton*> btnList;
    QButtonGroup *action_btn_group;

    QList<QSlider*> qsliderList;

    PortSetupDialog *portSetupDialog;
    ScanIpDiaog *scanIpDialog;
    ServerWifiSettings *serverWifiDialog;
    ConnectionBox *connectionBox;
    ParameterSettingDialog *parameterSettingDialog;

    QString m_server_ip;

    QTcpSocket *m_long_socket;
    bool m_isConnected;

    qint32 m_bufferReadSize;
    qint32 m_bufferTotalSize;

    QUdpSocket *udpSocket;
    QImage m_load_image;

    QTimer *m_timer;
    int m_frame_count;     

    QTimer *m_sliderTimer;
    QString m_command;

    QVector<QRect> m_markRectVec;

    void readConfigFile();
    void saveConfigFile();
};

#endif // VIDEOAREA_H
