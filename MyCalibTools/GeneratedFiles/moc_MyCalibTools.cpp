/****************************************************************************
** Meta object code from reading C++ file 'MyCalibTools.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../MyCalibTools.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MyCalibTools.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MyCalibTools_t {
    QByteArrayData data[7];
    char stringdata0[155];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MyCalibTools_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MyCalibTools_t qt_meta_stringdata_MyCalibTools = {
    {
QT_MOC_LITERAL(0, 0, 12), // "MyCalibTools"
QT_MOC_LITERAL(1, 13, 23), // "on_actionOpen_triggered"
QT_MOC_LITERAL(2, 37, 0), // ""
QT_MOC_LITERAL(3, 38, 27), // "on_actionOpenList_triggered"
QT_MOC_LITERAL(4, 66, 30), // "on_actionCalibration_triggered"
QT_MOC_LITERAL(5, 97, 34), // "on_actionStereoCalibrate_trig..."
QT_MOC_LITERAL(6, 132, 22) // "on_actionXML_triggered"

    },
    "MyCalibTools\0on_actionOpen_triggered\0"
    "\0on_actionOpenList_triggered\0"
    "on_actionCalibration_triggered\0"
    "on_actionStereoCalibrate_triggered\0"
    "on_actionXML_triggered"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MyCalibTools[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   39,    2, 0x08 /* Private */,
       3,    0,   40,    2, 0x08 /* Private */,
       4,    0,   41,    2, 0x08 /* Private */,
       5,    0,   42,    2, 0x08 /* Private */,
       6,    0,   43,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void MyCalibTools::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        MyCalibTools *_t = static_cast<MyCalibTools *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_actionOpen_triggered(); break;
        case 1: _t->on_actionOpenList_triggered(); break;
        case 2: _t->on_actionCalibration_triggered(); break;
        case 3: _t->on_actionStereoCalibrate_triggered(); break;
        case 4: _t->on_actionXML_triggered(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject MyCalibTools::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_MyCalibTools.data,
      qt_meta_data_MyCalibTools,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *MyCalibTools::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MyCalibTools::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_MyCalibTools.stringdata0))
        return static_cast<void*>(const_cast< MyCalibTools*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int MyCalibTools::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
