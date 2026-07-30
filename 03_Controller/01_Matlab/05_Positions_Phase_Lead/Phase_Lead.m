% MATLAB Script for Phase-Lead Controller Design

% System Parameters
Km = 7.2;               % DC gain
Tm = 1.4;               % Time constant (seconds)
Ts = 0.001;             % Sampling time (seconds)
tr = 0.5;               % Anstiegszeit
u = 10;                 % Überschwingen

wc = 1.5/tr;            % Bandwidth (rad/s)
phi_c = 70 - u;         % Desired phase margin (degrees)

num = [Km];
den = [Tm,1];
DC_sys = tf(num,den)
Gz = c2d(DC_sys, Ts);

% Phase



Tz = 1;
Tn = 1;

% Regler
numerator = Kc * [Tz, 1]; % Kc * (T1s + 1)
denominator = [Tn, 1];    % T2s + 1

% Transferfunktion erstellen
G = tf(numerator, denominator)

