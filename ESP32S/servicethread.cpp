#include "servicethread.h"

using namespace std;



ServiceThread::ServiceThread(qintptr ID, QObject *parent) : QThread(parent)
{
    this->socketDescriptor = ID;
}
void ServiceThread::run()
{
    socket = new QTcpSocket();
    /*socket = new QSslSocket();
    socket->setLocalCertificate("certs/espserver.pem");
    socket->setPrivateKey("certs/espserverkey.pem");
    socket->setProtocol(QSsl::SslV3);
    qDebug() << QSslSocket::supportsSsl();
    qDebug() << QSslSocket::sslLibraryBuildVersionString();
    qDebug() << QSslSocket::sslLibraryVersionString();
    */
    if(!socket->setSocketDescriptor(this->socketDescriptor))
    {
        // something's wrong, we just emit a signal
        emit error(socket->error());
        return;
    }
    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()), Qt::DirectConnection);
    connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    exec();
}

void ServiceThread::readyRead()
{
    QByteArray byteArray = socket->readAll();
    string data = QString(byteArray).toUtf8().constData();
    QList<Packet> listPackets;
    QList<string> hashes;
    vector<string> packets{split(data,'\n')};
    for(string s:packets)
    {
        vector<string> packet{split(s,'\t')};
        try
        {
            int esp32id = stoi(packet[0]);
            string ts = packet[1];
            uint timestamp = stoul(ts);
            int rssi = stoi(packet[2]);
            string mac = packet[3];
            string gethash = mac;
            string ssid = packet[4];
            string hash = generateHash(gethash.append(ts));
            int x = esp32devices.find(esp32id).value().getX();
            int y = esp32devices.find(esp32id).value().getY();
            Packet p(esp32id,x,y,timestamp,rssi,mac,ssid,hash);
            if(!hashes.contains(hash))
            {
                listPackets.push_back(p);
                hashes.push_back(hash);
            }
            qDebug() << QString::fromStdString("readyRead() "+ p.toString());

        } catch (...)
        {
            qDebug() << "Exception....";
        }
    }
    emit sendData(listPackets);
}

void ServiceThread::disconnected()
{
    //qDebug() << socketDescriptor << "ServiceThread::disconnected()";
    emit threadExit();
    socket->deleteLater();
    exit(0);
}
