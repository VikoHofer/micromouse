clear; clc; close all;

files = {
    'Step_Both_Wheels_IN200_20ms.csv'
    'Step_Both_Wheels_IN400_20ms.csv'
    'Step_Both_Wheels_IN600_20ms.csv'
    'Step_Both_Wheels_IN800_20ms.csv'
    'Step_Both_Wheels_IN1000_20ms.csv'
};

pwm_values = [200 400 600 800 1000];

n = numel(files);
v_ss = zeros(n,1);
tau_est = zeros(n,1);
K_fit = zeros(n,1);
tau_fit = zeros(n,1);

figure;
tiledlayout(n,1);

for i = 1:n
    T = readtable(files{i}, 'Delimiter', ';');
    t = T.TIME / 1000;          % ms -> s
    v = T.VELOCITY;

    % stationärer Wert = Mittelwert der letzten 25 %
    n_tail = max(5, floor(length(v)/4));
    v_ss(i) = mean(v(end-n_tail+1:end));

    % 63.2 %-Methode für tau
    v632 = 0.632 * v_ss(i);
    idx = find(v >= v632, 1, 'first');
    if ~isempty(idx)
        tau_est(i) = t(idx);
    else
        tau_est(i) = NaN;
    end

    % 1. Ordnung Fit: v(t) = K * (1 - exp(-t/tau))
    model = fittype('K*(1-exp(-x/tau))', ...
        'independent', 'x', 'coefficients', {'K','tau'});

    fitopts = fitoptions(model);
    fitopts.StartPoint = [v_ss(i), max(0.05, tau_est(i))];
    fitopts.Lower = [0, 0];
    fitopts.Upper = [Inf, 10];

    try
        f = fit(t, v, model, fitopts);
        K_fit(i) = f.K;
        tau_fit(i) = f.tau;
    catch
        K_fit(i) = NaN;
        tau_fit(i) = NaN;
    end

    nexttile;
    plot(t, v, 'o-'); hold on;
    if ~isnan(K_fit(i))
        tt = linspace(min(t), max(t), 200);
        plot(tt, K_fit(i)*(1-exp(-tt/tau_fit(i))), '--', 'LineWidth', 1.5);
    end
    grid on;
    title(sprintf('PWM = %d', pwm_values(i)));
    xlabel('t [s]');
    ylabel('v [mm/s]');
    legend('Messung', '1. Ordnung Fit', 'Location', 'best');
end

% globaler Fit: PWM = kV * speed + kS
valid = ~isnan(K_fit);
p = polyfit(K_fit(valid), pwm_values(valid)', 1);
kV = p(1);
kS = p(2);

tau_mean = mean(tau_fit(valid), 'omitnan');
kA = kV * tau_mean;

fprintf('--- Ergebnisse ---\n');
for i = 1:n
    fprintf('PWM=%4d | v_ss=%8.2f mm/s | tau_est=%6.4f s | K_fit=%8.2f | tau_fit=%6.4f s\n', ...
        pwm_values(i), v_ss(i), tau_est(i), K_fit(i), tau_fit(i));
end

fprintf('\nGlobaler Feedforward-Fit:\n');
fprintf('kV = %.6f PWM/(mm/s)\n', kV);
fprintf('kS = %.6f PWM\n', kS);
fprintf('tau_mean = %.6f s\n', tau_mean);
fprintf('kA = %.6f PWM/(mm/s^2)\n', kA);

% Kennlinie plotten
figure;
plot(K_fit(valid), pwm_values(valid), 'o', 'LineWidth', 1.5); hold on;
xx = linspace(0, max(K_fit(valid))*1.1, 200);
yy = polyval(p, xx);
plot(xx, yy, '--', 'LineWidth', 1.5);
grid on;
xlabel('Speed [mm/s]');
ylabel('PWM');
title('Feedforward Kennlinie: PWM = kV * v + kS');
legend('Messpunkte', 'Linearer Fit', 'Location', 'best');