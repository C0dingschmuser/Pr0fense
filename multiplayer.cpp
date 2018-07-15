#include "multiplayer.h"

Multiplayer::Multiplayer(QObject *parent) : QObject(parent)
{
    socket = new QTcpSocket();
    connect(socket, SIGNAL(readyRead()), this, SLOT(on_recvData()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(on_disconnect()));
}

void Multiplayer::on_recvData()
{

}

void Multiplayer::on_disconnect()
{

}
