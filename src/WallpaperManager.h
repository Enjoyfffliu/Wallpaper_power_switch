#pragma once
#include <QObject>
#include <QString>

class WallpaperManager : public QObject {
    Q_OBJECT
public:
    explicit WallpaperManager(QObject *parent = nullptr);

    QString currentWallpaper();
    QString backupWallpaperPath();
    bool backupExists();

public slots:
    bool setWallpaper(const QString &imagePath);
    QString saveBackup();
    bool restoreFromBackup();
    bool setBackupPath(const QString &imagePath);

signals:
    void wallpaperChanged(const QString &path);
    void backupUpdated(const QString &path);
    void errorOccurred(const QString &msg);

private:
    QString m_backupDir;
    QString m_backupFile;
    QString runOsascript(const QString &script);
};
