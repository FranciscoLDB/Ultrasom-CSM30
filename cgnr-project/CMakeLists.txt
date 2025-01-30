cmake_minimum_required(VERSION 3.10)

# Nome do projeto
project(UltrasomCSM30)

# Definir a versão do C++
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Adicionar o executável
add_executable(UltrasomCSM30 cgnr.cpp)

# Incluir diretórios
add_subdirectory(${PROJECT_SOURCE_DIR}/utils)

# Adicionar bibliotecas necessárias
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(UltrasomCSM30 ${OpenCV_LIBS})