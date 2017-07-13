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

signals:
    void actionFinished();

private slots:
    void onReadyRead();  //读取串口消息

private:
    QSerialPort *m_serialPort;    /**< QSerialPort类的对象 */
    QSerialPort::BaudRate toBaudRate(int rate);
};

#endif // SERIALPORT_H
