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
//#include "DiscernColor.h"

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

private slots:
    void onNewConnection();
    void onSocketRead();
    void disPlayError(QAbstractSocket::SocketError);
    void onDirectionChanged(int val);
    void onActionFinished();
    void readyNextAction();
    void onTimeout();
    void onSendInfo(const QString &msg);
    void pingRouter();
    void onStartMoveOn();
    void stopMoveOn();

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

    void readConfigFile();
    void WriteMsg(const QByteArray &msg);
    void WriteSerial(int val);
    void WriteSerial2(const QString &val);
    QString getLocalIP4Address() const;
    bool parseData(const QString &msg);
    void modifyNetworkFile(const QString &id, const QString &password);
};

#endif // SERVER_H
