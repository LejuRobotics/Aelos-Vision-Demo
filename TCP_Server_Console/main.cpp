#include "server.h"
#include <QCoreApplication>

const qint64 LOG_FILE_MAX_SIZE = 5*1024*1024;  //日志文件大小的最大值

/**
 * @brief 打印输出保存到日志文件
 */

void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    static QMutex mutex;
    mutex.lock();

    QString text;
    switch(type)
    {
    case QtDebugMsg:
        text = QString("Debug:");
        break;

    case QtWarningMsg:
        text = QString("Warning:");
        break;

    case QtCriticalMsg:
        text = QString("Critical:");
        break;

//    case QtInfoMsg:
//        text = QString("Information:");
//        break;

    case QtFatalMsg:
        text = QString("Fatal:");
    }

    QString context_info = QString("File:(%1) Line:(%2)").arg(QString(context.file)).arg(context.line);
    QString current_date_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd");
    QString current_date = QString("(%1)").arg(current_date_time);
    QString message = QString("%1 %2 %3 %4").arg(text).arg(context_info).arg(msg).arg(current_date);

    QFile file("log.txt");
    file.open(QIODevice::WriteOnly | QIODevice::Append);
    if (file.size() > LOG_FILE_MAX_SIZE)  //日志文件大小超过指定大小，则清除文件内容重新写入
    {
        file.close();
        file.open(QIODevice::WriteOnly);
        file.close();
        file.open(QIODevice::WriteOnly | QIODevice::Append);
    }
    QTextStream text_stream(&file);
    text_stream << message << "\r\n";
    file.flush();
    file.close();

    mutex.unlock();
}


/**
 * @brief 主函数
 */

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    //注册MessageHandler,监听打印输出
    qInstallMessageHandler(outputMessage);

    Server s;
    s.startListen();

    return a.exec();
}
