%% Micromouse controller

Km = 8; 
Tm = 0.5; %Freilauf

Damping_ratio = 0.707;
Settling_time =  0.4;

% PD-Values
KP_calculated = (Tm / Km) * (16/((Damping_ratio*Damping_ratio)*(Settling_time*Settling_time)));	
KD_calculated = ((8*Tm) - Settling_time) / (Settling_time*Km);

%Übertragungsfunktion DC-Motor
num = [Km];
den = [Tm,1];
DC_sys = tf(num,den)

step(DC_sys)
