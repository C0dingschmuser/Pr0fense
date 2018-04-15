#-------------------------------------------------
#
# Project created by QtCreator 2018-02-04T20:35:06
#
#-------------------------------------------------

QT       += core gui purchasing network
android: QT += androidextras

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Pr0fense
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        frmmain.cpp \
    tower.cpp \
    enemy.cpp \
    projectile.cpp \
    tile.cpp \
    objectpool.cpp \
    level.cpp


android: SOURCES += lockhelper.cpp

HEADERS += \
        frmmain.h \
    tower.h \
    enemy.h \
    projectile.h \
    tile.h \
    engine.h \
    objectpool.h \
    level.h


android: HEADERS += lockhelper.h

FORMS += \
        frmmain.ui

CONFIG += mobility c++14
MOBILITY =

DISTFILES += \
    android/AndroidManifest.xml \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradlew \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew.bat

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

RESOURCES += \
    resources.qrc

