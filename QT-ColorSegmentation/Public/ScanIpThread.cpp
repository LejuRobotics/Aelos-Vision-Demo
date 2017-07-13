/**
 * @file       ScanIpThread.cpp
 * @version    1.0
 * @date       2017年07月08日
 * @author     C_Are
 * @copyright  Leju
 *
 * @brief      扫描ip的线程类，ScanIpThread类的cpp文件
 * @details    扫描局域网内连接的机器人
 */

#include "ScanIpThread.h"

/**
 * @brief     ScanIpThread类的构造函数
 * @param     parent 父对象
 */

ScanIpThread::ScanIpThread(QObject *parent) : QThread(parent)
{
    m_port = 0;
    m_wait_connected_time = 500;
    m_wait_read_time = 5000;
    m_bStopped = false;
}

/**
 * @brief     ScanIpThread类的析构函数
 * @details   关闭并销毁线程
 */

ScanIpThread::~ScanIpThread()
{
    m_bStopped = true;
    this->quit();
    if (!this->wait(1000))
        this->terminate();
}

/**
 * @brief     设置tcp端口
 * @param     port tcp端口号
 */

void ScanIpThread::setPort(int port)
{
    m_port = port;
}

/**
 * @brief     设置扫描的ip范围
 * @param     third 局域网段
 * @param     min 起始位置
 * @param     max 终止位置
 */

void ScanIpThread::setScanRange(int third, int min, int max)
{
    m_original_list.clear();
    for (int i=min; i<=max; i++)
    {
        QString addr = QString("192.168.%1.%2").arg(third).arg(i);
        m_original_list.append(addr);
    }
}

/**
 * @brief     开启线程，扫描m_addr_list
 */

void ScanIpThread::startScan()
{
    m_mutex.lock();
    m_bStopped = true;
    m_addr_list = m_original_list;
    m_mutex.unlock();
    m_bStopped = false;
    if (!this->isRunning())
        this->start();
}

/**
 * @brief     结束扫描
 */

void ScanIpThread::stop()
{
    m_bStopped = true;
}


/**
 * @brief     重写run函数，在线程中扫描局域网ip
 */

void ScanIpThread::run()
{
    QTcpSocket socket;
    QDataStream in(&socket);
    in.setVersion(QDataStream::Qt_5_3);
    QByteArray inBlock;
    QString msg, readData;
    QTime t;
    t.start();

    while (!m_addr_list.isEmpty()) {
        if (m_bStopped)
        {
            m_addr_list.clear();
            return;
        }

        QString nextAddr = m_addr_list.first();
        m_addr_list.removeFirst();
        socket.abort();
        socket.connectToHost(nextAddr, m_port);
        if (!socket.waitForConnected(m_wait_connected_time))
        {
            msg = QString("connect to %1 timeout").arg(nextAddr);
            emit connectFailed(msg);
            continue;
        }

        if (!socket.waitForReadyRead(m_wait_read_time))
        {
            msg = QString("%1 wait fot read timeout").arg(nextAddr);
            emit connectFailed(msg);
            continue;
        }

        in >> inBlock;
        readData = QString::fromUtf8(inBlock);
        if (readData.startsWith("Connect to server successful"))
        {
            emit connectSuccessed(nextAddr);
        }
        else
        {
            msg = QString("%1 already has a connection").arg(nextAddr);
            emit connectFailed(msg);
        }
        inBlock.resize(0);
    }
    emit scanFinished(t.elapsed()/1000);
}
