static double angle_controller(double set_angle, bool sensor_on) {
    static double prev_error = 0;
    static double prev_output = 0;

    // Reglerparameter (anpassen)
    double K = 25.0;      // Verstärkung
    double z = 0.6;       // Nullstelle
    double p = 0.3;       // Pol

    // Abtastzeit (in Sekunden)
    double T = LOOP_INTERVAL * 0.001;

    // Get current angle from encoders
    double position_L = Get_Encoder_Position_mm_Left();
    double position_R = Get_Encoder_Position_mm_Right();
    double angle = position_L - position_R;

    // Fehlerberechnung
    double error = set_angle - angle;

    // Falls IR-Sensoren aktiviert sind, Fehler anpassen
    if (sensor_on) {
        double error_ir = ir_angle_error();
        error += calculate_steering_adjustment(error_ir);
    }

    // Phasenführungsregler (diskrete Implementierung)
    double output = prev_output + K * ((1 + z * T) * error - (1 + p * T) * prev_error);

    // Begrenzung des Outputs (optional)
    if (output > 255) output = 255;
    if (output < -255) output = -255;

    // Update vorherige Werte
    prev_error = error;
    prev_output = output;

    return output;
}
static double position_controller(double set_position) {
    static double prev_error = 0;
    static double prev_output = 0;

    // Reglerparameter (anpassen)
    double K = 17.0;      // Verstärkung
    double z = 0.5;       // Nullstelle
    double p = 0.2;       // Pol

    // Abtastzeit (in Sekunden)
    double T = LOOP_INTERVAL * 0.001;

    // Get current position from encoders
    double position_L = Get_Encoder_Position_mm_Left();
    double position_R = Get_Encoder_Position_mm_Right();
    double position = (position_L + position_R) / 2;

    // Fehlerberechnung
    double error = set_position - position;

    // Phasenführungsregler (diskrete Implementierung)
    double output = prev_output + K * ((1 + z * T) * error - (1 + p * T) * prev_error);

    // Begrenzung des Outputs (optional)
    if (output > 255) output = 255;
    if (output < -255) output = -255;

    // Update vorherige Werte
    prev_error = error;
    prev_output = output;

    return output;
}
