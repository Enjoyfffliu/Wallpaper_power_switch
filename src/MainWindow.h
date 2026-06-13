#pragma once
#include <QMainWindow>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QCheckBox>

class PowerMonitor;
class AppManager;
class WallpaperManager;
class ConflictResolver;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;
    bool event(QEvent *e) override;

private slots:
    void onPowerChanged(int state);
    void onAppRunningChanged(bool running);
    void onCheckNow();
    void onSaveConfig();
    void onBrowseEngine();
    void onBrowseWallpaper();
    void onUpdateBackup();
    void onSetStaticWallpaper();
    void loadSettings();
    void saveSettings();

private:
    void setupUI();
    void connectSignals();
    void updateStatusDisplay();
    void showConflictWarning(const QString &msg);

    PowerMonitor *m_powerMonitor;
    AppManager *m_appManager;
    WallpaperManager *m_wallpaperManager;
    ConflictResolver *m_conflictResolver;

    // Status panel
    QLabel *m_powerIcon;
    QLabel *m_powerLabel;
    QLabel *m_batteryPctLabel;
    QLabel *m_engineStatusLabel;
    QLabel *m_wallpaperPreview;
    QLabel *m_wallpaperPathLabel;
    QLabel *m_backupPreview;
    QLabel *m_backupPathLabel;

    // Settings panel
    QComboBox *m_engineCombo;
    QPushButton *m_browseEngineBtn;
    QCheckBox *m_validateCheck;
    QLineEdit *m_restorePathEdit;
    QPushButton *m_browseWallpaperBtn;
    QPushButton *m_updateBackupBtn;
    QSpinBox *m_intervalSpin;
    QPushButton *m_saveBtn;
    QPushButton *m_checkNowBtn;
    QPushButton *m_setStaticBtn;

    // Conflict bar
    QLabel *m_conflictLabel;
    QWidget *m_conflictBar;
};
