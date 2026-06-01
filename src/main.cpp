#include "app/Application.h"
#include "ui/MainWindow.h"

int main(int argc, char* argv[])
{
    Application app(argc, argv);
    app.setApplicationName("TsunamiSimUI");
    app.setOrganizationName("PostGS");

    MainWindow window;
    window.show();

    return app.exec();
}
