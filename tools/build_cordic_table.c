#include <stdio.h>
#include <math.h>

#ifndef CORDIC_ITERATIONS
#define CORDIC_ITERATIONS 24
#endif

double precalculate_circular_gain(int n, FILE *log_file) {
    double an_gain = 1.0;
    for (int i = 0; i < n; i++) {
        double current_term = sqrt(1.0 + pow(4.0, -i));
        an_gain *= current_term;
        double gain = 1.0 / an_gain;
        printf("iteração %d: %.15f\n", i, gain);
        fprintf(log_file, "iteração %d: %.15f\n", i+1, gain);
    }
    return 1.0 / an_gain;
}

double precalculate_hyperbolic_gain(int n, FILE *log_file) {
    double an_gain = 1.0;
    for (int i = 1; i <= n; i++) {
        double current_term = sqrt(1.0 - pow(4.0, -i));
        an_gain *= current_term;
        if (i == 4 || i == 13) {
            current_term = sqrt(1.0 - pow(4.0, -i));
            an_gain *= current_term;
        }
        double gain = 1.0 / an_gain;
        printf("iteração %d: %.15f\n", i, gain);
        fprintf(log_file, "iteração %d: %.15f\n", i, gain);
    }
    return 1.0 / an_gain;
}

int main() {
    FILE *log_file = fopen("resultados/ganhos_cordic.md", "w");
    if (log_file == NULL) {
        printf("Fatal error: Could not create file resultados/ganhos_cordic.md\n");
        return 1;
    }

    FILE *output_file = fopen("include/cordic_lut.h", "w");
    
    if (output_file == NULL) {
        printf("Fatal error: Could not create file cordic_lut.h\n");
        fclose(log_file);
        return 1; 
    }

    fprintf(log_file, "# Ganho CORDIC circular\n\n");
    double k_circular = precalculate_circular_gain(CORDIC_ITERATIONS, log_file);
    fprintf(log_file, "\n# Ganho CORDIC hiperbólico\n\n");
    double k_hyperbolic = precalculate_hyperbolic_gain(CORDIC_ITERATIONS, log_file);

    fprintf(output_file, "#ifndef CORDIC_LUT_H\n");
    fprintf(output_file, "#define CORDIC_LUT_H\n\n");

    fprintf(output_file, "#define CORDIC_K_CIRCULAR %.15f\n", k_circular);
    fprintf(output_file, "#define CORDIC_K_HYPERBOLIC %.15f\n\n", k_hyperbolic);

    fprintf(output_file, "static const double CORDIC_CIRCULAR_LUT[%d] = {\n", CORDIC_ITERATIONS);
    for (int i = 0; i < CORDIC_ITERATIONS; i++) {
        double lut_value = atan(pow(2.0, -i));
        if (i < CORDIC_ITERATIONS - 1) {
            fprintf(output_file, "    %.15f,\n", lut_value);
        } else {
            fprintf(output_file, "    %.15f\n", lut_value);
        }
    }
    fprintf(output_file, "};\n\n");

    fprintf(output_file, "static const double CORDIC_HYPERBOLIC_LUT[%d] = {\n", CORDIC_ITERATIONS + 1);
    fprintf(output_file, "    0.000000000000000,\n");
    for (int i = 1; i <= CORDIC_ITERATIONS; i++) {
        double lut_value = atanh(pow(2.0, -i));
        if (i < CORDIC_ITERATIONS) {
            fprintf(output_file, "    %.15f,\n", lut_value);
        } else {
            fprintf(output_file, "    %.15f\n", lut_value);
        }
    }
    fprintf(output_file, "};\n\n");
    
    fprintf(output_file, "#endif // CORDIC_LUT_H\n");
    fclose(output_file);
    fclose(log_file);

    printf("Success! 'include/cordic_lut.h' generated with LUTs and K constants.\n");
    return 0;
}