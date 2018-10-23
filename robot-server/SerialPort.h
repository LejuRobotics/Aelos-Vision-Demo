/**
 * @file       SerialPort.h
 * @version    1.0
 * @date       2017年07月08日
 * @author     C_Are
 * @copyright  Leju
 *
 * @brief      串口操作，SerialPort类的h文件
 */

#ifndef SERIALPORT_H
#define SERIALPORT_H

#include "global_var.h"

#define SCM_MAX_BUFFER_SIZE 65535 /**< 通过串口单次收发数据的最大包长 */

/**
 * @class     SerialPort
 * @brief     通过继承SerialPort类对串口进行读写操作
 */

class SerialPort : public QObject
{
    Q_OBJECT
public:
    explicit SerialPort(QObject *parent = 0);
    ~SerialPort();

    //打开串口
    bool openSerilPort();

    //发送指令
    void sendMsg(char* msg, int len);

    void sendMsg(QString &str);

signals:
    void actionFinished();
    void lowBattery();

private slots:
    void onReadyRead();  //读取串口消息

private:
    QSerialPort *m_serialPort;    /**< QSerialPort类的对象 */
    QSerialPort::BaudRate toBaudRate(int rate);

    void convertStringToHex(const QString &str, QByteArray &byteData);
    char convertCharToHex(char ch);
};

#endif // SERIALPORT_H
