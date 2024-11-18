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

# Обновляем индексы пакетов
echo "Updating package list..."
sudo apt update

# Обновляем установленные пакеты до актуальных версий
echo "Upgrading installed packages..."
sudo apt upgrade -y

# Создаем временную папку для установки Raylib
TEMP_DIR=$(mktemp -d)

echo "Cloning the Raylib repository into $TEMP_DIR"
git clone https://github.com/raysan5/raylib.git "$TEMP_DIR/raylib"

echo "Building the library"
cd "$TEMP_DIR/raylib"
mkdir build && cd build
cmake ..
make

echo "Installing the library into the system"
sudo make install

# Удаляем временную папку
echo "Cleaning up..."
rm -rf "$TEMP_DIR"

echo "Raylib installation completed successfully!"