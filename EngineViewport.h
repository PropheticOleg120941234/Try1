#ifndef ENGINEVIEWPORT_H
#define ENGINEVIEWPORT_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_1_Core>

class EngineViewport : public QOpenGLWidget, protected QOpenGLFunctions_4_1_Core {
    Q_OBJECT

public:
    // Этот конструктор обязателен, чтобы Qt могла встроить виджет внутрь QMainWindow
    explicit EngineViewport(QWidget* parent = nullptr);
    ~EngineViewport() override;

protected:
    void initializeGL() override;
    void paintGL() override;
};

#endif // ENGINEVIEWPORT_H
