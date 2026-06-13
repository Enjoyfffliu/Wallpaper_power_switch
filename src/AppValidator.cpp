#include "AppValidator.h"
#include <QDir>
#include <QFileInfo>

AppValidator::AppValidator(QObject *parent) : QObject(parent) {}

AppValidator::Result AppValidator::validate(const QString &appName) {
    return validatePath("/Applications/" + appName + ".app");
}

AppValidator::Result AppValidator::validatePath(const QString &appPath) {
    Result r;

    QFileInfo bundle(appPath);
    if (!bundle.exists() || !bundle.isDir()) {
        r.error = QString("App bundle 不存在: %1").arg(appPath);
        emit validationFailed(r.error);
        return r;
    }

    QFileInfo plist(appPath + "/Contents/Info.plist");
    if (!plist.exists() || !plist.isFile()) {
        r.error = QString("App 缺少 Info.plist: %1").arg(appPath);
        emit validationFailed(r.error);
        return r;
    }

    QDir macosDir(appPath + "/Contents/MacOS");
    if (!macosDir.exists()) {
        r.error = QString("App 缺少可执行目录: %1/Contents/MacOS").arg(appPath);
        emit validationFailed(r.error);
        return r;
    }

    bool foundExe = false;
    for (const QFileInfo &entry : macosDir.entryInfoList(QDir::Files)) {
        if (entry.isExecutable()) {
            foundExe = true;
            break;
        }
    }
    if (!foundExe) {
        r.error = QString("App 中没有可执行文件: %1").arg(appPath);
        emit validationFailed(r.error);
        return r;
    }

    r.valid = true;
    return r;
}
