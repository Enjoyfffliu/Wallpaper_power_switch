#pragma once
#include <QObject>
class WallpaperManager : public QObject { Q_OBJECT public: explicit WallpaperManager(QObject *p=nullptr) : QObject(p) {} };
