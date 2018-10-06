#include "server.h"
#include "servicethread.h"
#include "globals.h"

Server::Server(QObject *parent) : QTcpServer (parent)
{
}

void Server::startServer()
{

    quint16 port = serverport;

    if(!this->listen(QHostAddress::Any, port))
    {
        qDebug() << "server::startServer failed ";
    }
    else
    {
        qDebug() << "server::startServer succeed " << port << "...";
    }
}

void Server::incomingConnection(qintptr socketDescriptor)
{
    qDebug() << socketDescriptor << "server::incomingConnection called";
    ServiceThread *thread = new ServiceThread(socketDescriptor, this);

    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(thread, SIGNAL(sendData(QListPacket)), this, SLOT(receiveData(QListPacket)));
    connect(thread, SIGNAL(threadExit()), this, SLOT(threadJoined()));

    thread->start();
}

void Server::stopRunning()
{
    qDebug() << "Server::stopRunning()";
    this->close();
}
void Server::receiveData(QListPacket packets)
{
    //qDebug() << "Server::receiveData(QListPacket packets)";
    emit sendData(packets);
}
void Server::threadJoined()
{
    lock_guard<mutex> lg(m);
    nthreadjoined++;
    if(nthreadjoined==nesp32)
    {
        nthreadjoined = 0;
        emit doUpdate();
    }
    qDebug() << "Server::threadJoined()";
}
