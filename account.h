#ifndef ACCOUNT_H
#define ACCOUNT_H
#include <QObject>
#include <QString>
#include <QCryptographicHash>
#include <QTcpSocket>
#include <QThread>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>

class Account : public QObject
{
    Q_OBJECT
public:
    explicit Account(QObject *parent = nullptr);
    QString username;
    QString password;
    QString ip;
    int accountState = 0;
    int connTries = 0;
    QString generatePwHash(QString string);
    void checkAccount(bool &statusConn);
    int checkName(QString name);
    int registerAccount(QString name, QString pw);
    int login(QString name, QString pw);
    void backupData(QString data);
    void registerPurchase(QString receipt, QString purchase);
};

#endif // ACCOUNT_H
