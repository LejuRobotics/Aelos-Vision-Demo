#include "server.h"
#include <QCoreApplication>

/**
 * @brief 主函数
 */

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Server s;
    s.startListen();

    return a.exec();
}
