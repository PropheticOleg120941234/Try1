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
    timer.start(); // Запускаем высокоточный таймер
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
}

void EngineViewport::resizeGL(int w, int h) {
    glViewport(0, 0, w, h); // Обновляем область отрисовки при изменении размера окна
}

void EngineViewport::paintGL() {
    // 1. Получаем чистые миллисекунды от таймера
    double rawFrameTime = timer.restart() / 1000.0;

    // Защита от слишком больших скачков (например, если свернули окно)
    if (rawFrameTime > 0.25) {
        rawFrameTime = FIXED_DT;
    }

    // 2. ФИЛЬТР СГЛАЖИВАНИЯ (Экспоненциальное скользящее среднее)
    // Мы берем 90% времени от прошлого кадра и только 10% от нового текущего.
    // Это сглаживает случайные микро-задержки операционной системы.
    static double smoothedFrameTime = FIXED_DT;
    smoothedFrameTime = smoothedFrameTime * 0.9 + rawFrameTime * 0.1;

    accumulator += smoothedFrameTime;

    // 3. Фиксированный шаг для физики
    while (accumulator >= FIXED_DT) {
        prevAngle = currentAngle;
        updatePhysics(FIXED_DT);
        accumulator -= FIXED_DT;
    }

    // 4. Отрисовка с правильным коэффициентом сглаживания
    //double alpha = accumulator / FIXED_DT;
    renderScene(accumulator / FIXED_DT);

    update(); // Запрашиваем перерисовку
}

void EngineViewport::updatePhysics(double dt) {
    // Вращаем треугольник со скоростью 90 градусов в секунду строго в физическом тике
    currentAngle += 90.0f * static_cast<float>(dt);
    if (currentAngle >= 360.0f) {
        currentAngle -= 360.0f;
    }
}

void EngineViewport::renderScene(double alpha) {
    // Очищаем экран
    glClearColor(0.15f, 0.15f, 0.18f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //glDisable(GL_CULL_FACE);

    // Плавный угол рендера с учетом интерполяции
    float renderAngle = prevAngle * (1.0f - alpha) + currentAngle * alpha;

    // Создаем матрицу трансформации (поворот вокруг оси Z)
    QMatrix4x4 model;
    model.setToIdentity();
    model.translate(0.0f, 0.0f, -3.5f); // Отодвигаем чуть дальше (на -3), чтобы он точно влез в камеру
    model.rotate(renderAngle, 0.5f, 1.0f, 0.0f); // Вращаем вокруг вертикальной оси Y (в 3D!)

    QMatrix4x4 view; // матрица взгляда
    view.setToIdentity();

    QMatrix4x4 projection; // матрица проекции (оптика и перспектива, нпр дальность объекта)
    projection.setToIdentity();

    float aspectRatio = static_cast<float>(width()) / std::max(1, height());
    projection.perspective(45.0f, aspectRatio, 0.1f, 100.0f);

    QMatrix4x4 transform = projection * view * model;

    // Активируем шейдеры и передаем матрицу на видеокарту
    shaderProgram->bind();
    shaderProgram->setUniformValue("transform", transform);

    // Рисуем геометрию
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}