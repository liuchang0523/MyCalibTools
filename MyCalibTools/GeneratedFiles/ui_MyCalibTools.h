/********************************************************************************
** Form generated from reading UI file 'MyCalibTools.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MYCALIBTOOLS_H
#define UI_MYCALIBTOOLS_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>
#include "myimagewidget.h"

QT_BEGIN_NAMESPACE

class Ui_MyCalibToolsClass
{
public:
    QAction *actionOpen;
    QAction *actionOpenList;
    QAction *actionCalibration;
    QAction *actionStereoCalibrate;
    QAction *actionXML;
    QAction *actionMatching;
    QAction *actionLoadAndMatching;
    QWidget *centralWidget;
    QGridLayout *gridLayout;
    MyImageWidget *widget;
    QToolBar *mainToolBar;

    void setupUi(QMainWindow *MyCalibToolsClass)
    {
        if (MyCalibToolsClass->objectName().isEmpty())
            MyCalibToolsClass->setObjectName(QStringLiteral("MyCalibToolsClass"));
        MyCalibToolsClass->resize(1280, 1024);
        actionOpen = new QAction(MyCalibToolsClass);
        actionOpen->setObjectName(QStringLiteral("actionOpen"));
        QIcon icon;
        icon.addFile(QStringLiteral(":/MyCalibTools/resource/Hopstarter-Scrap-Folder-Open.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionOpen->setIcon(icon);
        actionOpenList = new QAction(MyCalibToolsClass);
        actionOpenList->setObjectName(QStringLiteral("actionOpenList"));
        QIcon icon1;
        icon1.addFile(QStringLiteral(":/MyCalibTools/resource/Hopstarter-Scrap-My-Documents.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionOpenList->setIcon(icon1);
        actionCalibration = new QAction(MyCalibToolsClass);
        actionCalibration->setObjectName(QStringLiteral("actionCalibration"));
        QIcon icon2;
        icon2.addFile(QStringLiteral(":/MyCalibTools/resource/letter uppercase C.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionCalibration->setIcon(icon2);
        actionStereoCalibrate = new QAction(MyCalibToolsClass);
        actionStereoCalibrate->setObjectName(QStringLiteral("actionStereoCalibrate"));
        QIcon icon3;
        icon3.addFile(QStringLiteral(":/MyCalibTools/resource/letter uppercase S.ico"), QSize(), QIcon::Normal, QIcon::Off);
        actionStereoCalibrate->setIcon(icon3);
        actionXML = new QAction(MyCalibToolsClass);
        actionXML->setObjectName(QStringLiteral("actionXML"));
        QIcon icon4;
        icon4.addFile(QStringLiteral(":/MyCalibTools/resource/xml-icon.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionXML->setIcon(icon4);
        actionMatching = new QAction(MyCalibToolsClass);
        actionMatching->setObjectName(QStringLiteral("actionMatching"));
        actionLoadAndMatching = new QAction(MyCalibToolsClass);
        actionLoadAndMatching->setObjectName(QStringLiteral("actionLoadAndMatching"));
        centralWidget = new QWidget(MyCalibToolsClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        gridLayout = new QGridLayout(centralWidget);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        widget = new MyImageWidget(centralWidget);
        widget->setObjectName(QStringLiteral("widget"));

        gridLayout->addWidget(widget, 0, 0, 1, 1);

        MyCalibToolsClass->setCentralWidget(centralWidget);
        mainToolBar = new QToolBar(MyCalibToolsClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        mainToolBar->setIconSize(QSize(48, 48));
        MyCalibToolsClass->addToolBar(Qt::TopToolBarArea, mainToolBar);

        mainToolBar->addAction(actionOpen);
        mainToolBar->addAction(actionOpenList);
        mainToolBar->addAction(actionCalibration);
        mainToolBar->addAction(actionStereoCalibrate);
        mainToolBar->addAction(actionXML);

        retranslateUi(MyCalibToolsClass);

        QMetaObject::connectSlotsByName(MyCalibToolsClass);
    } // setupUi

    void retranslateUi(QMainWindow *MyCalibToolsClass)
    {
        MyCalibToolsClass->setWindowTitle(QApplication::translate("MyCalibToolsClass", "MyCalibTools", Q_NULLPTR));
        actionOpen->setText(QApplication::translate("MyCalibToolsClass", "Open", Q_NULLPTR));
        actionOpenList->setText(QApplication::translate("MyCalibToolsClass", "OpenList", Q_NULLPTR));
        actionCalibration->setText(QApplication::translate("MyCalibToolsClass", "Calibration", Q_NULLPTR));
        actionStereoCalibrate->setText(QApplication::translate("MyCalibToolsClass", "StereoCalibrate", Q_NULLPTR));
        actionXML->setText(QApplication::translate("MyCalibToolsClass", "XML", Q_NULLPTR));
        actionMatching->setText(QApplication::translate("MyCalibToolsClass", "Matching", Q_NULLPTR));
        actionLoadAndMatching->setText(QApplication::translate("MyCalibToolsClass", "LoadAndMatching", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class MyCalibToolsClass: public Ui_MyCalibToolsClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MYCALIBTOOLS_H
