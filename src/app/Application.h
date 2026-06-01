#pragma once

#include <QApplication>
#include <QSettings>

class Application : public QApplication
{
    Q_OBJECT

public:
    Application(int& argc, char** argv);

    QString lastProjectPath() const;
    void setLastProjectPath(const QString& path);

    QString simulatorPath() const;
    void setSimulatorPath(const QString& path);

private:
    QSettings settings_;
};
