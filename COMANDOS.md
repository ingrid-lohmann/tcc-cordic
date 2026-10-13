# Comandos do Makefile

Execute os comandos na raiz do projeto. O Makefile usa `gcc` e define `ITERATIONS=24` por padrão. Para informar outra quantidade de iterações, use `ITERATIONS=N`, com um inteiro entre 1 e 32.

## Compilar e executar

| Comando | O que faz |
| --- | --- |
| `make` ou `make all` | Valida `ITERATIONS`, gera/atualiza a tabela LUT e compila o executável principal em `build/main`. |
| `make run` | Compila o projeto, se necessário, e executa `build/main`. |
| `make ITERATIONS=32` | Compila o alvo padrão usando 32 iterações. |
| `make run ITERATIONS=16` | Compila e executa o programa usando 16 iterações. |

## Testes experimentais

| Comando | O que faz |
| --- | --- |
| `make tests` | Compila todos os binários de teste, sem executá-los. |
| `make edge` | Compila e executa os testes de casos de borda. |
| `make stress` | Compila e executa o teste de estresse. |
| `make convergence` | Compila e executa o teste de convergência. |
| `make bench` | Compila e executa o benchmark de latência. |
| `make run-all-tests` | Executa sequencialmente `edge`, `stress`, `convergence` e `bench`. |
| `make edge ITERATIONS=32` | Executa os testes de casos de borda com 32 iterações. O mesmo parâmetro pode ser usado nos outros alvos de teste. |

## Suíte Unity

| Comando | O que faz |
| --- | --- |
| `make test` | Compila e executa os arquivos `tests/test_*.c` junto com `tests/unity/unity.c`. Requer que os arquivos da Unity estejam disponíveis nesse diretório. |

## Manutenção

| Comando | O que faz |
| --- | --- |
| `make debug` | Remove `build/` e recompila o alvo padrão com símbolos de depuração (`-g`). |
| `make re` | Remove `build/` e recompila o alvo padrão. |
| `make clean` | Remove o diretório `build/`. |
| `make check-iterations ITERATIONS=24` | Verifica se o valor de `ITERATIONS` está entre 1 e 32. |

`make run-main` não é um alvo definido neste Makefile; use `make run` para executar o programa principal.