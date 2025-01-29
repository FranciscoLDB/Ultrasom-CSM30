### Atualizar os pacotes:
sudo apt-get update

### Instalar as dependências necessárias:
sudo apt-get install build-essential cmake git
sudo apt-get install libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev
sudo apt-get install python3.6-dev python3-numpy libtbb2 libtbb-dev
sudo apt-get install libjpeg-dev libpng-dev libtiff-dev libdc1394-22-dev

### Baixar o código-fonte do OpenCV:
cd ~
git clone https://github.com/opencv/opencv.git
git clone https://github.com/opencv/opencv_contrib.git

### Compilar e instalar o OpenCV:
cd ~/opencv
mkdir build
cd build
cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=/usr/local -D OPENCV_EXTRA_MODULES_PATH=~/opencv_contrib/modules ..
make -j8
sudo make install
sudo ldconfig

### Verificar a instalação:
pkg-config --modversion opencv4