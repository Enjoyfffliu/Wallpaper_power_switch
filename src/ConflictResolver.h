#pragma once
#include <QObject>
#include <QString>

class ConflictResolver : public QObject {
    Q_OBJECT
public:
    enum ConflictAction {
        Cancel,
        CloseEngineAndSet,
        ForceSetStatic,
        UpdateBackup
    };
    Q_ENUM(ConflictAction)

    explicit ConflictResolver(QObject *parent = nullptr);

    ConflictAction resolveEngineConflict(const QString &engineName);
    ConflictAction resolveBackupConflict(const QString &currentPath,
                                         const QString &backupPath);

signals:
    void conflictDetected(const QString &description);
};
