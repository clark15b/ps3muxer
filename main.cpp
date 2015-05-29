#include <QtGui/QApplication>
#include <QTranslator>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString locale=QLocale::system().name();

    QTranslator *translator = new QTranslator(&a);
    if( translator->load( QApplication::applicationDirPath()+QString("/ps3muxer_%1").arg(locale) ) )
        a.installTranslator(translator);
    else
        delete translator;

    QString cmd;
    if(QApplication::arguments().size()>1)
        cmd=QApplication::arguments().last();


    MainWindow w(0,cmd);


    w.show();
    return a.exec();
}
