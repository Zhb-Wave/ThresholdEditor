#include "thresholdeditor.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ThresholdEditor w;
    w.show();
    return a.exec();
}
