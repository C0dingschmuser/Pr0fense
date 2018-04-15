#pragma once
#include <QAndroidJniObject>

class KeepAwakeHelper
{
public:
    KeepAwakeHelper();
    virtual ~KeepAwakeHelper();

private:
    QAndroidJniObject m_wakeLock;
};
