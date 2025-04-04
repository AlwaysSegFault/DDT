
#!/bin/bash

# Проверка, установлен ли Raylib
if [ -f "/usr/local/include/raylib.h" ] && [ -f "/usr/local/lib/libraylib.a" ]; then
    echo "Raylib уже установлен на вашем компьютере!"
    read -p "Хотите переустановить Raylib? (y/n): " choice
    if [ "$choice" != "y" ]; then
        echo "Установка отменена."
        exit 0
    fi
fi


echo "Installing the make and CMake"
sudo apt install make
sudo apt install cmake

# Обновляем индексы пакетов
echo "Updating package list..."
sudo apt update && \
# Обновляем установленные пакеты до актуальных версий
echo "Upgrading installed packages..." && \
sudo apt upgrade -y && \
# Устанавливаем все необходимые зависимости
echo "Installing dependencies..." && \
sudo apt install -y build-essential cmake pkg-config libssl-dev libboost-all-dev libcurl4-openssl-dev \
libxrandr-dev libglfw3-dev libglew-dev libx11-dev libxft-dev libxext-dev libgl1-mesa-dev \
libxinerama-dev libxi-dev libxcursor-dev libsndfile1-dev libfreetype6-dev libopenal-dev \
libjpeg-dev libpng-dev libtiff-dev libxmu-dev libxpm-dev libeigen3-dev libboost-filesystem-dev \
libboost-system-dev libboost-thread-dev libasound2-dev libudev-dev libwayland-dev libxcomposite-dev \
libxdamage-dev libegl1-mesa-dev libgbm-dev libdrm-dev && \
# Загружаем репозиторий Raylib в текущую директорию
echo "Cloning the Raylib repository into the current directory" && \
git clone https://github.com/raysan5/raylib.git && \
# Проверка успешного клонирования репозитория
if [ ! -d "raylib" ]; then
    echo "Ошибка клонирования репозитория. Пожалуйста, проверьте соединение с интернетом."
    exit 1
fi && \
# Переходим в директорию с репозиторием
cd raylib && \
# Создаем директорию для сборки и переходим в нее
echo "Building the library" && \
mkdir build && cd build && \
# Запуск CMake для подготовки сборки
cmake .. && \
# Проверка, прошла ли настройка CMake без ошибок
if [ $? -ne 0 ]; then
    echo "Ошибка при конфигурации проекта. Пожалуйста, проверьте вывод CMake для деталей."
    exit 1
fi && \
# Компиляция библиотеки
make && \
# Проверка успешности сборки
if [ $? -ne 0 ]; then
    echo "Ошибка при сборке проекта. Пожалуйста, проверьте вывод make для деталей."
    exit 1
fi && \
# Устанавливаем библиотеку в систему
echo "Installing the library into the system" && \
sudo make install && \
# Проверка успешности установки
if [ $? -ne 0 ]; then
    echo "Ошибка при установке библиотеки. Пожалуйста, проверьте вывод для деталей."
    exit 1
fi && \
# Удаляем директорию с исходниками (если нужно)
echo "Cleaning up..." && \
cd ../.. && \
rm -rf raylib && \
echo "Raylib installation completed successfully!"