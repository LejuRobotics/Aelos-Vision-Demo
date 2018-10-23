#include "VideoArea.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

#if 1  //目前发现在qt5.9.1上运行release版，需要打印两行才能运行程序，具体原因暂未查明
    qDebug()<< "this is robot vision client";
    qDebug()<<"version： 1.0";
#endif

    VideoArea w;
    w.show();
    w.setFixedSize(w.size());

    return a.exec();
}
