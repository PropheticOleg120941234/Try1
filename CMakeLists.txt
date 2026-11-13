cmake_minimum_required(VERSION 3.16)
project(Try1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Магия Qt для обработки макроса Q_OBJECT
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Находим нужные модули Qt
find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets)

# Перечисляем ВСЕ файлы вашего проекта
add_executable(Try1
    main.cpp
    EngineViewport.h
    EngineViewport.cpp
)

# Связываем вашу программу с модулями Qt
target_link_libraries(Try1 PRIVATE
    Qt6::Widgets
    Qt6::OpenGLWidgets
)