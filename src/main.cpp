#include <QApplication>
#include <QDebug>
#include <iostream>
#include "ui/MainWindow.h"

class SoftphoneApplication : public QApplication {
public:
    using QApplication::QApplication;

    bool notify(QObject* receiver, QEvent* event) override {
        try {
            return QApplication::notify(receiver, event);
        } catch (const std::exception& e) {
            qCritical() << "[CRITICAL] Caught unhandled exception in event loop:" << e.what();
            return false;
        } catch (...) {
            qCritical() << "[CRITICAL] Caught unhandled unknown exception in event loop.";
            return false;
        }
    }
};

int main(int argc, char *argv[])
{
    try {
        SoftphoneApplication app(argc, argv);
        app.setApplicationName("GeSoftphone");
        app.setOrganizationName("GE");

        MainWindow window;
        window.show();

        return app.exec();
    } catch (const std::exception& e) {
        std::cerr << "[CRITICAL] Unhandled top-level exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "[CRITICAL] Unhandled top-level unknown exception." << std::endl;
        return 1;
    }
}

