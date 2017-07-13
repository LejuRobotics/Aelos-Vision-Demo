/**
 * @file       ScanIpThread.h
 * @version    1.0
 * @date       2017年07月08日
 * @author     C_Are
 * @copyright  Leju
 *
 * @brief      扫描ip的线程类，ScanIpThread类的h文件
 * @details    扫描局域网内连接的机器人
 */

#ifndef SCANIPTHREAD_H
#define SCANIPTHREAD_H

#include "precompiled.h"

/**
 * @class     ScanIpThread
 * @brief     在线程中扫描局域网内2-254之间的ip
 */

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
    int m_port;                    /**< tcp端口 */
    int m_wait_connected_time;     /**< 连接超时的时长 */
    int m_wait_read_time;          /**< 等待读取超时的时长 */
    QStringList m_original_list;
    QStringList m_addr_list;
    QMutex m_mutex;
    bool m_bStopped;
};

#endif // SCANIPTHREAD_H
