#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QNetworkProxyFactory>
#include <QNetworkInterface>
#include <QSettings>
#include <QFile>
#include <QProcess>
#include "SerialPort.h"
#include "VideoControl.h"
#include "global_var.h"

class server : public QObject
{
    Q_OBJECT
public:
    server();
    ~server();

    void startListen(); //开始监听
    bool parseData(const QString &msg); //解析数据
    void WriteMsg(const QByteArray &msg); //通过tcp发送信息
    void WriteSerial(int val);  //写入串口

    void pingNetwork();
    QString getLocalIP4Address() const;

private slots:
    void onNewConnection();
    void onSocketRead();
    void disPlayError(QAbstractSocket::SocketError);
    void onDirectionChanged(int val);
    void onActionFinished();
    void onTimeout();
    void onSendInfo(const QString &msg);

    void onNetTimeout();

private:
    QString m_client_ip;
    bool isOPenSerial;
    int m_connection_count;
    QString m_result_msg;

    QTcpServer *tcpServer; //TCP服务器
    QTcpSocket *tcpSocket; //soket套接字
    SerialPort *serialPort; //操控串口类
    VideoControl *videoControl; //读取摄像头的类
    QTimer *m_timer;
    QTimer *m_netTimer;

    void readConfigFile();  //读取配置文件
    void modifyNetworkFile(const QString &id, const QString &password);


};

#endif // SERVER_H
