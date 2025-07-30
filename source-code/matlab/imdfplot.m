%% Calculate instantaneous median frequency and animate the process
% Author: Rengganis RH Santoso
% LstUpd: 2025-07-29

clear; close all;

% Load the data. The first column is the raw signal, while the second one
% was filtered using proprietary digital filter from UDLabs' Github
data = readtable("subjek1.txt");

%% Signal parameter

Fs = 500; % Sampling frequency 
raw = data.Var1(10*Fs:53000);
L = length(raw);
t = (0:L-1) / Fs; % Create a time vector based on the sampling frequency

%% Perform a bandpass filter using Butterworth IIR filter 10th order at 20-230 Hz

iirbandpass = designfilt('bandpassiir', 'FilterOrder', 10, ...
    'HalfPowerFrequency1', 20, 'HalfPowerFrequency2', 230, ...
    'SampleRate', Fs);
filtered = filter(iirbandpass,raw);

%% Animate MDF calculation and sliding window on the graph

k = 0; % Window index
wlen = 2000; % Window length in sample
overlap = floor(0.5*wlen); % Number of overlap in sample, set as 50%
xshift = wlen-overlap; % The amount of x-shift for each subsequent window
wsig = zeros(wlen,ceil((L-wlen)/xshift)); % Create a variable to store windowed signals

figure
pause
while true
    xstart = 1 + k*xshift; % Sliding window starting x position
    xend = xstart + wlen - 1; % Sliding window end x position
    if xend > L
        break
    end
    wsig(:,k+1) = filtered(xstart:xend); % Store all windowed signal into an array
    mdf = medfreq(wsig,Fs)'; % Calculate the median frequency
    
    subplot(2,1,1); plot(mdf,'o') % MDF plot
    title(sprintf('wlen=%.d, overlap=%.d (Fs=%.d)',wlen,overlap,Fs))
    xlabel('window')
    ylabel('MDF')
    xlim([1 ceil((L-wlen)/xshift)])

    subplot(2,1,2); plot(t,filtered) % EMG plot
    xlabel('time (s)')
    ylabel('raw EMG')
    xlim([t(1) t(end)])
    hold on
    xregion(xstart/Fs,xend/Fs, 'FaceColor', 'y'); 
    xline(xstart/Fs)
    xline(xend/Fs)
    hold off

    pause(1/60) % Set animation speed to 60 fps (affected by computation power)
    k = k+1;
end

kspace = 1:k; % Horizontal axis for median frequency
p = polyfit(kspace,mdf,1); % Perform linear regression using 1st order polynomial fitting
yfit = polyval(p,kspace);
yresid = mdf - yfit';
SSresid = sum(yresid.^2);
SStotal = (length(mdf)-1) * var(mdf);
rsq = 1 - SSresid/SStotal % Calculate R-squared

subplot(2,1,1); hold on; plot(yfit) % Plot the regression result
text(1,min(mdf), ...
    sprintf('Regression model: $y=%.03f x + %.03f $',p(1),p(2)), ...
    'Interpreter', 'latex') % Display the regression model
