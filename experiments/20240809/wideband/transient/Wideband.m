 load('wideband_transient_ch1.mat')
CH1data=data; 
CH1_time=time; 

 load('wideband_transient_ch2.mat')
CH2data=data; 
CH2_time=time; 

load('wideband_transient_ch3.mat')
CH3data=data; 
CH3_time=time; 

load('wideband_transient_ch4.mat')
CH4data=data; 
CH4_time=time; 

load('wideband_transient_ch5.mat')
CH5data=data; 
CH5_time=time; 

%  load('wideband_transient_ch5.mat')
% CH6data=data; 
% CH6_time=time; 

clear horizontalUnits model  recordLength sampleInterval time data verticalUnits waveformType waveformSource zeroIndex

%%
N1=250000*0+1;
N=250000*1
figure()
plot(CH4_time*1e3, CH4data)
% hold on; 
% plot(CH5_time*1e3, CH5data)
hold on; 
% plot(CH2_time(N1:N)*1e3, CH2data(N1:N))
% % % hold on; 
% plot(CH3_time(N1:N)*1e3, CH3data(N1:N)*10)
hold on 
% plot(CH1_time*1e3, CH1data)

% rms(CH3data(1:N))
rms(CH3data(N1:N))

% rms(CH2data(1:N))
rms(CH2data(N1:N))

%%
figure()
plot(CH2_time*1e3, CH2data+65)
hold on; 
plot(CH3_time*1e3, CH3data*10+42)
hold on; 
plot(CH4_time*1e3, CH4data)
hold on 
plot(CH5_time*1e3, CH5data-21)
hold on; 
% plot(CH6_time*1e3, CH6data/50+25)
% ylim([-5 5])
%%

figure1 = figure('Renderer', 'painters', 'Position', [10 10 1000 400]);

% Create axes
axes1 = axes('Parent',figure1);
hold(axes1,'on');

% Create multiple line objects using matrix input to plot
plot2 = plot(CH2_time*1e3, CH2data+65,'Parent',axes1);
plot1 = plot(CH3_time*1e3, CH3data*5+42,'Parent',axes1);
plot3 = plot(CH4_time*1e3, CH4data,'Parent',axes1);
plot4 = plot(CH5_time*1e3, CH5data-21,'Parent',axes1);

% % Create xlabel
 % xlabel({'Time (ms)'});
% 
% Uncomment the following line to preserve the Y-limits of the axes
% ylim(axes1,[-20 40]);
% xlim(axes1, [0.5 4])
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
    'XGrid','on','XMinorGrid','on','XTick',[-40 -20 0 20 40 60 80 100 120 140 160 180 200],...
    'XTickLabel',{},...
    'YGrid','on','YMinorGrid','on','YTick',[-20:5:85],'YTickLabel', {});

ylim(axes1,[-20 85]);

% Create legend
% legend1 = legend(axes1,'show');
% set(legend1,...
%     'Position',[0.866443454010762 0.333134929504662 0.122767855180427 0.359523799234913],...
%     'EdgeColor','none');

