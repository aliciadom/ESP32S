#ifndef SERVER_H
#define SERVER_H
#include <QTcpServer>
#include <QMainWindow>
#include "globals.h"


class Server : public QTcpServer
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = nullptr);
    void startServer();
signals:
    void sendData(QListPacket packets);
    void doUpdate();
public slots:
    void stopRunning();
    void receiveData(QListPacket packets);
    void threadJoined();
protected:
    void incomingConnection(qintptr socketDescriptor);
private:
    mutex m;
    int nthreadjoined = 0;
};


#endif // SERVER_H
