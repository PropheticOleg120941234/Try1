#include <QApplication>
#include <QMainWindow>
#include <QSurfaceFormat>
#include "EngineViewport.h"

int main(int argc, char *argv[]) {
    // Настраиваем современный OpenGL 4.1 Core Profile (работает и на Windows, и на macOS)
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setVersion(4, 1);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setSwapInterval(1); // Включаем V-Sync (привязка к частоте монитора)
    QSurfaceFormat::setDefaultFormat(format);

    QApplication app(argc, argv);

    QMainWindow mainWindow;
    mainWindow.setWindowTitle("My 3D Engine");
    mainWindow.resize(1280, 720);

    // Вставляем наш 3D-вьпорт в центр главного окна
    EngineViewport* viewport = new EngineViewport(&mainWindow);
    mainWindow.setCentralWidget(viewport);

    mainWindow.show();
    return app.exec();
}