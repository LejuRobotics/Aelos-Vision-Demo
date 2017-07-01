#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QSettings>
#include "SerialPort.h"
#include "VideoControl.h"
#include "global_var.h"
#include <QTimer>

class server : public QObject
{
    Q_OBJECT
public:
    server();
    ~server();

    void startListen(); //开始监听
    bool parseData(const QString &msg); //解析数据
    void WriteMsg(const QByteArray &msg);

private slots:
    void onNewConnection();
    void onSocketRead();
    void disPlayError(QAbstractSocket::SocketError);
    void onMarkChanged(bool flag);
    void onDirectionChanged(int val);
    void onActionFinished();
    void onTimeout();
    void onSendInfo(const QString &msg);

private:
    QTcpServer *tcpServer; //TCP服务器
    QTcpSocket *tcpSocket; //soket套接字
    SerialPort *serialPort; //操控串口类
    VideoControl *videoControl; //读取摄像头的类
    bool isOPenSerial;

    void readConfigFile();

    QTimer *m_timer;
};

#endif // SERVER_H
