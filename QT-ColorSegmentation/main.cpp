#include "VideoArea.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    VideoArea w;
    w.setFixedSize(978,543);
    w.show();

    return a.exec();
}
