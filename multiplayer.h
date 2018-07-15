#ifndef MULTIPLAYER_H
#define MULTIPLAYER_H

#include <QObject>
#include <QTcpSocket>

enum _serverState {
    SERVERSTATE_DISCONNECTED = 0,
    SERVERSTATE_CONNECTING = 1,
    SERVERSTATE_CONNECTED = 2,
};

class Multiplayer : public QObject
{
    Q_OBJECT
private:
    QTcpSocket *socket;

public:
    explicit Multiplayer(QObject *parent = nullptr);
    int serverState = 0;

private slots:
    void on_recvData();
    void on_disconnect();

signals:

public slots:
};

#endif // MULTIPLAYER_H
