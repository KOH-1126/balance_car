%% First-order LPF and HPF magnitude responses
% H_L(s) = 1 / (tau*s + 1)
% H_H(s) = tau*s / (tau*s + 1)
%
% Use the normalized angular frequency x = omega*tau, so tau does not
% need to be assigned a numerical value. The cutoff is at x = 1, i.e.
% omega_c = 1/tau rad/s and f_c = 1/(2*pi*tau) Hz.

clear;
clc;
close all;

x = logspace(-2, 2, 2000);       % x = omega*tau

magLPF = 1 ./ sqrt(1 + x.^2);
magHPF = x ./ sqrt(1 + x.^2);

magLPF_dB = 20*log10(magLPF);
magHPF_dB = 20*log10(magHPF);
cutoff_dB = 20*log10(1/sqrt(2)); % -3.0103 dB

figure('Color', 'w');
semilogx(x, magLPF_dB, 'b-',  'LineWidth', 2); hold on;
semilogx(x, magHPF_dB, 'r--', 'LineWidth', 2);

% Mark the common cutoff frequency and the two cutoff points.
xline(1, 'k:', '$\omega\tau=1$', ...
    'Interpreter', 'latex', 'LabelVerticalAlignment', 'bottom', ...
    'LineWidth', 1.2);
yline(cutoff_dB, 'k:', '$-3.01\ \mathrm{dB}$', ...
    'Interpreter', 'latex', 'LabelHorizontalAlignment', 'left', ...
    'LineWidth', 1.2);
plot(1, cutoff_dB, 'ko', 'MarkerFaceColor', 'k', 'MarkerSize', 6);

grid on;
box on;
xlim([1e-2, 1e2]);
ylim([-42, 3]);
xticks([1e-2 1e-1 1 1e1 1e2]);
xlabel('Normalized angular frequency $\omega\tau$', ...
    'Interpreter', 'latex');
ylabel('Magnitude $20\log_{10}|H(j\omega)|$ (dB)', ...
    'Interpreter', 'latex');
title({'First-order LPF and HPF magnitude responses', ...
       '$\omega_c=1/\tau\ \mathrm{rad/s},\quad f_c=1/(2\pi\tau)\ \mathrm{Hz}$'}, ...
    'Interpreter', 'latex');
legend({'LPF: $|H_L(j\omega)|$', 'HPF: $|H_H(j\omega)|$'}, ...
    'Interpreter', 'latex', 'Location', 'southeast');

