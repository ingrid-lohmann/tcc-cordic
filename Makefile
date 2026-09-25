# Makefile simples
# all:
#   gcc *.c -o main

# run:
#   ./main

# ----------

# 📁 Diretórios
SRC_DIR = src
BUILD_DIR = build
INC_DIR = include
TEST_DIR = tests
TOOLS_DIR = tools
UNITY_DIR = $(TEST_DIR)/unity

# ⚙️ Compilador e flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -I$(INC_DIR)
LDFLAGS = -lm
DEBUG_FLAGS = -g

# 📦 Arquivos fonte e tabelas
SRCS = $(wildcard $(SRC_DIR)/*.c)
LUT_HEADER = $(INC_DIR)/cordic_lut.h
TABLE_GEN_SRC = $(TOOLS_DIR)/build_cordic_table.c
TABLE_GEN_BIN = $(BUILD_DIR)/table_generator

# 🎯 Nome do executável final unificado
TARGET = $(BUILD_DIR)/main

# 🔄 Regra padrão (Gera a tabela primeiro, depois compila o pacote)
all: $(LUT_HEADER) $(TARGET)

# 📐 Regra para gerar o arquivo de tabelas automaticamente
$(LUT_HEADER): $(TABLE_GEN_SRC)
	@mkdir -p $(BUILD_DIR)
	@echo "--- Compilando e executando o gerador de tabelas ---"
	$(CC) $(TABLE_GEN_SRC) -o $(TABLE_GEN_BIN) $(LDFLAGS)
	./$(TABLE_GEN_BIN)

# 🔨 Regra de compilação unificada (junta todos os .c de uma vez)
$(TARGET): $(SRCS) $(LUT_HEADER)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

# ▶️ Rodar o executável principal diretamente
run: all
	@echo "--- Executando $(TARGET) ---"
	./$(TARGET)

# 🧪 Testes
test: $(LUT_HEADER)
	$(CC) $(CFLAGS) -DTEST \
	$(filter-out $(SRC_DIR)/main.c, $(SRC_DIR)/*.c) \
	$(TEST_DIR)/test_*.c \
	$(UNITY_DIR)/unity.c \
	-o test_runner $(LDFLAGS)
	./test_runner

# 🐞 Debug
debug: CFLAGS += $(DEBUG_FLAGS)
debug: clean all

# 🧹 Limpeza
clean:
	rm -rf $(BUILD_DIR)
	rm -f test_runner

# 🔁 Rebuild
re: clean all