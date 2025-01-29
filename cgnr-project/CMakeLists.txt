cmake_minimum_required(VERSION 3.10)

# Define o nome do projeto
project(CGNR_Project)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Define o padrão de compilação
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Adiciona o diretório src
add_subdirectory(src)

# Instruções para instalação
install(TARGETS cgnr DESTINATION bin)