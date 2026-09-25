gcc main.c -o main
./main


❗ Se der erro no gcc
Rode:
sudo apt update
sudo apt install build-essential -y

✔ Compilar tudo
gcc *.c -o main
./main

make run-main ARGS=ieee_utils.c