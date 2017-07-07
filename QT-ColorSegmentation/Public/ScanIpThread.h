#ifndef SCANIPTHREAD_H
#define SCANIPTHREAD_H

#include "precompiled.h"

class ScanIpThread : public QThread
{
    Q_OBJECT
public:
    explicit ScanIpThread(QObject *parent = 0);
    ~ScanIpThread();

    void setPort(int port);
    void setScanRange(int third, int min, int max);
    void startScan();
    void stop();

signals:
    void connectSuccessed(const QString &);
    void connectFailed(const QString &);
    void scanFinished(int sec);  //The signal that is sent when all the IP address are scanned

protected:
    virtual void run() Q_DECL_OVERRIDE;

private:
    int m_port;
    int m_wait_connected_time;
    int m_wait_read_time;
    QStringList m_original_list;
    QStringList m_addr_list;
    QMutex m_mutex;
    bool m_bStopped;
};

#endif // SCANIPTHREAD_H
