#include "MainWindow.h"
#include "PowerMonitor.h"
#include "AppManager.h"
#include "WallpaperManager.h"
#include "ConflictResolver.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>
#include <QTimer>
#include <QFrame>
#include <QApplication>
#include <QPixmap>
#include <QFileInfo>
#include <QCloseEvent>
#include <CoreFoundation/CFNotificationCenter.h>

// CFNotification 回调：macOS Dock 点击 / Cmd+Tab 切回时触发
static void dockActivationCallback(CFNotificationCenterRef, void *observer,
                                    CFStringRef, const void *,
                                    CFDictionaryRef) {
    auto *w = static_cast<QWidget *>(observer);
    if (w && !w->isVisible()) {
        w->show();
        w->raise();
        w->activateWindow();
    }
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Wallpaper Power Switch");
    setMinimumSize(520, 420);

    m_powerMonitor = new PowerMonitor(this);
    m_appManager = new AppManager(this);
    m_wallpaperManager = new WallpaperManager(this);
    m_conflictResolver = new ConflictResolver(this);

    setupUI();
    connectSignals();
    loadSettings();

    // 注册 macOS 原生通知：Dock 点击时重新显示窗口
    CFNotificationCenterAddObserver(
        CFNotificationCenterGetLocalCenter(),
        this,                                // 传递 this 作为 observer
        &dockActivationCallback,
        CFSTR("NSApplicationDidBecomeActiveNotification"),
        nullptr,
        CFNotificationSuspensionBehaviorDeliverImmediately);

    m_powerMonitor->setPollInterval(m_intervalSpin->value());
    m_powerMonitor->start();

    QTimer::singleShot(500, this, &MainWindow::onCheckNow);
}

MainWindow::~MainWindow() {}

void MainWindow::closeEvent(QCloseEvent *event) {
    // 点红色 X → 隐藏到后台，不退出
    hide();
    event->ignore();
}

bool MainWindow::event(QEvent *e) {
    // 处理各种激活事件：Dock 点击、Cmd+Tab 切回等
    // QEvent::ApplicationActivate 和 ApplicationStateChange 都试试
    if (e->type() == QEvent::ApplicationActivate ||
        e->type() == QEvent::ApplicationStateChange) {
        if (!isVisible()) {
            show();
            raise();
            activateWindow();
        }
    }
    return QMainWindow::event(e);
}

