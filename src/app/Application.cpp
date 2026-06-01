#include "Application.h"

Application::Application(int& argc, char** argv)
    : QApplication(argc, argv),
      settings_("PostGS", "TsunamiSimUI")
{
}

QString Application::lastProjectPath() const
{
    return settings_.value("lastProjectPath").toString();
}

void Application::setLastProjectPath(const QString& path)
{
    settings_.setValue("lastProjectPath", path);
}

QString Application::simulatorPath() const
{
    return settings_.value("simulatorPath").toString();
}

void Application::setSimulatorPath(const QString& path)
{
    settings_.setValue("simulatorPath", path);
}
