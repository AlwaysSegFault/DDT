# DDT
The best videogame for last 1000 years

## Установка Raylib с помощью автоматизированного скрипта


### Как использовать


1. Дайте права на выполнение скрипта:
    ```bash
    chmod +x install_raylib.sh
    ```

2. Запустите скрипт:
    ```bash
    ./install_raylib.sh
    ```

### Что делает этот скрипт?

- Обновляет список пакетов и обновляет установленные пакеты до актуальных версий.
- Клонирует репозиторий Raylib из официального GitHub.
- Компилирует и устанавливает Raylib в систему.
- Очищает временные файлы после установки.


## Работа с Makefile

1.  Компиляция всех .c файлов в директории
    ```bash
    make
    ```

2.  Компиляция конкретного файла:
    ```bash
    make SRC=file_name.c
    ```
    После этого он скомпилируе file_name.c в file_name
    

Directed by Askhabov Deni
Producer by Supaev Viskhan
GitMaster by Khartsaev Muslim 
Designer by Mustafaev Zelimkhan 