void MainWindow::setupUI() {
    auto *central = new QWidget;
    auto *mainLayout = new QVBoxLayout(central);
    mainLayout->setSpacing(8);

    auto *topRow = new QHBoxLayout;

    // --- Status panel ---
    auto *statusGroup = new QGroupBox("状态");
    auto *statusLayout = new QVBoxLayout(statusGroup);

    m_powerIcon = new QLabel("🔌");
    m_powerIcon->setStyleSheet("font-size: 18px;");
    m_powerLabel = new QLabel("检测中...");
    m_batteryPctLabel = new QLabel("");
    m_engineStatusLabel = new QLabel("引擎: --");

    statusLayout->addWidget(m_powerIcon);
    statusLayout->addWidget(m_powerLabel);
    statusLayout->addWidget(m_batteryPctLabel);
    statusLayout->addWidget(m_engineStatusLabel);

    m_wallpaperPreview = new QLabel;
    m_wallpaperPreview->setFixedSize(96, 60);
    m_wallpaperPreview->setStyleSheet("background: #333; border: 1px solid #555;");
    m_wallpaperPreview->setAlignment(Qt::AlignCenter);
    m_wallpaperPreview->setText("壁纸");
    statusLayout->addWidget(m_wallpaperPreview);

    m_wallpaperPathLabel = new QLabel("--");
    m_wallpaperPathLabel->setWordWrap(true);
    m_wallpaperPathLabel->setMaximumWidth(180);
    statusLayout->addWidget(m_wallpaperPathLabel);

    auto *backupTitle = new QLabel("断电恢复:");
    backupTitle->setStyleSheet("font-size: 10px; color: #888;");
    statusLayout->addWidget(backupTitle);

    m_backupPreview = new QLabel;
    m_backupPreview->setFixedSize(64, 40);
    m_backupPreview->setStyleSheet("background: #222; border: 1px solid #444;");
    m_backupPreview->setAlignment(Qt::AlignCenter);
    m_backupPreview->setText("备");
    statusLayout->addWidget(m_backupPreview);

    m_backupPathLabel = new QLabel("--");
    m_backupPathLabel->setWordWrap(true);
    m_backupPathLabel->setMaximumWidth(180);
    statusLayout->addWidget(m_backupPathLabel);

    statusLayout->addStretch();
    topRow->addWidget(statusGroup);

    // --- Settings panel ---
    auto *settingsGroup = new QGroupBox("设置");
    auto *settingsLayout = new QVBoxLayout(settingsGroup);

    settingsLayout->addWidget(new QLabel("壁纸引擎:"));
    auto *engineRow = new QHBoxLayout;
    m_engineCombo = new QComboBox;
    m_engineCombo->setEditable(true);
    m_engineCombo->setMinimumWidth(120);
    m_browseEngineBtn = new QPushButton("浏览...");
    engineRow->addWidget(m_engineCombo);
    engineRow->addWidget(m_browseEngineBtn);
    settingsLayout->addLayout(engineRow);

    m_validateCheck = new QCheckBox("启动前验证 App 完整性");
    m_validateCheck->setChecked(true);
    settingsLayout->addWidget(m_validateCheck);

    settingsLayout->addSpacing(8);
    settingsLayout->addWidget(new QLabel("断电恢复壁纸:"));
    auto *restoreRow = new QHBoxLayout;
    m_restorePathEdit = new QLineEdit;
    m_restorePathEdit->setPlaceholderText("自动保存当前壁纸...");
    m_browseWallpaperBtn = new QPushButton("浏览...");
    restoreRow->addWidget(m_restorePathEdit);
    restoreRow->addWidget(m_browseWallpaperBtn);
    settingsLayout->addLayout(restoreRow);

    auto *backupBtnRow = new QHBoxLayout;
    m_updateBackupBtn = new QPushButton("更新为当前壁纸");
    m_setStaticBtn = new QPushButton("设置此壁纸");
    backupBtnRow->addWidget(m_updateBackupBtn);
    backupBtnRow->addWidget(m_setStaticBtn);
    settingsLayout->addLayout(backupBtnRow);

    settingsLayout->addSpacing(8);
    settingsLayout->addWidget(new QLabel("检测间隔 (秒):"));
    m_intervalSpin = new QSpinBox;
    m_intervalSpin->setRange(5, 300);
    m_intervalSpin->setValue(30);
    settingsLayout->addWidget(m_intervalSpin);

    settingsLayout->addSpacing(12);
    auto *actionRow = new QHBoxLayout;
    m_saveBtn = new QPushButton("保存配置");
    m_checkNowBtn = new QPushButton("⚡ 立即检测");
    actionRow->addWidget(m_saveBtn);
    actionRow->addWidget(m_checkNowBtn);
    settingsLayout->addLayout(actionRow);

    settingsLayout->addStretch();
    topRow->addWidget(settingsGroup);

    mainLayout->addLayout(topRow);

    // --- Conflict bar ---
    m_conflictBar = new QWidget;
    m_conflictBar->setStyleSheet("background: #553300; border-radius: 4px;");
    auto *conflictLayout = new QHBoxLayout(m_conflictBar);
    conflictLayout->setContentsMargins(8, 4, 8, 4);
    m_conflictLabel = new QLabel;
    m_conflictLabel->setStyleSheet("color: #FFB000;");
    conflictLayout->addWidget(m_conflictLabel);
    m_conflictBar->hide();
    mainLayout->addWidget(m_conflictBar);

    setCentralWidget(central);
}

