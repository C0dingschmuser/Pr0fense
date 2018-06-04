#include "account.h"

Account::Account(QObject *parent) : QObject(parent)
{
}

QString Account::generatePwHash(QString string)
{
    return QString((QCryptographicHash::hash(string.toUtf8(), QCryptographicHash::Sha256).toHex()));
}

void Account::checkAccount(bool &statusConn)
{
    bool connOk = false;
    QTcpSocket *tmpSocket = new QTcpSocket();
    int ok = accountState;
    tmpSocket->connectToHost(ip,38910);
    tmpSocket->waitForConnected();
    if(tmpSocket->state() == QTcpSocket::ConnectedState) {
        QString msg = ".3#" + username + "#~";
        tmpSocket->write(msg.toUtf8());
        tmpSocket->waitForBytesWritten();
        tmpSocket->waitForReadyRead();
        QString response = tmpSocket->readAll();
        if(response.size()) {
            if(response.at(0) == "." && response.contains("~")) {
                connOk = true;
                response.remove(".");
                response.remove("#~");
                ok = response.toInt();
                accountState = ok;
            }
        }
    }
    tmpSocket->close();
    tmpSocket->deleteLater();
    statusConn = connOk;
}

int Account::checkName(QString name)
{
    int ok = -1;
    QTcpSocket *socket = new QTcpSocket();
    socket->connectToHost(ip,38910);
    socket->waitForConnected(5000);
    if(socket->state() == QTcpSocket::ConnectedState) {
        QString msg = ".1#"+name+"#~";
        socket->write(msg.toUtf8());
        socket->waitForBytesWritten();
        socket->waitForReadyRead();
        QString buffer;
        bool ok2 = false;
        int count = 0;
        while(!ok2) {
            count++;
            QThread::msleep(250);
            buffer = socket->readAll();
            if(buffer.size()) {
                if(buffer.at(buffer.size()-1) == "~") ok2 = true;
            }
            if(count > 60) {
                socket->close();
                socket->deleteLater();
                return -1;
            }
        }
        QString response = buffer;
        if(response.size()) {
            if(response.at(0) == "." && response.contains("~")) {
                response.remove(".");
                response.remove("#~");
                ok = response.toInt();
            }
        }
        socket->close();
    }
    socket->deleteLater();
    return ok;
}

int Account::registerAccount(QString name, QString pw)
{
    int ok = -1;
    QTcpSocket *socket = new QTcpSocket();
    socket->connectToHost(ip,38910);
    socket->waitForConnected(5000);
    if(socket->state() == QTcpSocket::ConnectedState) {
        QString msg = ".2#" + name + "#" + generatePwHash(pw) + "#~";
        socket->write(msg.toUtf8());
        socket->waitForBytesWritten();
        socket->waitForReadyRead();
        QString buffer;
        bool ok2 = false;
        int count = 0;
        while(!ok2) {
            count++;
            QThread::msleep(250);
            buffer = socket->readAll();
            if(buffer.size()) {
                if(buffer.at(buffer.size()-1) == "~") ok2 = true;
            }
            if(count > 60) {
                socket->close();
                socket->deleteLater();
                return -1;
            }
        }
        QString response = buffer;
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
    socket->deleteLater();
    return ok;
}

int Account::login(QString name, QString pw)
{
    int ok = -1;
    QTcpSocket *socket = new QTcpSocket();
    socket->connectToHost(ip,38910);
    socket->waitForConnected(5000);
    if(socket->state() == QTcpSocket::ConnectedState) {
        QString msg = ".5#" + name + "#" + generatePwHash(pw) + "#~";
        socket->write(msg.toUtf8());
        socket->waitForBytesWritten();
        socket->waitForReadyRead();
        QString buffer;
        bool ok2 = false;
        int count = 0;
        while(!ok2) {
            count++;
            QThread::msleep(250);
            buffer = socket->readAll();
            if(buffer.size()) {
                if(buffer.at(buffer.size()-1) == "~") ok2 = true;
            }
            if(count > 60) {
                socket->close();
                socket->deleteLater();
                return -1;
            }
        }
        QString response = buffer;
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
    socket->deleteLater();
    return ok;
}

void Account::backupData(QString data)
{
    QTcpSocket *tmpSocket = new QTcpSocket();
    tmpSocket->connectToHost(ip,38910);
    tmpSocket->waitForConnected(5000);
    if(tmpSocket->state() == QTcpSocket::ConnectedState) {       
        QString msg = ".-1#"+username+"#"+generatePwHash(password)+"#"+data+"#~";
        tmpSocket->write(msg.toUtf8());
        tmpSocket->waitForBytesWritten();
        tmpSocket->close();
    }
    tmpSocket->deleteLater();
}

void Account::registerPurchase(QString receipt, QString purchase)
{
    QTcpSocket *tmpSocket = new QTcpSocket();
    tmpSocket->connectToHost(ip,38910);
    tmpSocket->waitForConnected(5000);
    if(tmpSocket->state() == QTcpSocket::ConnectedState) {
        QString msg = ".4#" + username + "#" + receipt + "#" + purchase + "#~";
        tmpSocket->write(msg.toUtf8());
        tmpSocket->waitForBytesWritten();
        tmpSocket->close();
    }
    tmpSocket->deleteLater();
}
