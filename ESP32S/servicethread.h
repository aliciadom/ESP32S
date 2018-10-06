#ifndef SERVICETHREAD_H
#define SERVICETHREAD_H
#include <QThread>
#include <QTcpSocket>
//#include <QSslSocket>
#include <QDebug>
#include "globals.h"


class ServiceThread : public QThread
{
    Q_OBJECT
public:
    explicit ServiceThread(qintptr ID, QObject *parent = nullptr);
    void run();

signals:
    void error(QTcpSocket::SocketError socketerror);
    void sendData(QListPacket packets);
    void threadExit();
public slots:
    void readyRead();
    void disconnected();
private:
    QTcpSocket *socket;
    //QSslSocket *socket;
    qintptr socketDescriptor;
};

#endif // SERVICETHREAD_H