void MainWindow::connectSignals() {
    connect(m_powerMonitor, &PowerMonitor::powerChanged, this,
            &MainWindow::onPowerChanged);
    connect(m_powerMonitor, &PowerMonitor::batteryPercentage, this,
            [this](int pct) {
                m_batteryPctLabel->setText(QString("电量: %1%").arg(pct));
            });

    connect(m_appManager, &AppManager::appRunningChanged, this,
            &MainWindow::onAppRunningChanged);
    connect(m_appManager, &AppManager::launchFailed, this,
            [this](const QString &reason) {
                showConflictWarning(QString("启动失败: %1").arg(reason));
            });
    connect(m_appManager, &AppManager::quitFailed, this,
            [this](const QString &reason) {
                showConflictWarning(QString("关闭失败: %1").arg(reason));
            });

    connect(m_wallpaperManager, &WallpaperManager::errorOccurred, this,
            [this](const QString &msg) { showConflictWarning(msg); });

    connect(m_checkNowBtn, &QPushButton::clicked, this, &MainWindow::onCheckNow);
    connect(m_saveBtn, &QPushButton::clicked, this, &MainWindow::onSaveConfig);
    connect(m_browseEngineBtn, &QPushButton::clicked, this, &MainWindow::onBrowseEngine);
    connect(m_browseWallpaperBtn, &QPushButton::clicked, this, &MainWindow::onBrowseWallpaper);
    connect(m_updateBackupBtn, &QPushButton::clicked, this, &MainWindow::onUpdateBackup);
    connect(m_setStaticBtn, &QPushButton::clicked, this, &MainWindow::onSetStaticWallpaper);
}

void MainWindow::onPowerChanged(int state) {
    updateStatusDisplay();

    if (state == PowerMonitor::AC) {
        if (!m_appManager->isRunning()) {
            m_appManager->launchApp();
        }
    } else {
        if (m_appManager->isRunning()) {
            m_appManager->quitApp();
        }
        QTimer::singleShot(2000, this, [this]() {
            if (!m_appManager->isRunning()) {
                m_wallpaperManager->restoreFromBackup();
                updateStatusDisplay();
            }
        });
    }
}

void MainWindow::onAppRunningChanged(bool running) {
    updateStatusDisplay();
    if (!running) {
        QString current = m_wallpaperManager->currentWallpaper();
        QString backup = m_wallpaperManager->backupWallpaperPath();
        if (!backup.isEmpty() && !current.isEmpty() && current != backup) {
            showConflictWarning("备份壁纸与当前壁纸不同，可点击 [更新为当前壁纸]");
        }
    }
}

void MainWindow::onCheckNow() {
    m_powerMonitor->checkNow();
    m_appManager->checkRunning();
    updateStatusDisplay();
}

void MainWindow::onSaveConfig() {
    saveSettings();
    m_powerMonitor->setPollInterval(m_intervalSpin->value());
    m_conflictBar->hide();
}

void MainWindow::onBrowseEngine() {
    QString path = QFileDialog::getOpenFileName(
        this, "选择壁纸引擎 App", "/Applications", "Applications (*.app)");
    if (!path.isEmpty()) {
        QFileInfo fi(path);
        QString name = fi.completeBaseName();
        m_engineCombo->setCurrentText(name);
        m_appManager->setAppName(name);
        updateStatusDisplay();
    }
}

void MainWindow::onBrowseWallpaper() {
    QString path = QFileDialog::getOpenFileName(
        this, "选择断电恢复壁纸", QDir::homePath(),
        "Images (*.jpg *.jpeg *.png *.heic *.bmp)");
    if (!path.isEmpty()) {
        m_restorePathEdit->setText(path);
        m_wallpaperManager->setBackupPath(path);
        updateStatusDisplay();
    }
}

void MainWindow::onUpdateBackup() {
    QString saved = m_wallpaperManager->saveBackup();
    if (!saved.isEmpty()) {
        m_conflictBar->hide();
        updateStatusDisplay();
    }
}

