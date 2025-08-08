% PLOTFFT Plot the Fast Fourier Transform of a given signal.
%   Syntax
%     [P1,f] = PLOTFFT(X,Fs)
% 
%   Input Arguments
%     X - Vector containing the signal.
%       vector
%     Fs - Sampling frequency in Hz.
%       positive integer scalar
% 
%   Output Arguments
%     P1 - One-sided amplitude spectrum of X.
%       vector
%     f - Frequency vector (bin) of the FFT.
%       vector
% 
%   See also FFT.
% Author: Rengganis RH Santoso
% LstUpd: 2025-08-08

function [P1,f] = PlotFFT(X, Fs)
    L = length(X);  % Signal length
    T = 1/Fs;       % Sampling period
    t = (0:L-1)*T;  % Time vector

    Y = fft(X);                     % Calculate the FFT
    P2 = abs(Y/L);                  % Two-sided spectrum
    P1 = P2(1:L/2+1);               % One-sided spectrum
    P1(2:end-1) = 2*P1(2:end-1);    % Scale the amplitude
    f = (0:(L/2)) * Fs/L;           % Frequency vector

    figure; 
    subplot(2,1,1); plot(t,X);      % Plot the signal
    xlabel('Time (t) [s]');
    ylabel('X(t)');
    title('Signal X in Time Domain');
    subplot(2,1,2); plot(f, P1);    % Plot the single-sided amplitude spectrum
    xlabel('Frequency (f) [Hz]');
    ylabel('|P1(f)|');
    title('Single-Sided Amplitude Spectrum of X');
end