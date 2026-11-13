#ifndef ENGINEVIEWPORT_H
#define ENGINEVIEWPORT_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_1_Core>
#include <QOpenGLShaderProgram>
#include <QElapsedTimer>
#include <QTimer>

#include <QKeyEvent>
#include <QMouseEvent>

class EngineViewport : public QOpenGLWidget, protected QOpenGLFunctions_4_1_Core {
    Q_OBJECT

public:
    explicit EngineViewport(QWidget* parent = nullptr);
    ~EngineViewport() override;

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

public slots:
    // Эти функции Qt будет вызывать автоматически, когда мы будем двигать UI элементы
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

    void setRotationSpeed(int speed);
    void setFarPlane(double farValue);

private slots:
    void gameLoop(); // Наш собственный стабильный игровой цикл

private:
    void updatePhysics(double dt);
    void renderScene(double alpha);

    float rotationSpeed = 45.0f; // Начальная скорость (градусов в секунду)
    float farPlane = 100.0f;

    QPoint CenterPoint() const;

    // Переменные для фиксированного игрового цикла
    QTimer* frameTimer = nullptr; // Таймер, который будет пинать игровой цикл
    QElapsedTimer timer;          // Высокоточный замерщик прошедшего времени
    double accumulator = 0.0;
    const double FIXED_DT = 1.0 / 60.0; // Логика обновляется 60 раз в секунду

    // Данные для физики (угол вращения)
    float prevAngle = 0.0f;
    float currentAngle = 0.0f;

    // Объекты OpenGL
    QOpenGLShaderProgram* shaderProgram = nullptr;
    unsigned int VAO = 0;
    unsigned int VBO = 0;

    QVector3D cameraPos = QVector3D(0.0f, 0.0f, 0.0f); // Камера изначально в центре
    float cameraYaw = -90.0f;  // Поворот влево-вправо (смотрим вперед по умолчанию)
    float cameraPitch = 0.0f;

    bool keys[1024] = { false };

    // Для расчета движения мыши
    QPoint lastMousePos;
    //bool firstMouse = true;
    bool cursor_captured = false;
};

#endif // ENGINEVIEWPORT_H