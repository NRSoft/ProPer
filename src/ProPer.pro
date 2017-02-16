#-------------------------------------------------
#
# Project created by QtCreator 2016-09-06T15:38:11
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ProPer
TEMPLATE = app


SOURCES += main.cpp\
    mainwindow.cpp \
    taskdialog.cpp \
    task.cpp \
    remotefile.cpp \
    remoteaccessdialog.cpp \
    mainwindow_xml.cpp \
    hyperlinkdialog.cpp \
    note.cpp \
    taskpipe.cpp \
    taskbubble.cpp \
    taskpipeview.cpp \
    categorydialog.cpp

HEADERS  += mainwindow.h \
    taskdialog.h \
    task.h \
    remotefile.h \
    remoteaccessdialog.h \
    version.h \
    hyperlinkdialog.h \
    note.h \
    taskpipe.h \
    taskbubble.h \
    taskpipeview.h \
    categorydialog.h \
    projectheaderview.h

FORMS    += mainwindow.ui \
    taskdialog.ui \
    remoteaccessdialog.ui \
    hyperlinkdialog.ui \
    categorydialog.ui

DISTFILES += \
    ../doc/questions.txt

RESOURCES += \
    mainwindow.qrc

INCLUDEPATH += ../ext
