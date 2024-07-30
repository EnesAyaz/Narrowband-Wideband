clear ;
clc;
tic;
%% Time array
ma = 1;
fout = 80e3; % Hz
fsw = 1e6; % Hz
% Tstep = 1/(fsw*(fsw-2*fout)*(fsw+2*fout)); % s
% Tstep=Tstep/2;
Tstep=1e-12;
Ts = Tstep; % s
Tfinal =16/fout ; % s
time_array = 0:Tstep:Tfinal-Tstep;
NumberofSteps = numel(time_array);
%Generate switching signals
The_c=0;
The_f=-pi/6;
phaseP=The_f+0;
phaseN=The_f-pi;
VrefP = ma*cos(2*pi*fout*time_array+phaseP);
VrefN = ma*cos(2*pi*fout*time_array+phaseN);
Vtriang = zeros(1, NumberofSteps);
for k = 1:Tfinal*fsw
   Triang_temp = triang(1/(Ts*fsw));
   Vtriang((length(Triang_temp)*(k-1)+1:k/(Tstep*fsw))) = (Triang_temp*2)-1;
end

carrierPhP=0;
carrierPhN=0;
carP= round(carrierPhP/(fsw*Ts)/360);
if carP==0
    carP=1;
end
carN= round(carrierPhN/(fsw*Ts)/360);
if carN==0
    carN=1;
end

VcarrierP = [ Vtriang(carP:end), zeros(1,carP-1)];
VcarrierN = [ Vtriang(carN:end), zeros(1,carN-1)];

SP = double(VrefP > VcarrierP);
SN = double(VrefN > VcarrierN);
%%
Y = fft(SP);

P2 = abs(Y/NumberofSteps);
P1 = P2(1:NumberofSteps/2+1);
P1(2:end-1) = 2*P1(2:end-1);

Fs=1/Ts;

f = Fs/NumberofSteps*(0:(NumberofSteps/2));
stem(f,P1,"LineWidth",3) 
title("Single-Sided Amplitude Spectrum of X(t)")
xlabel("f (Hz)")
ylabel("|P1(f)|")
xlim([0 2e6])
