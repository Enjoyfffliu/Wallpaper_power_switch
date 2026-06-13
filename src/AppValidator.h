#pragma once
#include <QObject>
#include <QString>

class AppValidator : public QObject {
    Q_OBJECT
public:
    explicit AppValidator(QObject *parent = nullptr);

    struct Result {
        bool valid = false;
        QString error;
    };

    Result validate(const QString &appName);
    Result validatePath(const QString &appPath);

signals:
    void validationFailed(const QString &error);
};
