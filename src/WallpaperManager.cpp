#include "WallpaperManager.h"
#include <QProcess>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDebug>

WallpaperManager::WallpaperManager(QObject *parent) : QObject(parent) {
    m_backupDir = QDir::homePath() + "/.cache";
    m_backupFile = m_backupDir + "/wallpaper-static-backup";
    QDir().mkpath(m_backupDir);
}

QString WallpaperManager::runOsascript(const QString &script) {
    QProcess proc;
    proc.start("osascript", QStringList() << "-e" << script);
    proc.waitForFinished(5000);
    QString out = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
    if (proc.exitCode() != 0) {
        QString err = QString::fromUtf8(proc.readAllStandardError()).trimmed();
        if (!err.isEmpty()) {
            qWarning() << "osascript error:" << err;
            emit errorOccurred(err);
        }
    }
    return out;
}

QString WallpaperManager::currentWallpaper() {
    return runOsascript(
        "tell application \"System Events\" to get picture of desktop 1");
}

bool WallpaperManager::setWallpaper(const QString &imagePath) {
    QFileInfo fi(imagePath);
    if (!fi.exists()) {
        emit errorOccurred(QString("壁纸文件不存在: %1").arg(imagePath));
        return false;
    }

    QString script = QString(
        "tell application \"System Events\"\n"
        "    set dc to count of desktops\n"
        "    repeat with i from 1 to dc\n"
        "        tell desktop i\n"
        "            set picture to POSIX file \"%1\"\n"
        "        end tell\n"
        "    end repeat\n"
        "end tell").arg(imagePath);

    runOsascript(script);
    return true;
}

bool WallpaperManager::backupExists() {
    return QFile::exists(m_backupFile);
}

QString WallpaperManager::backupWallpaperPath() {
    QFile f(m_backupFile);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return {};
    QString content = QString::fromUtf8(f.readAll()).trimmed();
    return QString(content).replace("file://", "");
}

QString WallpaperManager::saveBackup() {
    QString current = currentWallpaper();
    if (current.isEmpty()) {
        emit errorOccurred("无法获取当前壁纸路径");
        return {};
    }

    QFile f(m_backupFile);
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        f.write(QString("file://%1\n").arg(current).toUtf8());
    }
    emit backupUpdated(current);
    qDebug() << "Backup saved:" << current;
    return current;
}

bool WallpaperManager::restoreFromBackup() {
    if (!backupExists()) {
        emit errorOccurred("未找到壁纸备份");
        return false;
    }
    QString path = backupWallpaperPath();
    if (path.isEmpty()) return false;
    return setWallpaper(path);
}

bool WallpaperManager::setBackupPath(const QString &imagePath) {
    QFile f(m_backupFile);
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        f.write(QString("file://%1\n").arg(imagePath).toUtf8());
        emit backupUpdated(imagePath);
        return true;
    }
    return false;
}
