#include <stdint.h>

#define FILTER_ORDER 64
float filter_coeffs[FILTER_ORDER] = { /* hier die exportierten FIR-Koeffizienten */ };
float buffer[FILTER_ORDER] = {0}; // Ringpuffer

float apply_fir_filter(float input) {
    // Neue Eingabe in den Puffer schieben
    for (int i = FILTER_ORDER - 1; i > 0; i--) {
        buffer[i] = buffer[i - 1];
    }
    buffer[0] = input;

    // Faltung durchführen
    float output = 0.0;
    for (int i = 0; i < FILTER_ORDER; i++) {
        output += buffer[i] * filter_coeffs[i];
    }
    return output;
}
#define IIR_ORDER 4
float a_coeffs[IIR_ORDER + 1] = {1, -3.18063854887472, 3.86119434899422, -2.11215535511097, 0.438265142261980}; // Aus MATLAB exportieren
float b_coeffs[IIR_ORDER + 1] = {0.000416599204406596, 0.00166639681762638, 0.00249959522643958, 0.00166639681762638, 0.000416599204406596};

float input_buffer[IIR_ORDER + 1] = {0};
float output_buffer[IIR_ORDER + 1] = {0};

float apply_iir_filter(float input) {
    // Eingabe aktualisieren
    for (int i = IIR_ORDER; i > 0; i--) {
        input_buffer[i] = input_buffer[i - 1];
        output_buffer[i] = output_buffer[i - 1];
    }
    input_buffer[0] = input;

    // Berechnung
    float output = 0.0;
    for (int i = 0; i <= IIR_ORDER; i++) {
        output += b_coeffs[i] * input_buffer[i];
        if (i > 0) {
            output -= a_coeffs[i] * output_buffer[i];
        }
    }
    output_buffer[0] = output;

    return output;
}
