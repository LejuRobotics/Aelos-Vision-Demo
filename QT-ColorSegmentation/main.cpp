#include "VideoArea.h"
#include <QApplication>
#include <QDesktopWidget>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    VideoArea w;
    w.show();

    return a.exec();
}
