% Identify VZ2 system
function [Sys, V, xi, T] = identPT2(h1, t1, hinf, r)
V = 1;
xi = 1;
T = 1;
Sys = 1;

V = hinf / r;


sigma = (h1 - hinf) / hinf;
xi = sqrt((log(sigma))^2 / (pi^2 + (log(sigma))^2));

wd = pi / t1; 
wn = wd / sqrt(1 - xi^2); 
T = 1 / wn; 

s = tf('s');
Sys = V / (T^2 * s^2 + 2 * xi * T * s + 1);