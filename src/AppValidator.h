#pragma once
#include <QObject>
class AppValidator : public QObject { Q_OBJECT public: explicit AppValidator(QObject *p=nullptr) : QObject(p) {} };
