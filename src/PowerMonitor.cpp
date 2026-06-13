#include "PowerMonitor.h"
#include <IOKit/ps/IOPSKeys.h>
#include <QDebug>

PowerMonitor::PowerMonitor(QObject *parent) : QObject(parent) {
    m_state = querySystemState();
    connect(&m_timer, &QTimer::timeout, this, &PowerMonitor::checkNow);
}

PowerMonitor::~PowerMonitor() { stop(); }

void PowerMonitor::start() {
    CFRunLoopSourceRef src = IOPSNotificationCreateRunLoopSource(
        powerSourceCallback, this);
    if (src) {
        CFRunLoopAddSource(CFRunLoopGetCurrent(), src, kCFRunLoopDefaultMode);
        m_runLoopSource = src;
    }
    m_timer.start();
}

void PowerMonitor::stop() {
    m_timer.stop();
    if (m_runLoopSource) {
        CFRunLoopRemoveSource(CFRunLoopGetCurrent(), m_runLoopSource,
                              kCFRunLoopDefaultMode);
        CFRelease(m_runLoopSource);
        m_runLoopSource = nullptr;
    }
}

void PowerMonitor::setPollInterval(int seconds) {
    m_timer.setInterval(seconds * 1000);
}

int PowerMonitor::pollInterval() const { return m_timer.interval() / 1000; }

PowerMonitor::PowerState PowerMonitor::currentState() const { return m_state; }

void PowerMonitor::checkNow() { handlePowerChange(); }

void PowerMonitor::powerSourceCallback(void *context) {
    auto *self = static_cast<PowerMonitor *>(context);
    self->checkNow();
}

void PowerMonitor::handlePowerChange() {
    PowerState prev = m_state;
    m_state = querySystemState();

    int pct = -1;
    CFTypeRef blob = IOPSCopyPowerSourcesInfo();
    if (blob) {
        CFArrayRef list = IOPSCopyPowerSourcesList(blob);
        if (list && CFArrayGetCount(list) > 0) {
            CFDictionaryRef ps = IOPSGetPowerSourceDescription(
                blob, CFArrayGetValueAtIndex(list, 0));
            if (ps) {
                CFNumberRef pctNum = (CFNumberRef)CFDictionaryGetValue(
                    ps, CFSTR(kIOPSCurrentCapacityKey));
                if (pctNum) CFNumberGetValue(pctNum, kCFNumberIntType, &pct);
            }
        }
        if (list) CFRelease(list);
        CFRelease(blob);
    }
    if (pct >= 0) emit batteryPercentage(pct);

    if (m_state != prev) {
        qDebug() << "Power state changed:" << prev << "->" << m_state;
        emit powerChanged(m_state);
    }
}

PowerMonitor::PowerState PowerMonitor::querySystemState() const {
    CFTypeRef blob = IOPSCopyPowerSourcesInfo();
    if (!blob) return AC;

    CFArrayRef list = IOPSCopyPowerSourcesList(blob);
    if (!list || CFArrayGetCount(list) == 0) {
        if (list) CFRelease(list);
        CFRelease(blob);
        return AC;
    }

    CFDictionaryRef ps = IOPSGetPowerSourceDescription(
        blob, CFArrayGetValueAtIndex(list, 0));
    CFRelease(list);
    CFRelease(blob);

    if (!ps) return AC;

    CFStringRef state = (CFStringRef)CFDictionaryGetValue(
        ps, CFSTR(kIOPSPowerSourceStateKey));
    if (!state) return AC;

    if (CFStringCompare(state, CFSTR(kIOPSACPowerValue), 0) == kCFCompareEqualTo)
        return AC;
    return Battery;
}
