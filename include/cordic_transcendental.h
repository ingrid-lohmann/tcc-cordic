#ifndef CORDIC_TRANSCENDENTAL_H
#define CORDIC_TRANSCENDENTAL_H

#include "cordic_types.h"

/* Funções transcendentais compostas */
double cordic_exp(double x, int iterations);
double cordic_ln(double x, int iterations);
double cordic_sqrt(double x, int iterations);
double cordic_pow(double base, double exp, int iterations);

#endif