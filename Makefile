CC = gcc
CFLAGS = -I/usr/local/include -L/usr/local/lib -lraylib -lm -lpthread -ldl -lrt -lX11

# Если SRC не задан, компилируем все *.c файлы
SRC = $(if $(SRC),$(SRC),$(wildcard *.c))
OUT = $(if $(SRC),$(basename $(SRC)),$(basename $(wildcard *.c)))

# Компиляция и линковка в одном шаге
$(OUT): $(SRC)
	$(CC) $(SRC) -o $(OUT) $(CFLAGS)

# Очистка
clean:
	rm -f $(OUT)

# По умолчанию выполнить компиляцию
all: $(OUT)
