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
    QWidget *centralWidget;
    QGridLayout *gridLayout;
    MyImageWidget *widget;
    QToolBar *mainToolBar;

    void setupUi(QMainWindow *MyCalibToolsClass)
    {
        if (MyCalibToolsClass->objectName().isEmpty())
            MyCalibToolsClass->setObjectName(QStringLiteral("MyCalibToolsClass"));
        MyCalibToolsClass->resize(1304, 1072);
        actionOpen = new QAction(MyCalibToolsClass);
        actionOpen->setObjectName(QStringLiteral("actionOpen"));
        centralWidget = new QWidget(MyCalibToolsClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        gridLayout = new QGridLayout(centralWidget);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        widget = new MyImageWidget(centralWidget);
        widget->setObjectName(QStringLiteral("widget"));

        gridLayout->addWidget(widget, 0, 0, 1, 1);

        MyCalibToolsClass->setCentralWidget(centralWidget);
        mainToolBar = new QToolBar(MyCalibToolsClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        MyCalibToolsClass->addToolBar(Qt::TopToolBarArea, mainToolBar);

        mainToolBar->addAction(actionOpen);

        retranslateUi(MyCalibToolsClass);

        QMetaObject::connectSlotsByName(MyCalibToolsClass);
    } // setupUi

    void retranslateUi(QMainWindow *MyCalibToolsClass)
    {
        MyCalibToolsClass->setWindowTitle(QApplication::translate("MyCalibToolsClass", "MyCalibTools", Q_NULLPTR));
        actionOpen->setText(QApplication::translate("MyCalibToolsClass", "Open", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class MyCalibToolsClass: public Ui_MyCalibToolsClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MYCALIBTOOLS_H
