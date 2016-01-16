
#include <QApplication>
#include <myserver.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MyServer myserver(2323);
    myserver.show();

    return a.exec();
}
