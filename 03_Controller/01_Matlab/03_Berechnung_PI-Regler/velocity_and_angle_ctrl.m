% --- Schritt 1: CSV-Datei einlesen ---
filename = 'Step_One_Wheel_10ms_filtered.csv';
data = readtable(filename, 'HeaderLines', 1); % Erste Zeile überspringen

time = data{:, 1}; % Zeitspalte (erste Spalte)
speed = data{:, 2}; % Geschwindigkeitsspalte (zweite Spalte)
time = time / 1000;

figure;
plot(time, speed, 'b-');
grid on;
title('Sprungantwort der forwarVelocity');
xlabel('Zeit (s)');
ylabel('Geschwindigkeit (mm/s)');

filename = 'Step_AngleVel_10ms_filtered.csv'; 
data = readtable(filename, 'HeaderLines', 1); % Erste Zeile überspringen

time = data{:, 1}; % Zeitspalte (erste Spalte)
speed = data{:, 2}; % Geschwindigkeitsspalte (zweite Spalte)
time = time / 1000;

figure;
plot(time, speed, 'b-');
grid on;
title('Sprungantwort der AngleVelocity');
xlabel('Zeit (s)');
ylabel('Geschwindigkeit (mm/s)');

%% Modellidentifikation velocity (Motor)

Km_vel = 7.2; 
Tm_Vel = 1.4;
Damping_ratio = 0.707;
Settling_time =  0.2;

% PD-Values
KP_calculated_vel = (Tm_Vel / Km_vel) * (16/((Damping_ratio*Damping_ratio)*(Settling_time*Settling_time)));	
KD_calculated_vel = ((8*Tm_Vel) - Settling_time) / (Settling_time*Km_vel);

%Übertragungsfunktion DC-Motor
num = [Km_vel];
den = [Tm_Vel,1];
DC_sys = tf(num,den);

% Plotten der Systemantwort
figure;
step(DC_sys)
title('Systemantwort DC_sys');

%% Modellidentifikation angle_velocity (Maus)

Km_ang = 11.36; 
Tm_ang = 0.8;
Damping_ratio = 0.707;
Settling_time =  0.4;

% PD-Values
KP_calculated_angle = (Tm_ang / Km_ang) * (16/((Damping_ratio*Damping_ratio)*(Settling_time*Settling_time)));	
KD_calculated_angle = ((8*Tm_ang) - Settling_time) / (Settling_time*Km_ang);

%Übertragungsfunktion DC-Motor
num = [Km_ang];
den = [Tm_ang,1];
ANG_sys = tf(num,den);

% Plotten der Systemantwort
figure;
step(ANG_sys)
title('Systemantwort ANG_sys');



%% MATLAB-Skript für den Entwurf eines PID-Reglers basierend auf Anstiegszeit tr und Überschwingen 

Ta = 0.01;
% Eingabeparameter
tr = 1;  % Anstiegszeit in Sekunden (z. B. 2s)
Ue = 5; % Überschwingen in Prozent (z. B. 10%)

% Faustformeln
omega_c = 1.5 / tr;                % Durchtrittsfrequenz
beta = 70 - Ue;                   % Phasenreserve

% Modell der Strecke (beliebige Übertragungsfunktion)

G_z = c2d(DC_sys, Ta);  % Abtastzeit Ta = 10ms

% Berechnung von Alpha
alpha = cos(omega_c * Ta) - sin(omega_c * Ta) / tan(deg2rad(beta));

% Berechnung von VR
% Betrag der Übertragungsfunktion bei omega_c
G_mag = abs(evalfr(G_z, exp(1j * omega_c * Ta)));

% Betrag des Reglers bei omega_c
R_mag = sqrt((cos(omega_c * Ta) - alpha)^2 + sin(omega_c * Ta)^2) / ...
        sqrt((cos(omega_c * Ta) - 1)^2 + sin(omega_c * Ta)^2);

VR = 1 / (G_mag * R_mag);

% Berechnung der PID-Parameter
Kp = VR;                          % Proportionalanteil
Ti = Ta / (1 - alpha);           % Nachstellzeit (Integralanteil)

% Berechnung von Td basierend auf Phasenkorrektur
% Zusätzliche Phase durch den Differentialanteil
phi_d = deg2rad(beta) - (angle(evalfr(G_z, exp(1j * omega_c * Ta))) + ...
        atan2(sin(omega_c * Ta), cos(omega_c * Ta) - alpha));

% Vorhaltzeit aus der Phasenkorrektur
Td = tan(phi_d) / omega_c;

% Erstellung des Reglers
numerator_R = Kp * [Ti*Td Ti 1];
denominator_R = [Ti 0];
R_z = tf(numerator_R, denominator_R, Ta);

% Offene Übertragungsfunktion
L_z = series(R_z, G_z);

% Geschlossener Regelkreis
T_z = feedback(L_z, 1);

% Ergebnisse anzeigen
fprintf('Digitaler PID-Regler:\n');
fprintf('Kp = %.3f\n', Kp);
fprintf('Ti = %.3f\n', Ti);
fprintf('Td = %.3f\n', Td);

% Sprungantwort
figure;
step(T_z);
grid on;
title('Sprungantwort des geschlossenen Regelkreises');
xlabel('Zeit [s]');
ylabel('Amplitude');

% Bode-Diagramm
figure;
bode(G_z, L_z);
grid on;
title('Bode-Diagramme von G(z) und L(z)');
