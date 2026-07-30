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
Settling_time =  0.25;

% PD-Values
KP_calculated_vel = (Tm_Vel / Km_vel) * (16/((Damping_ratio*Damping_ratio)*(Settling_time*Settling_time)));	
KD_calculated_vel = ((8*Tm_Vel) - Settling_time) / (Settling_time*Km_vel);

%Übertragungsfunktion DC-Motor
num = [Km_vel];
den = [Tm_Vel,1];
DC_sys = tf(num,den);
Gz = c2d(DC_sys, 0.01);

% Plotten der Systemantwort
figure;
step(DC_sys)
title('Systemantwort DC_sys');

%% Modellidentifikation angle_velocity (Maus)

Km_ang = 19.80; 
Tm_ang = 0.26;
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