% --- Schritt 1: CSV-Datei einlesen ---
filename = 'Test.csv'; % Dateiname anpassen
data = readtable(filename, 'HeaderLines', 1); % Erste Zeile überspringen

time = data{:, 1}; % Zeitspalte (erste Spalte)
speed = data{:, 2}; % Geschwindigkeitsspalte (zweite Spalte)

time = time /1000;

% Abtastfrequenz berechnen
Fs = 1/0.010; % Abtastfrequenz basierend auf den Zeitdaten

% --- Schritt 2: Daten visualisieren ---
figure;
subplot(3, 2, 1);
plot(time, speed, 'b-');
grid on;
title('Sprungantwort der Geschwindigkeit (ungefiltert)');
xlabel('Zeit (s)');
ylabel('Geschwindigkeit (mm/s)');

% --- Schritt 3: FFT der ungefilterten Daten ---
N = length(speed); % Anzahl der Datenpunkte
f = Fs * (0:(N/2)) / N; % Frequenzachse

Y = fft(speed);
P2 = abs(Y / N); % Normierte Amplitude
P1 = P2(1:N/2+1); % Nur die positive Hälfte der Frequenzen
P1(2:end-1) = 2 * P1(2:end-1); % Verdoppelung für die positive Hälfte

subplot(3, 2, 2);
plot(f, P1, 'r-');
grid on;
title('Frequenzspektrum (ungefiltert)');
xlabel('Frequenz (Hz)');
ylabel('Amplitude');

% --- Schritt 4: Tiefpassfilter anwenden ---
Fc = 1; % Grenzfrequenz in Hz
Wn = Fc / (Fs / 2); % Normalisierte Grenzfrequenz
b = fir1(16, Wn, 'low'); % FIR-Filter 4. Ordnung
fprintf('%f, ', b);
%[b, a] = butter(4, Wn, 'low'); % Butterworth-Filter 4. Ordnung

%speed_filtered = filtfilt(b, a, speed); % Gefilterte Geschwindigkeit
speed_filtered = filter(b, 1, speed); % Gefilterte Geschwindigkeit

subplot(3, 2, 3);
plot(time, speed_filtered, 'g-');
grid on;
title('Sprungantwort der Geschwindigkeit (gefiltert)');
xlabel('Zeit (s)');
ylabel('Geschwindigkeit (mm/s)');

% --- Schritt 5: FFT der gefilterten Daten ---
Y_filtered = fft(speed_filtered);
P2_filtered = abs(Y_filtered / N); % Normierte Amplitude
P1_filtered = P2_filtered(1:N/2+1); % Nur die positive Hälfte der Frequenzen
P1_filtered(2:end-1) = 2 * P1_filtered(2:end-1); % Verdoppelung für die positive Hälfte

subplot(3, 2, 4);
plot(f, P1_filtered, 'r-');
grid on;
title('Frequenzspektrum (gefiltert)');
xlabel('Frequenz (Hz)');
ylabel('Amplitude');

% --- Schritt 6: Vergleich gefiltert vs. ungefiltert ---
subplot(3, 2, [5, 6]);
plot(f, P1, 'b-', 'DisplayName', 'Ungefiltert');
hold on;
plot(f, P1_filtered, 'r-', 'DisplayName', 'Gefiltert');
grid on;
title('Vergleich Frequenzspektren');
xlabel('Frequenz (Hz)');
ylabel('Amplitude');
legend show;
