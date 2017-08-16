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
    void WriteData(const QByteArray &msg);

    void on_connect_btn_clicked();
    void onStartConnectTo(const QString &ip);

    void onLongSocketReadyRead();
    void onSocketDisconnect();

    void onUdpSocketReadyRead();
    void onTimeout();

    void selectFinished(const QRect &_rect);

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

    void onUdpPortChanged();
    void onWifiChanged(const QString &userName, const QString &password);

    void onSliderTimeout();

    //选择图像格式
    void on_rgb_radio_toggled(bool checked);
    void on_yuv_radio_toggled(bool checked);
    void on_hsv_radio_toggled(bool checked);

    //亮度，对比度，Y值, HSV值滚动条
    void on_colorY_slider_valueChanged(int value);
    void on_brightness_slider_valueChanged(int value);
    void on_contrast_slider_valueChanged(int value);
    void on_colorMinH_slider_valueChanged(int value);
    void on_colorMinS_slider_valueChanged(int value);
    void on_colorMinV_slider_valueChanged(int value);
    void on_colorMaxH_slider_valueChanged(int value);
    void on_colorMaxS_slider_valueChanged(int value);
    void on_colorMaxV_slider_valueChanged(int value);

    void addInfomation(const QString &msg);

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

    PortSetupDialog *portSetupDialog;
    ScanIpDiaog *scanIpDialog;
    ServerWifiSettings *serverWifiDialog;
    ConnectionBox *connectionBox;
    ParameterSettingDialog *parameterSettingDialog;

    QString m_server_ip;

    QTcpSocket *m_long_socket;
    bool m_isConnected;

    QUdpSocket *udpSocket;
    QImage m_load_image;

    QTimer *m_timer;
    int m_frame_count;     

    QTimer *m_sliderTimer;
    QString m_command;

    QVector<QRect> m_markRectVec;

    void readConfigFile();
    void saveConfigFile();
    void startSliderTimer();
};

#endif // VIDEOAREA_H
