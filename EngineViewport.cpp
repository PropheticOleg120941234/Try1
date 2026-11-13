#include "EngineViewport.h"

// Конструктор: просто передает parent в базовый класс QOpenGLWidget
EngineViewport::EngineViewport(QWidget* parent) : QOpenGLWidget(parent) {
}

// Деструктор: пока пустой
EngineViewport::~EngineViewport() {
}

void EngineViewport::initializeGL() {
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
}

void EngineViewport::paintGL() {
    // Очищаем экран базовым цветом (темно-серым) перед каждым кадром
    glClearColor(0.15f, 0.15f, 0.18f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}