#pragma once
#include <QObject>
#include <QTimer>
#include <IOKit/ps/IOPowerSources.h>

class PowerMonitor : public QObject {
    Q_OBJECT
public:
    enum PowerState { AC, Battery, Unknown };
    Q_ENUM(PowerState)

    explicit PowerMonitor(QObject *parent = nullptr);
    ~PowerMonitor();

    PowerState currentState() const;
    void setPollInterval(int seconds);
    int pollInterval() const;

public slots:
    void start();
    void stop();
    void checkNow();

signals:
    void powerChanged(PowerMonitor::PowerState state);
    void batteryPercentage(int percent);

private:
    static void powerSourceCallback(void *context);
    void handlePowerChange();
    PowerState querySystemState() const;

    QTimer m_timer;
    PowerState m_state = Unknown;
    CFRunLoopSourceRef m_runLoopSource = nullptr;
};
