#include "SerialPort.h"
#include <QDebug>

SerialPort::SerialPort(QObject *parent) : QObject(parent)
{
    m_serialPort = new QSerialPort(this);
}

SerialPort::~SerialPort()
{
    if (m_serialPort->isOpen())
    {
        m_serialPort->close();
    }
    delete m_serialPort;
}

bool SerialPort::openSerilPort()
{
    //如果串口已经打开，先关闭
    if (m_serialPort->isOpen())
    {
        m_serialPort->clear();
        m_serialPort->close();
    }

    //设置串口名称
    m_serialPort->setPortName(g_serial_name);

    if(!m_serialPort->open(QIODevice::ReadWrite)) //尝试打开串口
    {
        qDebug()<<QString("open %1 failed !").arg(g_serial_name);
        return false;
    }

    qDebug()<<QString("open %1 successful !").arg(g_serial_name);
    m_serialPort->setBaudRate(toBaudRate(g_baud_rate),QSerialPort::AllDirections);//设置波特率和读写方向
    m_serialPort->setDataBits(QSerialPort::Data8);		//数据位为8位
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);//无流控制
    m_serialPort->setParity(QSerialPort::NoParity);	//无校验位
    m_serialPort->setStopBits(QSerialPort::OneStop); //一位停止位

    connect(m_serialPort,SIGNAL(readyRead()),this,SLOT(onReadyRead()));
    return true;
}

void SerialPort::sendMsg(const QByteArray &msg)
{
    m_serialPort->write(msg);
}

void SerialPort::sendMsg(char *msg, int len)
{
    for(int i=0; i<len; ++i)
    {
        printf("0x%x\n", msg[i]);
    }
    m_serialPort->write(msg,len);
}

void SerialPort::sendMsg(char *msg)
{
    m_serialPort->write(msg,qstrlen(msg));
}

//机器人动作完成后会返回数据包
void SerialPort::onReadyRead()
{
    QByteArray ba = m_serialPort->readAll();
    QByteArray hexData = ba.toHex();
    qDebug()<< "RET: "<< hexData;

    if (hexData == "1a000000000000001a00000000000000") //这个是动作完成后返回的值
    {
        emit actionFinished(); //发送动作完成信号
    }
}

QSerialPort::BaudRate SerialPort::toBaudRate(int rate)
{
    switch (rate) {
    case 1200:
        return QSerialPort::Baud1200;
    case 2400:
        return QSerialPort::Baud2400;
    case 4800:
        return QSerialPort::Baud4800;
    case 9600:
        return QSerialPort::Baud9600;
    case 19200:
        return QSerialPort::Baud19200;
    case 38400:
        return QSerialPort::Baud38400;
    case 57600:
        return QSerialPort::Baud57600;
    case 115200:
        return QSerialPort::Baud115200;
    default:
        return QSerialPort::UnknownBaud;
    }
}
