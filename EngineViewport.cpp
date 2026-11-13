#include "EngineViewport.h"
#include <QMatrix4x4>
#include <algorithm>

// Исходный код вершинного шейдера (задает координаты)
const char* vertexShaderSource = R"(
    #version 410 core
    layout (location = 0) in vec3 aPos;
    uniform mat4 transform;
    void main() {
        gl_Position = transform * vec4(aPos, 1.0);
    }
)";

// Исходный код фрагментного шейдера (задает цвет пикселей)
const char* fragmentShaderSource = R"(
    #version 410 core
    out vec4 FragColor;
    void main() {
        FragColor = vec4(1.0, 0.5, 0.2, 1.0); // Оранжевый цвет (RGBA)
    }
)";

EngineViewport::EngineViewport(QWidget* parent) : QOpenGLWidget(parent) {
    setFocusPolicy(Qt::StrongFocus); // Разрешаем виджету принимать фокус клавиатуры
    setMouseTracking(true);          // Ловим движения мыши, даже если кнопки не зажаты
    timer.start();
}

EngineViewport::~EngineViewport() {
    makeCurrent(); // Активируем контекст OpenGL перед удалением ресурсов из GPU
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    delete shaderProgram;
    doneCurrent();
}

void EngineViewport::initializeGL() {
    initializeOpenGLFunctions(); // Связываем функции OpenGL с драйвером ОС
    glEnable(GL_DEPTH_TEST);     // Включаем Z-буфер глубины

    // 1. Создаем и компилируем шейдерную программу
    shaderProgram = new QOpenGLShaderProgram(this);
    shaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    shaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    shaderProgram->link();

    // 2. Координаты вершин треугольника (X, Y, Z) в пределах от -1.0 до 1.0
    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,
        0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f, -0.5f, -0.5f,

        -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f,
        0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f, -0.5f,  0.5f,

        -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f, -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,  0.5f,

        0.5f,  0.5f,  0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f,

        -0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,
        0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f, -0.5f,

        -0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,
        0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f
    };

    // 3. Выделяем память на видеокарте и настраиваем "паспорт" VAO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Объясняем видеокарте структуру байт: 0-й атрибут, 3 float на вершину
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    timer.restart();

    frameTimer = new QTimer(this);
    // Связываем сигнал таймера с нашим игровым циклом gameLoop
    connect(frameTimer, &QTimer::timeout, this, &EngineViewport::gameLoop);

    // Заставляем таймер срабатывать как можно чаще (0 мс означает "вызывать при любой свободной итерации")
    // Это дает максимальный FPS, но управление временем теперь полностью наше
    frameTimer->start(0);
}

void EngineViewport::resizeGL(int w, int h) {
    glViewport(0, 0, w, h); // Обновляем область отрисовки при изменении размера окна
}

void EngineViewport::paintGL() {
    renderScene(accumulator / FIXED_DT);
}

void EngineViewport::updatePhysics(double dt) {
    // Вращаем треугольник со скоростью 90 градусов в секунду строго в физическом тике
    currentAngle += rotationSpeed * static_cast<float>(dt);

    // СЧИТАЕМ НАПРАВЛЕНИЕ ВЗГЛЯДА КАМЕРЫ (Математика из Категории 1)
    // Переводим углы Эйлера в трехмерный вектор направления (Front)
    QVector3D front;
    front.setX(cos(qDegreesToRadians(cameraYaw)) * cos(qDegreesToRadians(cameraPitch)));
    front.setY(sin(qDegreesToRadians(cameraPitch)));
    front.setZ(sin(qDegreesToRadians(cameraYaw)) * cos(qDegreesToRadians(cameraPitch)));
    front.normalize();

    float cameraSpeed = 2.5f * static_cast<float>(dt); // Скорость полета

    // Движение вперед-назад
    if (keys[Qt::Key_W]) cameraPos += front * cameraSpeed;
    if (keys[Qt::Key_S]) cameraPos -= front * cameraSpeed;

    // Движение влево-вправо (используем векторное произведение с вектором "Верх" для шага вбок)
    QVector3D right = QVector3D::crossProduct(front, QVector3D(0.0f, 1.0f, 0.0f)).normalized();
    if (keys[Qt::Key_A]) cameraPos -= right * cameraSpeed;
    if (keys[Qt::Key_D]) cameraPos += right * cameraSpeed;
}

