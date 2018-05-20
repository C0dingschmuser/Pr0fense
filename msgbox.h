#ifndef MSGBOX_H
#define MSGBOX_H
#include <QMessageBox>

class MsgBox
{
public:
    MsgBox();
    static void createMsgBox(QMessageBox &box, QString text, QString btn1Text = "Ja", QString btn2Text = "Nein", QString btn3Text = "");
};

#endif // MSGBOX_H
