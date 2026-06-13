#pragma once
#include <QObject>
class PowerMonitor : public QObject { Q_OBJECT public: explicit PowerMonitor(QObject *p=nullptr) : QObject(p) {} };
