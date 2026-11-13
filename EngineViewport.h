#ifndef ENGINEVIEWPORT_H
#define ENGINEVIEWPORT_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_1_Core>
#include <QOpenGLShaderProgram>
#include <QElapsedTimer>

class EngineViewport : public QOpenGLWidget, protected QOpenGLFunctions_4_1_Core {
    Q_OBJECT

public:
    explicit EngineViewport(QWidget* parent = nullptr);
    ~EngineViewport() override;

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    void updatePhysics(double dt);
    void renderScene(double alpha);

    // Переменные для фиксированного игрового цикла
    QElapsedTimer timer;
    double accumulator = 0.0;
    const double FIXED_DT = 1.0 / 60.0; // Логика обновляется 60 раз в секунду

    // Данные для физики (угол вращения)
    float prevAngle = 0.0f;
    float currentAngle = 0.0f;

    // Объекты OpenGL
    QOpenGLShaderProgram* shaderProgram = nullptr;
    unsigned int VAO = 0;
    unsigned int VBO = 0;
};

#endif // ENGINEVIEWPORT_H