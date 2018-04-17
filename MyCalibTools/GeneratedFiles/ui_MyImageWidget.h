/********************************************************************************
** Form generated from reading UI file 'MyImageWidget.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MYIMAGEWIDGET_H
#define UI_MYIMAGEWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MyImageWidgetClass
{
public:
    QGridLayout *gridLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *labelInformation;
    QLabel *labelStatus;
    QLabel *labelImage;

    void setupUi(QWidget *MyImageWidgetClass)
    {
        if (MyImageWidgetClass->objectName().isEmpty())
            MyImageWidgetClass->setObjectName(QStringLiteral("MyImageWidgetClass"));
        MyImageWidgetClass->resize(877, 657);
        MyImageWidgetClass->setMouseTracking(false);
        gridLayout = new QGridLayout(MyImageWidgetClass);
        gridLayout->setSpacing(1);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        gridLayout->setContentsMargins(1, 1, 1, 1);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(1);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        labelInformation = new QLabel(MyImageWidgetClass);
        labelInformation->setObjectName(QStringLiteral("labelInformation"));
        labelInformation->setStyleSheet(QStringLiteral("font: 75 10pt \"Source Code Pro\";"));
        labelInformation->setFrameShape(QFrame::StyledPanel);
        labelInformation->setTextInteractionFlags(Qt::LinksAccessibleByMouse);

        horizontalLayout->addWidget(labelInformation);

        labelStatus = new QLabel(MyImageWidgetClass);
        labelStatus->setObjectName(QStringLiteral("labelStatus"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(labelStatus->sizePolicy().hasHeightForWidth());
        labelStatus->setSizePolicy(sizePolicy);
        labelStatus->setStyleSheet(QStringLiteral("font: 75 10pt \"Source Code Pro\";"));
        labelStatus->setFrameShape(QFrame::StyledPanel);
        labelStatus->setFrameShadow(QFrame::Sunken);

        horizontalLayout->addWidget(labelStatus);


        gridLayout->addLayout(horizontalLayout, 0, 0, 1, 1);

        labelImage = new QLabel(MyImageWidgetClass);
        labelImage->setObjectName(QStringLiteral("labelImage"));
        labelImage->setCursor(QCursor(Qt::CrossCursor));
        labelImage->setMouseTracking(true);
        labelImage->setContextMenuPolicy(Qt::DefaultContextMenu);
        labelImage->setScaledContents(true);
        labelImage->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(labelImage, 1, 0, 1, 1);

        gridLayout->setRowStretch(1, 1);

        retranslateUi(MyImageWidgetClass);

        QMetaObject::connectSlotsByName(MyImageWidgetClass);
    } // setupUi

    void retranslateUi(QWidget *MyImageWidgetClass)
    {
        MyImageWidgetClass->setWindowTitle(QApplication::translate("MyImageWidgetClass", "MyImageWidget", Q_NULLPTR));
        labelInformation->setText(QString());
        labelStatus->setText(QString());
        labelImage->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class MyImageWidgetClass: public Ui_MyImageWidgetClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MYIMAGEWIDGET_H