void MainWindow::onSetStaticWallpaper() {
    QString path = m_restorePathEdit->text();
    if (path.isEmpty()) {
        path = m_wallpaperManager->backupWallpaperPath();
    }
    if (path.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择或更新壁纸备份");
        return;
    }

    if (m_appManager->isRunning()) {
        auto action = m_conflictResolver->resolveEngineConflict(m_appManager->appName());
        switch (action) {
        case ConflictResolver::CloseEngineAndSet:
            m_appManager->quitApp();
            QTimer::singleShot(2000, this, [this, path]() {
                m_wallpaperManager->setWallpaper(path);
                updateStatusDisplay();
            });
            break;
        case ConflictResolver::ForceSetStatic:
            m_wallpaperManager->setWallpaper(path);
            updateStatusDisplay();
            showConflictWarning("静态壁纸已设置，但引擎仍在运行，可能会被覆盖");
            break;
        case ConflictResolver::Cancel:
        default:
            break;
        }
    } else {
        m_wallpaperManager->setWallpaper(path);
        updateStatusDisplay();
    }
}

void MainWindow::updateStatusDisplay() {
    switch (m_powerMonitor->currentState()) {
    case PowerMonitor::AC:
        m_powerIcon->setText("🔌");
        m_powerLabel->setText("AC 电源 (充电中)");
        break;
    case PowerMonitor::Battery:
        m_powerIcon->setText("🔋");
        m_powerLabel->setText("电池供电");
        break;
    default:
        m_powerIcon->setText("❓");
        m_powerLabel->setText("未知 (台式机)");
        break;
    }

    if (m_appManager->isRunning()) {
        m_engineStatusLabel->setText(
            QString("引擎: %1 ● 运行中").arg(m_appManager->appName()));
        m_engineStatusLabel->setStyleSheet("color: #4CAF50;");
    } else {
        m_engineStatusLabel->setText(
            QString("引擎: %1 ○ 已关闭").arg(m_appManager->appName()));
        m_engineStatusLabel->setStyleSheet("color: #888;");
    }

    QString wp = m_wallpaperManager->currentWallpaper();
    if (!wp.isEmpty()) {
        m_wallpaperPathLabel->setText(wp);
        QPixmap pm(wp);
        if (!pm.isNull()) {
            m_wallpaperPreview->setPixmap(
                pm.scaled(96, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }

    if (m_wallpaperManager->backupExists()) {
        QString bp = m_wallpaperManager->backupWallpaperPath();
        m_backupPathLabel->setText(bp);
        QPixmap bpm(bp);
        if (!bpm.isNull()) {
            m_backupPreview->setPixmap(
                bpm.scaled(64, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }
}

void MainWindow::showConflictWarning(const QString &msg) {
    m_conflictLabel->setText("⚠ " + msg);
    m_conflictBar->show();
}

void MainWindow::loadSettings() {
    QSettings settings;
    QString engineName = settings.value("engineAppName", "WaifuX").toString();
    m_engineCombo->setCurrentText(engineName);
    m_appManager->setAppName(engineName);

    QString restorePath = settings.value("restoreWallpaperPath", "").toString();
    if (!restorePath.isEmpty()) {
        m_restorePathEdit->setText(restorePath);
        m_wallpaperManager->setBackupPath(restorePath);
    }

    int interval = settings.value("checkInterval", 30).toInt();
    m_intervalSpin->setValue(interval);

    bool validate = settings.value("validateBeforeLaunch", true).toBool();
    m_validateCheck->setChecked(validate);
}

void MainWindow::saveSettings() {
    QSettings settings;
    settings.setValue("engineAppName", m_engineCombo->currentText());
    settings.setValue("restoreWallpaperPath", m_restorePathEdit->text());
    settings.setValue("checkInterval", m_intervalSpin->value());
    settings.setValue("validateBeforeLaunch", m_validateCheck->isChecked());

    m_appManager->setAppName(m_engineCombo->currentText());
    QString newPath = m_restorePathEdit->text();
    if (!newPath.isEmpty()) {
        m_wallpaperManager->setBackupPath(newPath);
    }
}
