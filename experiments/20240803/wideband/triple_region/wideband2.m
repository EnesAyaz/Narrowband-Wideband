 load('C:\Github\Narrow-Wide Band Gap\Experiment\20240619\wide\wide_ch2.mat')
CH2data=data; 
CH2_time=time; 

load('C:\Github\Narrow-Wide Band Gap\Experiment\20240619\wide\wide_ch3.mat')
CH3data=data; 
CH3_time=time; 

load('C:\Github\Narrow-Wide Band Gap\Experiment\20240619\wide\wide_ch4.mat')
CH4data=data; 
CH4_time=time; 

load('C:\Github\Narrow-Wide Band Gap\Experiment\20240619\wide\wide_ch5.mat')
CH5data=data; 
CH5_time=time; 

load('C:\Github\Narrow-Wide Band Gap\Experiment\20240619\wide\wide_ch6.mat')
CH6data=data; 
CH6_time=time; 

clear horizontalUnits model  recordLength sampleInterval time data verticalUnits waveformType waveformSource zeroIndex

%%
N=74000
figure()
plot(CH4_time*1e3, CH4data)
hold on; 
plot(CH5_time*1e3, CH5data)
hold on; 
% plot(CH2_time(1:N)*1e3, CH2data(1:N))
% hold on; 
% plot(CH3_time(1:N)*1e3, CH3data(1:N))
% hold on 
% plot(CH6_time*1e3, CH6data)

N=74000;
N2=175000;
[rms(CH3data(1:N)) rms(CH3data(N:N2)) rms(CH3data(N2:end)) ]
N=74000;
N2=175000;
[rms(CH2data(1:N)) rms(CH2data(N:N2)) rms(CH2data(N2:end))]



% N=175000;
% rms(CH3data(1:N))
% rms(CH3data(N:end))
% N=74000;
% rms(CH2data(1:N))
% rms(CH2data(N:end))

%%
figure()
plot(CH4_time*1e3+1.5, CH4data)
hold on; 
plot(CH5_time*1e3+1.5, CH5data/2+1.5)
hold on; 
plot(CH2_time*1e3+1.5, CH2data/5-0.5)
hold on; 
plot(CH3_time*1e3+1.5, CH3data/5-3)
hold on 
plot(CH6_time*1e3+1.5, CH6data/100+4.5)
% ylim([-5 5])
%%
close all
figure1 = figure('Renderer', 'painters', 'Position', [10 10 1000 400]);

% Create axes
axes1 = axes('Parent',figure1);
hold(axes1,'on');

% Create multiple line objects using matrix input to plot
plot1 = plot(CH4_time*1e3+1.5, CH4data,'Parent',axes1);
plot2 = plot(CH5_time*1e3+1.5, CH5data/2+1.5,'Parent',axes1);
plot3 = plot(CH2_time*1e3+1.5, CH2data/5-0.5,'Parent',axes1);
plot4 = plot(CH3_time*1e3+1.5, CH3data/5-3,'Parent',axes1);
plot5 = plot(CH6_time*1e3+1.5, CH6data/100+4.5,'Parent',axes1);

% set(plot1,'DisplayName','I_{Load-LF}','Color',[0 0 1]);
% set(plot2,'DisplayName','I_{Load-HF}','Color',[1 0 0]);
% set(plot3,'DisplayName','I_{RX-LF}',...
%     'Color',[0.505882352941176 0.619607843137255 0.968627450980392]);
% set(plot4,'DisplayName','I_{RX-HF}',...
%     'Color',[0.92156862745098 0.654901960784314 0.607843137254902]);
% set(plot5,'DisplayName','V_{AB}',...
%     'Color',[0.737254901960784 0.509803921568627 0.749019607843137]);

% % Create xlabel
 % xlabel({'Time (ms)'});
% 
% Uncomment the following line to preserve the Y-limits of the axes
ylim(axes1,[-5 6.5]);
xlim(axes1, [-4 15])
box(axes1,'on');
hold(axes1,'off');
% Set the remaining axes properties
% set(axes1,'FontName','Times New Roman','FontSize',15,'GridAlpha',0.25,...
%     'GridLineWidth',0.1,'MinorGridLineStyle','-','MinorGridLineWidth',0.1,...
%     'XGrid','on','XMinorGrid','on','XTick',[ 0.5 1 1.5 2 2.5 3 3.5 4],...
%     'XTickLabel',{'0','0.5','1','1.5','2','2.5','3','3.5'},...
%     'YGrid','on','YMinorGrid','on','YTick',[-5 -2.5 0 2.5 5],'YTickLabel', {});

set(axes1,'FontName','Times New Roman','FontSize',15,'GridAlpha',0.75,...
    'GridLineWidth',0.1,'MinorGridLineStyle','-','MinorGridLineWidth',0.1,'MinorGridAlpha',0.25,...
    'XGrid','on','XMinorGrid','on','XTick',[ -4    -3    -2    -1     0     1     2     3     4     5     6     7     8     9    10    11    12    13    14],...
    'XTickLabel',{},...
    'YGrid','on','YMinorGrid','on','YTick',[-5 -2.5 0 2.5 5],'YTickLabel', {});

% Create legend
% legend1 = legend(axes1,'show');
% set(legend1,...
%     'Position',[0.866443454010762 0.333134929504662 0.122767855180427 0.359523799234913],...
%     'EdgeColor','none');

