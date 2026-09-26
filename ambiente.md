echo "=================================================="
echo "METADADOS DO AMBIENTE EXPERIMENTAL (CODESPACE)"
echo "=================================================="
echo -n "Sistema Operacional: "
lsb_release -ds 2>/dev/null || cat /etc/*release | grep PRETTY_NAME | cut -d= -f2 | tr -d '"'
echo -n "Kernel Linux: "
uname -r
echo -n "Arquitetura: "
uname -m
echo -n "Modelo de CPU: "
lscpu | grep "Model name" | sed 's/Model name:[ \t]*//'
echo -n "Núcleos Lógicos (vCPUs): "
nproc
echo -n "Frequência da CPU (MHz): "
lscpu | grep "CPU MHz" | sed 's/CPU MHz:[ \t]*//'
echo "Caches de Memória:"
lscpu | grep -E "L1d|L1i|L2|L3" | sed 's/^[ \t]*/  - /'
echo -n "Memória RAM Total: "
free -h | awk '/^Mem:/ {print $2}'
echo -n "Versão do Compilador (GCC): "
gcc --version | head -n 1
echo -n "Versão da GLIBC: "
ldd --version | head -n 1 | awk '{print $NF}'
echo "=================================================="


echo "--- Detalhes da CPU ---"
grep "model name" /proc/cpuinfo | head -n 1 | sed 's/model name[ \t]*: //'
echo -n "Frequência nominal: "
grep "cpu MHz" /proc/cpuinfo | head -n 1 | sed 's/cpu MHz[ \t]*: //'
echo -n "Tamanho de Cache L3 (se reportado): "
grep "cache size" /proc/cpuinfo | head -n 1 | sed 's/cache size[ \t]*: //'
echo -n "Flags de vetorização relevantes: "
grep "flags" /proc/cpuinfo | head -n 1 | grep -o -E "(sse2|avx|avx2|fma)" | sort -u | tr '\n' ' '
echo ""

++++++++ RESULTADO ++++++++++++
==================================================
METADADOS DO AMBIENTE EXPERIMENTAL (CODESPACE)
==================================================
Sistema Operacional: Alpine Linux v3.23
Kernel Linux: 6.8.0-1064-azure
Arquitetura: x86_64
Modelo de CPU: bash: lscpu: command not found
Núcleos Lógicos (vCPUs): 2
Frequência da CPU (MHz): bash: lscpu: command not found
Caches de Memória:
bash: lscpu: command not found
Memória RAM Total: 7.8Gi
Versão do Compilador (GCC): gcc (Alpine 15.2.0) 15.2.0
Versão da GLIBC: musl libc (x86_64)
Version 1.2.5
Dynamic Program Loader
Usage: /lib/ld-musl-x86_64.so.1 [options] [--] pathname
==================================================


--- Detalhes da CPU ---
AMD EPYC 9V74 80-Core Processor
Frequência nominal: 2871.294
Tamanho de Cache L3 (se reportado): 1024 KB
Flags de vetorização relevantes: avx avx2 fma sse2 