#include "account.h"

Account::Account()
{
    socket = new QTcpSocket();
}

QString Account::generatePwHash(QString string)
{
    return QString((QCryptographicHash::hash(string.toUtf8(), QCryptographicHash::Sha256).toHex()));
}

void Account::checkAccount()
{
    QTcpSocket *tmpSocket = new QTcpSocket();
    int ok = accountState;
    tmpSocket->connectToHost(ip,38910);
    tmpSocket->waitForConnected(5000);
    if(tmpSocket->state() == QTcpSocket::ConnectedState) {
        QString msg = ".3#" + username + "#~";
        tmpSocket->write(msg.toUtf8());
        tmpSocket->waitForBytesWritten(5000);
        tmpSocket->waitForReadyRead(5000);
        QString response = tmpSocket->readAll();
        if(response.size()) {
            if(response.at(0) == "." && response.contains("~")) {
                response.remove(".");
                response.remove("#~");
                ok = response.toInt();
                accountState = ok;
            }
        }
    }
    tmpSocket->deleteLater();
}

int Account::checkName(QString name)
{
    int ok = -1;
    socket->connectToHost(ip,38910);
    socket->waitForConnected(5000);
    if(socket->state() == QTcpSocket::ConnectedState) {
        QString msg = ".1#"+name+"#~";
        socket->write(msg.toUtf8());
        socket->waitForBytesWritten(5000);
        socket->waitForReadyRead(5000);
        QString response = socket->readAll();
        if(response.size()) {
            if(response.at(0) == "." && response.contains("~")) {
                response.remove(".");
                response.remove("#~");
                ok = response.toInt();
            }
        }
        socket->close();
    }
    return ok;
}

int Account::registerAccount(QString name, QString pw)
{
    int ok = -1;
    socket->connectToHost(ip,38910);
    socket->waitForConnected(5000);
    if(socket->state() == QTcpSocket::ConnectedState) {
        QString msg = ".2#" + name + "#" + generatePwHash(pw) + "#~";
        socket->write(msg.toUtf8());
        socket->waitForBytesWritten(5000);
        socket->waitForReadyRead(5000);
        QString response = socket->readAll();
        if(response.size()) {
            if(response.at(0) == "." && response.contains("~")) {
                response.remove(".");
                response.remove("#~");
                ok = response.toInt();
                if(ok == 1) {
                    username = name;
                    password = pw;
                    accountState = 1;
                }
            }
        }
        socket->close();
    }
    return ok;
}

int Account::login(QString name, QString pw)
{
    int ok = -1;
    socket->connectToHost(ip,38910);
    socket->waitForConnected(5000);
    if(socket->state() == QTcpSocket::ConnectedState) {
        QString msg = ".5#" + name + "#" + generatePwHash(pw) + "#~";
        socket->write(msg.toUtf8());
        socket->waitForBytesWritten(5000);
        socket->waitForReadyRead(5000);
        QString response = socket->readAll();
        if(response.size()) {
            if(response.at(0) == "." && response.contains("~")) {
                response.remove(".");
                response.remove("#~");
                ok = response.toInt();
                if(ok == 1) {
                    username = name;
                    password = pw;
                    accountState = 1;
                }
            }
        }
        socket->close();
    }
    return ok;
}

void Account::registerPurchase(QString receipt, QString purchase)
{
    QTcpSocket *tmpSocket = new QTcpSocket();
    tmpSocket->connectToHost(ip,38910);
    tmpSocket->waitForConnected(5000);
    if(tmpSocket->state() == QTcpSocket::ConnectedState) {
        QString msg = ".4#" + username + "#" + receipt + "#" + purchase + "#~";
        tmpSocket->write(msg.toUtf8());
        tmpSocket->waitForBytesWritten(5000);
        tmpSocket->close();
    }
    tmpSocket->deleteLater();
}
