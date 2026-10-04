# Relatório de Casos de Borda e Conformidade IEEE 754

| Categoria | Teste / Entrada | Obtido | Esperado | Diferença Abs. | Status |
| :--- | :--- | :--- | :--- | :---: | :---: |
| Zero/Identidade | `cordic_circular_sin(0.0)` | 0.000000000 | 0.000000000 | 1.30e-10 | **PASS** |
| Zero/Identidade | `cordic_circular_cos(0.0)` | 1.000000000 | 1.000000000 | 5.55e-16 | **PASS** |
| Zero/Identidade | `cordic_circular_tan(0.0)` | 0.000000000 | 0.000000000 | 1.30e-10 | **PASS** |
| Zero/Identidade | `cordic_hyperbolic_sinh(0.0)` | -0.000000000 | 0.000000000 | 1.64e-10 | **PASS** |
| Zero/Identidade | `cordic_hyperbolic_cosh(0.0)` | 1.000000000 | 1.000000000 | 2.22e-16 | **PASS** |
| Zero/Identidade | `cordic_exp(0.0)` | 1.000000000 | 1.000000000 | 0.00e+00 | **PASS** |
| Zero/Identidade | `cordic_ln(1.0)` | 0.000000000 | 0.000000000 | 0.00e+00 | **PASS** |
| Zero/Identidade | `cordic_sqrt(0.0)` | 0.000000000 | 0.000000000 | 0.00e+00 | **PASS** |
| Dominio Invalido | `cordic_sqrt(-1.0)` | NaN | NaN | 0.00e+00 | **PASS** |
| Dominio Invalido | `cordic_sqrt(-100.0)` | NaN | NaN | 0.00e+00 | **PASS** |
| Dominio Invalido | `cordic_ln(-2.0)` | NaN | NaN | 0.00e+00 | **PASS** |
| Dominio Invalido | `cordic_ln(0.0)` | -Inf | -Infinity | 0.00e+00 | **PASS** |
| Entrada Especial | `cordic_circular_sin(NAN)` | NaN | NaN | 0.00e+00 | **PASS** |
| Entrada Especial | `cordic_circular_cos(NAN)` | NaN | NaN | 0.00e+00 | **PASS** |
| Entrada Especial | `cordic_sqrt(NAN)` | NaN | NaN | 0.00e+00 | **PASS** |
| Entrada Especial | `cordic_ln(NAN)` | NaN | NaN | 0.00e+00 | **PASS** |
| Simetria/Quadrante | `cordic_circular_sin(-pi/4)` | -0.707106781 | -0.707106781 | 6.82e-11 | **PASS** |
| Simetria/Quadrante | `cordic_circular_cos(-pi/4)` | 0.707106781 | 0.707106781 | 6.82e-11 | **PASS** |
| Simetria/Quadrante | `cordic_circular_sin(3*pi)` | -0.000000000 | 0.000000000 | 1.30e-10 | **PASS** |
| Simetria/Quadrante | `cordic_circular_cos(3*pi)` | -1.000000000 | -1.000000000 | 5.55e-16 | **PASS** |
| Circular | `cordic_circular_atan(0.5)` | 0.463647609 | 0.463647609 | 2.04e-10 | **PASS** |
| Motor Circular | `rotate.x` | 0.707106781 | 0.707106781 | 6.82e-11 | **PASS** |
| Motor Circular | `rotate.y` | 0.707106781 | 0.707106781 | 6.82e-11 | **PASS** |
| Motor Circular | `rotate.z` | -0.000000000 | 0.000000000 | 9.65e-11 | **PASS** |
| Motor Circular | `vector.x` | 1.841133940 | 1.841133940 | 8.88e-16 | **PASS** |
| Motor Circular | `vector.y` | 0.000000000 | 0.000000000 | 3.75e-10 | **PASS** |
| Motor Circular | `vector.z` | 0.463647609 | 0.463647609 | 2.04e-10 | **PASS** |
| Hiperbolica | `cordic_hyperbolic_tanh(0.5)` | 0.462117157 | 0.462117157 | 1.28e-10 | **PASS** |
| Hiperbolica | `cordic_hyperbolic_atanh(1/3)` | 0.346573590 | 0.346573590 | 1.57e-10 | **PASS** |
| Dominio Invalido | `cordic_hyperbolic_atanh(1.0)` | NaN | NaN | 0.00e+00 | **PASS** |
| Motor Hiperbolico | `rotate.x` | 1.127625965 | 1.127625965 | 8.49e-11 | **PASS** |
| Motor Hiperbolico | `rotate.y` | 0.521095306 | 0.521095305 | 1.84e-10 | **PASS** |
| Motor Hiperbolico | `rotate.z` | -0.000000000 | 0.000000000 | 1.63e-10 | **PASS** |
| Motor Hiperbolico | `vector.x` | 0.780796133 | 0.780796133 | 0.00e+00 | **PASS** |
| Motor Hiperbolico | `vector.y` | 0.000000000 | 0.000000000 | 1.22e-10 | **PASS** |
| Motor Hiperbolico | `vector.z` | 0.346573590 | 0.346573590 | 1.57e-10 | **PASS** |
| Transcendental | `cordic_pow(1.5, 1.25)` | 1.660022879 | 1.660022880 | 4.24e-10 | **PASS** |
| Dominio Invalido | `cordic_pow(-1.0, 2.0)` | NaN | NaN | 0.00e+00 | **PASS** |

**Resumo**: 38/38 testes passaram (taxa de sucesso: 100.0%).
