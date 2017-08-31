#include "VideoArea.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    VideoArea w;
    w.show();
    w.setFixedSize(w.size());

    return a.exec();
}
