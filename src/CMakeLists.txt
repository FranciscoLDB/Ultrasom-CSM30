cmake_minimum_required(VERSION 3.10)

# Nome do projeto
project(UltrasomCSM30)

# Definir o padrão de compilação
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Adicionar diretórios de inclusão
include_directories(${PROJECT_SOURCE_DIR}/include)

# Adicionar os arquivos fonte
set(SOURCES
    ${PROJECT_SOURCE_DIR}/src/cgnr.cpp
    ${PROJECT_SOURCE_DIR}/src/readData.cpp
)

# Adicionar o executável
add_executable(UltrasomCSM30 ${SOURCES})

# Encontrar e adicionar bibliotecas necessárias
find_package(OpenCV REQUIRED)
target_link_libraries(UltrasomCSM30 ${OpenCV_LIBS})