#include <stdio.h>
#include <math.h>

double precalculate_circular_gain(int n) {
    double an_gain = 1.0;
    for (int i = 0; i < n; i++) {
        double current_term = sqrt(1.0 + pow(4.0, -i));
        an_gain *= current_term;
    }
    return 1.0 / an_gain;
}

double precalculate_hyperbolic_gain(int n) {
    double an_gain = 1.0;
    for (int i = 1; i <= n; i++) {
        double current_term = sqrt(1.0 - pow(4.0, -i));
        an_gain *= current_term;
        if (i == 4 || i == 13) {
            current_term = sqrt(1.0 - pow(4.0, -i));
            an_gain *= current_term;
        }
    }
    return 1.0 / an_gain;
}

int main() {
    FILE *output_file = fopen("include/cordic_lut.h", "w");
    
    if (output_file == NULL) {
        printf("Fatal error: Could not create file cordic_lut.h\n");
        return 1; 
    }

    double k_circular = precalculate_circular_gain(24);
    double k_hyperbolic = precalculate_hyperbolic_gain(24);

    fprintf(output_file, "#ifndef CORDIC_LUT_H\n");
    fprintf(output_file, "#define CORDIC_LUT_H\n\n");

    fprintf(output_file, "#define CORDIC_K_CIRCULAR %.15f\n", k_circular);
    fprintf(output_file, "#define CORDIC_K_HYPERBOLIC %.15f\n\n", k_hyperbolic);

    fprintf(output_file, "static const double CORDIC_CIRCULAR_LUT[24] = {\n");
    for (int i = 0; i < 24; i++) {
        double lut_value = atan(pow(2.0, -i));
        if (i < 23) {
            fprintf(output_file, "    %.15f,\n", lut_value);
        } else {
            fprintf(output_file, "    %.15f\n", lut_value);
        }
    }
    fprintf(output_file, "};\n\n");

    fprintf(output_file, "static const double CORDIC_HYPERBOLIC_LUT[25] = {\n");
    fprintf(output_file, "    0.000000000000000,\n");
    for (int i = 1; i <= 24; i++) {
        double lut_value = atanh(pow(2.0, -i));
        if (i < 24) {
            fprintf(output_file, "    %.15f,\n", lut_value);
        } else {
            fprintf(output_file, "    %.15f\n", lut_value);
        }
    }
    fprintf(output_file, "};\n\n");
    
    fprintf(output_file, "#endif // CORDIC_LUT_H\n");
    fclose(output_file);

    printf("Success! 'include/cordic_lut.h' generated with LUTs and K constants.\n");
    return 0;
}