void EngineViewport::renderScene(double alpha) {
    glClearColor(0.15f, 0.15f, 0.18f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float renderAngle = prevAngle * (1.0f - alpha) + currentAngle * alpha;

    // 1. МАТРИЦА МОДЕЛИ: Куб просто стоит в центре мира (0,0,0) и крутится
    QMatrix4x4 model;
    model.setToIdentity();
    model.rotate(renderAngle, 0.5f, 1.0f, 0.0f);

    // 2. МАТРИЦА ВИДА (НАША КАМЕРА): Рассчитываем взгляд (Категория 1)
    QMatrix4x4 view;
    view.setToIdentity();

    // Пересчитываем вектор взгляда для текущего кадра рендера
    QVector3D front;
    front.setX(cos(qDegreesToRadians(cameraYaw)) * cos(qDegreesToRadians(cameraPitch)));
    front.setY(sin(qDegreesToRadians(cameraPitch)));
    front.setZ(sin(qDegreesToRadians(cameraYaw)) * cos(qDegreesToRadians(cameraPitch)));
    front.normalize();

    // Полезный метод Qt: принимает (Где камера, Куда смотрит, Где верх)
    // Сдвигаем цель взгляда на -3.5 по Z изначально, чтобы куб оказался перед нами
    view.lookAt(cameraPos + QVector3D(0.0f, 0.0f, 3.5f), cameraPos + QVector3D(0.0f, 0.0f, 3.5f) + front, QVector3D(0.0f, 1.0f, 0.0f));

    // 3. МАТРИЦА ПРОЕКЦИИ
    QMatrix4x4 projection;
    projection.setToIdentity();
    float aspectRatio = static_cast<float>(width()) / std::max(1, height());
    projection.perspective(45.0f, aspectRatio, 0.1f, 100.0f);

    // Собираем конвейер
    QMatrix4x4 transform = projection * view * model;

    shaderProgram->bind();
    shaderProgram->setUniformValue("transform", transform);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void EngineViewport::gameLoop() {
    // 1. Считаем чистую разницу времени
    double rawFrameTime = timer.restart() / 1000.0;
    if (rawFrameTime > 0.25) rawFrameTime = FIXED_DT;

    accumulator += rawFrameTime;

    // 2. Стабильный шаг физики
    while (accumulator >= FIXED_DT) {
        prevAngle = currentAngle;
        updatePhysics(FIXED_DT);
        accumulator -= FIXED_DT;
    }

    // 3. Принудительно заставляем Qt вызвать paintGL прямо сейчас, без откладывания в долгий ящик
    update();
}

void EngineViewport::setRotationSpeed(int speed) {
    this->rotationSpeed = static_cast<float>(speed);
}

void EngineViewport::setFarPlane(double farValue) {
    this->farPlane = static_cast<float>(farValue);
}

void EngineViewport::keyPressEvent(QKeyEvent* event) {
    if (event->key() >= 0 && event->key() < 1024) {
        keys[event->key()] = true; // Запоминаем, что клавиша зажата
    }
}

void EngineViewport::keyReleaseEvent(QKeyEvent* event) {
    if (event->key() >= 0 && event->key() < 1024) {
        keys[event->key()] = false; // Клавиша отпущена
    }
}

void EngineViewport::mouseMoveEvent(QMouseEvent* event) {
    // Если это первый кадр — просто запоминаем позицию мыши без рывка камеры
    if (firstMouse) {
        lastMousePos = event->pos();
        firstMouse = false;
        return;
    }

    // Считаем разницу, на сколько пикселей сдвинулась мышь
    float xOffset = event->pos().x() - lastMousePos.x();
    float yOffset = lastMousePos.y() - event->pos().y(); // Переворачиваем Y, так как в окнах он идет сверху вниз
    lastMousePos = event->pos();

    float sensitivity = 0.1f; // Чувствительность мыши
    xOffset *= sensitivity;
    yOffset *= sensitivity;

    cameraYaw   += xOffset;
    cameraPitch += yOffset;

    // Ограничиваем наклон вверх-вниз, чтобы не перевернуться вниз головой
    if (cameraPitch > 89.0f)  cameraPitch = 89.0f;
    if (cameraPitch < -89.0f) cameraPitch = -89.0f;
}

