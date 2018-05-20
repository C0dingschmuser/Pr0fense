#ifndef ACCOUNT_H
#define ACCOUNT_H
#include <QString>
#include <QCryptographicHash>
#include <QTcpSocket>

class Account
{
private:
    QTcpSocket *socket;
public:
    Account();
    QString username;
    QString password;
    QString ip;
    int accountState = 0;
    QString generatePwHash(QString string);
    void checkAccount();
    int checkName(QString name);
    int registerAccount(QString name, QString pw);
    void registerPurchase(QString receipt, QString purchase);
};

#endif // ACCOUNT_H
