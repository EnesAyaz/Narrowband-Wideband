f1=200e3;
f2=2000e3;

rf=f2/f1;

rl=(4*rf^2*(rf^4+2*rf^2+1))/((rf^4-1)^2);
% rl=25;
 rl= 20;

a= ((rf^4+1)*rl-2*rf^2)
b=sqrt((rf^4-1)^2*rl^2-4*rf^2*(rf^4+2*rf^2+1)*rl)
c=2*(rf^2*rl^2+2*rf^2*rl+rf^2);

rc=(a-b)/c;

Lp1=460e-6;
Lp2=Lp1/rl;

a=sqrt((1/2)+(rc/2)*(1+rl-sqrt(((rl*rc-1)/rc)^2+2*((rl*rc+1)/rc)+1)))
b=sqrt((1/2)+(rc/2)*(1+rl+sqrt(((rl*rc-1)/rc)^2+2*((rl*rc+1)/rc)+1)))
fLp1cp1=f1/a
fLp2cp2=f2/b

Cp1=1/(4*pi*pi)/(fLp1cp1^2)/Lp1
Cp2=1/(4*pi*pi)/(fLp2cp2^2)/Lp2
% Cp2=Cp1/rc

Ls=48e-6;

Cs1=1/(4*pi*pi)/(f1^2)/Ls

Cs2=1/(4*pi*pi)/(f2^2)/Ls


