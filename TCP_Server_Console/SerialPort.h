#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include "global_var.h"

class SerialPort : public QObject
{
    Q_OBJECT
public:
    explicit SerialPort(QObject *parent = 0);
    ~SerialPort();

    //打开串口
    bool openSerilPort();

    //发送指令
    void sendMsg(const QByteArray &msg);
    void sendMsg(char* msg, int len);
    void sendMsg(char* msg);

signals:
    void actionFinished();

private slots:
    void onReadyRead();  //读取串口消息

private:
    QSerialPort *m_serialPort;
    QSerialPort::BaudRate toBaudRate(int rate);
};

#endif // SERIALPORT_H
