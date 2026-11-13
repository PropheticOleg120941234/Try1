#include <QApplication>
#include <QMainWindow>
#include <QSurfaceFormat>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QDoubleSpinBox>
#include "EngineViewport.h"

int main(int argc, char *argv[]) {
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setVersion(4, 1);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);

    QApplication app(argc, argv);

    QMainWindow mainWindow;
    mainWindow.setWindowTitle("My 3D Engine Editor");
    mainWindow.resize(1280, 720);

    // 1. Создаем главный контейнер, который разделит окно на Лево (3D) и Право (Интерфейс)
    QWidget* centralWidget = new QWidget(&mainWindow);
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    mainWindow.setCentralWidget(centralWidget);

    // 2. Создаем наше 3D окно движка
    EngineViewport* viewport = new EngineViewport(centralWidget);
    // Отдаем ему 80% ширины окна (коэффициент растяжения 4)
    mainLayout->addWidget(viewport, 4);

    // 3. Создаем правую панель инструментов редактора
    QWidget* rightPanel = new QWidget(centralWidget);
    QVBoxLayout* panelLayout = new QVBoxLayout(rightPanel);
    panelLayout->setAlignment(Qt::AlignTop); // Прижимаем элементы к верху панели
    mainLayout->addWidget(rightPanel, 1);    // Отдаем панели 20% ширины окна (коэффициент 1)

    // --- ЭЛЕМЕНТ UI 1: Скорость вращения ---
    QLabel* speedLabel = new QLabel("Скорость вращения куба:", rightPanel);
    panelLayout->addWidget(speedLabel);

    QSlider* speedSlider = new QSlider(Qt::Horizontal, rightPanel);
    speedSlider->setRange(0, 360); // От 0 до 360 градусов в секунду
    speedSlider->setValue(45);     // Стартовое значение
    panelLayout->addWidget(speedSlider);

    // --- ЭЛЕМЕНТ UI 2: Дальность обзора ---
    QLabel* farLabel = new QLabel("Дальность обзора (Far Plane):", rightPanel);
    panelLayout->addWidget(farLabel);

    QDoubleSpinBox* farSpinBox = new QDoubleSpinBox(rightPanel);
    farSpinBox->setRange(1.0, 500.0); // От 1 до 500 единиц
    farSpinBox->setValue(100.0);      // Стартовое значение
    panelLayout->addWidget(farSpinBox);

    // =========================================================================
    // СВЯЗЫВАЕМ ИНТЕРФЕЙС С ДВИЖКОМ (Магия Сигналов и Слотов)
    // =========================================================================

    // Когда меняется слайдер -> вызывается метод setRotationSpeed внутри нашего вьюпорта
    QObject::connect(speedSlider, &QSlider::valueChanged, viewport, &EngineViewport::setRotationSpeed);

    // 当 Когда меняется значение в спинбоксе -> вызывается setFarPlane внутри вьюпорта
    QObject::connect(farSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), viewport, &EngineViewport::setFarPlane);

    mainWindow.show();
    return app.exec();
}