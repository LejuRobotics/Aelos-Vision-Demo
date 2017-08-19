/**
 * @file       server.h
 * @version    1.0
 * @date       2017年07月08日
 * @author     C_Are
 * @copyright  Leju
 *
 * @brief      服务器，Server类的h文件
 * @details    服务器类，集合tcp服务器,图像处理，串口操作等
 */

#ifndef SERVER_H
#define SERVER_H

#include "global_var.h"
#include "SerialPort.h"
#include "VideoControl.h"

/**
 * @class     Server
 * @brief     服务器类,集合tcp服务器,图像处理，串口操作等
 */

class Server : public QObject
{
    Q_OBJECT
public:
    Server();
    ~Server();

    void startListen(); //开始监听

    /**
     * @brief     提示音类型
     */
    enum CueTone {
        StartUp,            /**< 主人，您好 */
        LowBattery,         /**< 电量低，请充电 */
        ConnectSucceeful,   /**< 路由器连接成功，请使用客户端扫描机器人 */
        ConnectFailed,      /**< 路由器连接失败，正在启动为热点模式 */
        ApModeAvailable,    /**< 切换到热点，请连接热点操作 */
        ResartToWifiMode,   /**< 已接收路由器设置，正在重启进入连接路由器模式 */
        Connectting,        /**< 正在连接路由器 */
        RestartToApMode,    /**< 重启进入热点模式 */
    };

    void playAudio(CueTone type);

private slots:
    void onNewConnection();
    void onSocketRead();
    void disPlayError(QAbstractSocket::SocketError);
    void onDirectionChanged(int val);
    void onActionFinished();
    void readyNextAction();
    void onTimeout();
    void onSendInfo(const QString &msg);
    void onStartMoveOn(int msec);
    void stopMoveOn();
    void startExcuteShell();
    void onLowBattery();
    void onReadyPlayLowBattery();

private:
    QString m_client_ip;                  /**< 客户端IP */
    bool isOPenSerial;                    /**< 是否打开串口 */
    int m_connection_count;               /**< tcp客户端连接的个数 */
    QString m_result_msg;

    QTcpServer *tcpServer;                /**< TCP服务器 */
    QTcpSocket *tcpSocket;                /**< TCP socket套接字 */
    SerialPort *serialPort;               /**< 串口操作类VideoControl的对象 */
    VideoControl *videoControl;           /**< 处理摄像头图像的类VideoControl的对象 */

    DiscernColor *discernColor;

    QTimer *m_timer;
    QString m_local_ip;
    QByteArray m_byteIpAndNo;
    QByteArray m_byteMd5;

    QProcess *mplayer;                      /**< 启动mplayer的进程对象 */

    QTimer *m_timer_2;
    bool m_bIsReady;

    bool m_bIsConnectRounter;
    int m_pingCount;
    QString m_shellName;

    void WriteMsg(const QByteArray &msg);
    void WriteSerial(int val);
    void WriteSerial2(const QString &val);
    QString getLocalIP4Address() const;
    bool parseData(const QString &msg);
    void modifyNetworkFile(const QString &id, const QString &password);

    qint32 m_bufferReadSize;
    qint32 m_bufferTotalSize;
};

#endif // SERVER_H
