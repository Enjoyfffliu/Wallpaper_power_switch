#include "ConflictResolver.h"
#include <QMessageBox>
#include <QPushButton>

ConflictResolver::ConflictResolver(QObject *parent) : QObject(parent) {}

ConflictResolver::ConflictAction
ConflictResolver::resolveEngineConflict(const QString &engineName) {
    QMessageBox box;
    box.setWindowTitle("冲突: 引擎正在运行");
    box.setText(QString("壁纸引擎 %1 正在运行中。"
                        "\n\n设置静态壁纸会被引擎覆盖。")
                    .arg(engineName));
    box.setIcon(QMessageBox::Warning);

    QPushButton *closeBtn =
        box.addButton("关闭引擎并设置", QMessageBox::AcceptRole);
    box.addButton("强制设置（临时覆盖）", QMessageBox::DestructiveRole);
    QPushButton *cancelBtn = box.addButton(QMessageBox::Cancel);

    box.exec();

    if (box.clickedButton() == closeBtn)
        return CloseEngineAndSet;
    if (box.clickedButton() == cancelBtn)
        return Cancel;
    return ForceSetStatic;
}

ConflictResolver::ConflictAction
ConflictResolver::resolveBackupConflict(const QString &currentPath,
                                        const QString &backupPath) {
    QMessageBox box;
    box.setWindowTitle("壁纸备份需要更新");
    box.setText(QString("当前壁纸与备份不同。\n\n"
                        "当前: %1\n"
                        "备份: %2\n\n"
                        "断电后将恢复到备份壁纸。是否更新备份？")
                    .arg(currentPath, backupPath));
    box.setIcon(QMessageBox::Information);

    QPushButton *updateBtn =
        box.addButton("更新备份为当前壁纸", QMessageBox::AcceptRole);
    box.addButton(QMessageBox::Cancel);

    box.exec();

    if (box.clickedButton() == updateBtn)
        return UpdateBackup;
    return Cancel;
}
