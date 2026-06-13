#pragma once
#include <QObject>
#include <QString>
#include <QTimer>

class AppValidator;

class AppManager : public QObject {
    Q_OBJECT
public:
    explicit AppManager(QObject *parent = nullptr);

    void setAppName(const QString &name);
    QString appName() const;
    bool isRunning();
    bool isValid();

public slots:
    void launchApp();
    void quitApp();
    void checkRunning();

signals:
    void appLaunched();
    void appQuit();
    void appRunningChanged(bool running);
    void launchFailed(const QString &reason);
    void quitFailed(const QString &reason);

private:
    QString m_appName;
    class AppValidator *m_validator;
    QTimer m_checkTimer;
    bool m_wasRunning = false;

    bool doLaunch();
    bool doQuit();
};
