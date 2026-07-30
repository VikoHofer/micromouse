% --- Schritt 1: CSV-Datei einlesen ---
filename = 'Step_Both_Wheels.csv';
data = readtable(filename, 'HeaderLines', 1); % Erste Zeile überspringen

time = data{:, 1}; % Zeitspalte (erste Spalte)
speed = data{:, 2}; % Geschwindigkeitsspalte (zweite Spalte)
time = time / 1000;

figure;
plot(time, speed, 'b-');
grid on;
title('Sprungantwort der Geschwindigkeit');
xlabel('Zeit (s)');
ylabel('Geschwindigkeit (mm/s)');

filename = 'Step_One_Wheel.csv'; 
data = readtable(filename, 'HeaderLines', 1); % Erste Zeile überspringen

time = data{:, 1}; % Zeitspalte (erste Spalte)
speed = data{:, 2}; % Geschwindigkeitsspalte (zweite Spalte)
time = time / 1000;

figure;
plot(time, speed, 'b-');
grid on;
title('Sprungantwort der Geschwindigkeit');
xlabel('Zeit (s)');
ylabel('Geschwindigkeit (mm/s)');

%% Modellidentifikation

Km = 7.2; 
Tm = 1.4;
Damping_ratio = 0.707;
Settling_time =  0.4;

% PD-Values
KP_calculated = (Tm / Km) * (16/((Damping_ratio*Damping_ratio)*(Settling_time*Settling_time)));	
KD_calculated = ((8*Tm) - Settling_time) / (Settling_time*Km);

%Übertragungsfunktion DC-Motor
num = [Km];
den = [Tm,1];
DC_sys = tf(num,den);

% Zeitvektor für Simulation
t = 0:0.001:10;  % Zeit von 0 bis 10 Sekunden mit Schrittgröße 0.01 Sekunden

% Eingangs-Sprungsignal mit einer Schrittgröße von A (z.B. A=5)
A = 110;
u = A * ones(size(t));  % Sprungsignal mit der gewünschten Schrittgröße

% Systemantwort auf den Sprung mit benutzerdefiniertem Eingangssignal
[y, t_out] = lsim(DC_sys, u, t);  % lsim für benutzerdefiniertes Eingangssignal

% Plotten der Systemantwort
figure;
hold on;
plot(time, speed, 'b-');
plot(t_out, y);
title('Systemantwort auf benutzerdefinierten Step Input');
xlabel('Zeit (s)');
ylabel('Antwort (Amplitude)');

%% Modellidentifikation angle_velocity (Maus)

Km_ang = 11.36; 
Tm_ang = 0.8;
Damping_ratio = 0.707;
Settling_time =  0.4;

% PD-Values
KP_calculated_vel = (Tm_ang / Km_ang) * (16/((Damping_ratio*Damping_ratio)*(Settling_time*Settling_time)));	
KD_calculated_angle = ((8*Tm_ang) - Settling_time) / (Settling_time*Km_ang);

%Übertragungsfunktion DC-Motor
num = [Km_ang];
den = [Tm_ang,1];
ANG_sys = tf(num,den);

% Plotten der Systemantwort
figure;
step(ANG_sys)
title('Systemantwort ANG_sys');


%% Regler Berechnung
Ta = 0.010;
G_s = DC_sys;
G_z = c2d(G_s, Ta, 'zoh'); 

step(G_z);
% Entwurfsvorgaben (aus der Aufgabenstellung)
% Entwurfsvorgaben (aus der Aufgabenstellung)
tr = 0.4;      % Anstiegszeit
Ue = 1;     % Überschwingen in Prozent

% Berechnung von omega_c und beta (basierend auf den Formeln)
omega_c = 1.5 / tr;
beta = 70 - Ue;

% Berechnung des Reglers (digital)
arg_G_omega_c_Ta = angle(freqresp(G_z, omega_c*Ta));
alpha = cos(omega_c*Ta) - sin(omega_c*Ta) / tan(deg2rad(-180 + beta - arg_G_omega_c_Ta));

abs_G_omega_c_Ta = abs(freqresp(G_z, omega_c*Ta));
VR = 1 / (abs_G_omega_c_Ta * sqrt((cos(omega_c*Ta) - alpha)^2 + sin(omega_c*Ta)^2) / sqrt((cos(omega_c*Ta) - 1)^2 + sin(omega_c*Ta)^2));

% Reglerdefinition (digital)
numR_z = VR*[1 -alpha];
denR_z = [1 -1];
R_z = tf(numR_z, denR_z, Ta); % Wichtig: Abtastzeit angeben!

% Offener Regelkreis (digital)
L_z = G_z*R_z;

% Geschlossener Regelkreis (digital)
T_z = feedback(L_z, 1);

% Ausgabe der Reglerparameter
fprintf('alpha: %f\n', alpha);
fprintf('VR: %f\n', VR);

% Umrechnung in KP und TN (für den Vergleich)
KP_z = VR;
TN_z = Ta / (1 - alpha);

fprintf('KP_z: %f\n', KP_z);
fprintf('TN_z: %f\n', TN_z);
fprintf('KI_z: %f\n', KP_z/TN_z);

% Bode-Diagramme
figure;
bode(G_s,G_z, L_z);
legend('G(s)','G(z)', 'L(z)');
grid on;

% Sprungantwort
figure;
step(T_z);
grid on;


%Nullstellen und Pole der Regler
zero_R_z = zero(R_z);
pole_R_z = pole(R_z);
fprintf('Nullstelle des digitalen Reglers: %f\n', zero_R_z);
fprintf('Pol des digitalen Reglers: %f\n', pole_R_z);