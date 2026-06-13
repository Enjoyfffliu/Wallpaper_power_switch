#pragma once
#include <QObject>
class ConflictResolver : public QObject { Q_OBJECT public: explicit ConflictResolver(QObject *p=nullptr) : QObject(p) {} };
