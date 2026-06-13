#include "AppManager.h"
#include "AppValidator.h"
#include <QProcess>
#include <QDebug>

AppManager::AppManager(QObject *parent) : QObject(parent) {
    m_validator = new AppValidator(this);
    m_checkTimer.setInterval(3000);
    connect(&m_checkTimer, &QTimer::timeout, this, &AppManager::checkRunning);
}

void AppManager::setAppName(const QString &name) { m_appName = name; }
QString AppManager::appName() const { return m_appName; }

bool AppManager::isRunning() {
    QProcess proc;
    proc.start("pgrep", QStringList() << "-x" << m_appName);
    proc.waitForFinished(2000);
    return proc.exitCode() == 0;
}

bool AppManager::isValid() {
    return m_validator->validate(m_appName).valid;
}

void AppManager::checkRunning() {
    bool running = isRunning();
    if (running != m_wasRunning) {
        m_wasRunning = running;
        emit appRunningChanged(running);
    }
}

void AppManager::launchApp() {
    if (!isValid()) {
        auto result = m_validator->validate(m_appName);
        emit launchFailed(result.error);
        return;
    }
    if (isRunning()) return;

    if (doLaunch()) {
        m_checkTimer.start();
    }
}

void AppManager::quitApp() {
    if (!isRunning()) {
        m_checkTimer.stop();
        return;
    }
    if (doQuit()) {
        m_checkTimer.stop();
    }
}

bool AppManager::doLaunch() {
    QProcess proc;
    proc.start("open", QStringList() << "-a" << m_appName);
    proc.waitForFinished(3000);

    if (proc.exitCode() != 0) {
        QString err = QString::fromUtf8(proc.readAllStandardError()).trimmed();
        emit launchFailed(err.isEmpty() ? QString("启动 %1 失败").arg(m_appName) : err);
        return false;
    }

    QProcess::execute("sleep", QStringList() << "1");
    m_wasRunning = isRunning();
    emit appLaunched();
    emit appRunningChanged(m_wasRunning);
    return true;
}

bool AppManager::doQuit() {
    QString script = QString("tell application \"%1\" to quit").arg(m_appName);
    QProcess proc1;
    proc1.start("osascript", QStringList() << "-e" << script);
    proc1.waitForFinished(3000);

    QProcess::execute("sleep", QStringList() << "1");

    if (!isRunning()) {
        m_wasRunning = false;
        emit appQuit();
        emit appRunningChanged(false);
        return true;
    }

    QProcess proc2;
    proc2.start("pkill", QStringList() << "-x" << m_appName);
    proc2.waitForFinished(2000);

    QProcess::execute("sleep", QStringList() << "1");

    if (isRunning()) {
        emit quitFailed(QString("无法关闭 %1").arg(m_appName));
        m_wasRunning = true;
        return false;
    }

    m_wasRunning = false;
    emit appQuit();
    emit appRunningChanged(false);
    return true;
}
