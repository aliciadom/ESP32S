#include "mainwindow.h"
#include <QApplication>
#include <QtDebug>
#include <QtCore>

#include <QtGui>
#include <QString>
#include <fstream>
#include <string.h>
#include "globals.h"

using namespace std;


void configureESP32Positon()
{
    ifstream infile;
    QString line;
    int x_coord;
    int y_coord;

    QFile file("config.txt");
    if(file.exists())
    {
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
           QTextStream in(&file);
           line = in.readLine();
           nesp32 = stoi(line.toUtf8().constData());
           for(int i = 0; i<nesp32; i++)
           {
              line = in.readLine();
              QStringList list = line.split(QRegExp("\\s"));
              x_coord = stoi(list.first().toUtf8().constData());
              y_coord = stoi(list.last().toUtf8().constData());
              qDebug() << x_coord;
              qDebug() << y_coord;
              ESP32 esp32(0,"ESP"+to_string(i),x_coord,y_coord);
              esp32devices.insert(i,esp32);
              if(x_coord > x_max) x_max = x_coord;
              if(y_coord > y_max) y_max = y_coord;
              if(x_coord < x_min) x_min = x_coord;
              if(y_coord < y_min) y_min = y_coord;
           }
           line = in.readLine();
           accuracy = stoi(line.toUtf8().constData());
           qDebug() << accuracy;
           file.close();
        }
        else
        {
            qInfo("File not open");
        }
    }
    else
    {
        qInfo("File does not exist");
    }

}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    configureESP32Positon();

    qRegisterMetaType<QListPacket>("QListPacket");
    MainWindow w;
    w.show();

    return a.exec();
}





