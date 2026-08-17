%% Bode plot of the motor transfer function
%                 0.176
% G_m(s) = -----------------------------
%          1.125e-6*s^2 + 22.5e-4*s + 0.03166

% This script uses tf and bodeplot from the Control System Toolbox.

clear;
clc;
close all;

numerator = 0.176;
denominator = [1.125e-6, 22.5e-4, 0.03166];
Gm = tf(numerator, denominator);

omega = logspace(-1, 5, 2000);  % Angular frequency (rad/s)

options = bodeoptions;
options.FreqUnits = 'rad/s';
options.MagUnits = 'dB';
options.PhaseUnits = 'deg';
options.Grid = 'on';

figure('Color', 'w');
bodeplot(Gm, omega, options);
title({'Bode Plot of Motor Transfer Function', ...
       '$G_m(s)=\frac{0.176}{1.125\times10^{-6}s^2+22.5\times10^{-4}s+0.03166}$'}, ...
      'Interpreter', 'latex');

disp('Motor transfer function:');
disp(Gm);
disp('Poles of G_m(s):');
disp(pole(Gm));
