#include "msgbox.h"

MsgBox::MsgBox()
{

}

void MsgBox::createMsgBox(QMessageBox &box, QString text, QString btn1Text, QString btn2Text, QString btn3Text)
{
    box.setText(text);
    box.addButton(btn1Text, QMessageBox::NoRole);
    if(btn2Text != "") {
        box.addButton(btn2Text, QMessageBox::NoRole);
    }
    if(btn3Text != "") {
        box.addButton(btn3Text, QMessageBox::NoRole);
    }
}
