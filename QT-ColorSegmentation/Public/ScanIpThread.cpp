#include "ScanIpThread.h"

ScanIpThread::ScanIpThread(QObject *parent) : QThread(parent)
{
    m_port = 0;
    m_wait_connected_time = 500;
    m_wait_read_time = 3000;
    m_bStopped = false;
}

ScanIpThread::~ScanIpThread()
{
    m_bStopped = true;
    this->quit();
    if (!this->wait(1000))
        this->terminate();
}

void ScanIpThread::setPort(int port)
{
    m_port = port;
}

void ScanIpThread::setScanRange(int third, int min, int max)
{
    m_original_list.clear();
    for (int i=min; i<=max; i++)
    {
        QString addr = QString("192.168.%1.%2").arg(third).arg(i);
        m_original_list.append(addr);
    }
}

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

void ScanIpThread::stop()
{
    m_bStopped = true;
}

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