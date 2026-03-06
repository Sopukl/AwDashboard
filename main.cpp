#include "MainWindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setOrganizationName("AwDashboard");
    QApplication::setApplicationName("AwDashboard");
    MainWindow w;
    w.show();
    return a.exec();
}
