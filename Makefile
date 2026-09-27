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

# 📦 Arquivos fonte (excluindo main.c para compilar com testes)
SRCS = $(wildcard $(SRC_DIR)/*.c)
CORE_SRCS = $(filter-out $(SRC_DIR)/main.c, $(SRCS))

# 📐 Tabelas e gerador
LUT_HEADER = $(INC_DIR)/cordic_lut.h
TABLE_GEN_SRC = $(TOOLS_DIR)/build_cordic_table.c
TABLE_GEN_BIN = $(BUILD_DIR)/table_generator

# 🎯 Binários
TARGET = $(BUILD_DIR)/main
EDGE_BIN = $(BUILD_DIR)/test_edge_cases
STRESS_BIN = $(BUILD_DIR)/stress_test
CONV_BIN = $(BUILD_DIR)/cordic_convergence
BENCH_BIN = $(BUILD_DIR)/benchmark_latency

# 🔄 Regra padrão
all: $(LUT_HEADER) $(TARGET)

# 📐 Gerador de tabelas
$(LUT_HEADER): $(TABLE_GEN_SRC)
	@mkdir -p $(BUILD_DIR)
	@echo "--- Compilando e executando o gerador de tabelas ---"
	$(CC) $(TABLE_GEN_SRC) -o $(TABLE_GEN_BIN) $(LDFLAGS)
	./$(TABLE_GEN_BIN)

# 🔨 Executável principal
$(TARGET): $(SRCS) $(LUT_HEADER)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

# ▶️ Executar main
run: all
	@echo "--- Executando $(TARGET) ---"
	./$(TARGET)

# 🧪 Regras de compilação dos binários de teste
$(EDGE_BIN): $(CORE_SRCS) $(TEST_DIR)/test_edge_cases.c $(LUT_HEADER)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CORE_SRCS) $(TEST_DIR)/test_edge_cases.c -o $(EDGE_BIN) $(LDFLAGS)

$(STRESS_BIN): $(CORE_SRCS) $(TEST_DIR)/stress_test.c $(LUT_HEADER)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CORE_SRCS) $(TEST_DIR)/stress_test.c -o $(STRESS_BIN) $(LDFLAGS)

$(CONV_BIN): $(CORE_SRCS) $(TEST_DIR)/cordic_convergence.c $(LUT_HEADER)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CORE_SRCS) $(TEST_DIR)/cordic_convergence.c -o $(CONV_BIN) $(LDFLAGS)

$(BENCH_BIN): $(CORE_SRCS) $(TEST_DIR)/benchmark_latency.c $(LUT_HEADER)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CORE_SRCS) $(TEST_DIR)/benchmark_latency.c -o $(BENCH_BIN) $(LDFLAGS)

# 🎯 Compilar todos os testes de uma vez
tests: $(EDGE_BIN) $(STRESS_BIN) $(CONV_BIN) $(BENCH_BIN)

# ▶️ Alvos para execução individual
edge: $(EDGE_BIN)
	@echo "--- Executando Testes de Casos de Borda ---"
	./$(EDGE_BIN)

stress: $(STRESS_BIN)
	@echo "--- Executando Teste de Estresse ---"
	./$(STRESS_BIN)

convergence: $(CONV_BIN)
	@echo "--- Executando Teste de Convergência ---"
	./$(CONV_BIN)

bench: $(BENCH_BIN)
	@echo "--- Executando Benchmark de Latência ---"
	./$(BENCH_BIN)

# ▶️ Executa toda a bateria experimental sequencialmente
run-all-tests: edge stress convergence bench

# 🧪 Testes via Unity (caso use a suíte completa)
test: $(LUT_HEADER)
	$(CC) $(CFLAGS) -DTEST \
	$(CORE_SRCS) \
	$(wildcard $(TEST_DIR)/test_*.c) \
	$(UNITY_DIR)/unity.c \
	-o $(BUILD_DIR)/test_runner $(LDFLAGS)
	./$(BUILD_DIR)/test_runner

# 🐞 Debug
debug: CFLAGS += $(DEBUG_FLAGS)
debug: clean all

# 🧹 Limpeza
clean:
	rm -rf $(BUILD_DIR)

# 🔁 Rebuild
re: clean all

.PHONY: all run tests edge stress convergence bench run-all-tests test debug clean re