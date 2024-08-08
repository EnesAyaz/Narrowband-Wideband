% load('triple_region_ch1.mat')
% CH1data=data; 
% CH1_time=time; 

load('triple_region_ch2.mat')
CH2data=data; 
CH2_time=time; 

load('triple_region_ch3.mat')
CH3data=data; 
CH3_time=time; 

load('triple_region_ch4.mat')
CH4data=data; 
CH4_time=time; 

load('triple_region_ch5.mat')
CH5data=data; 
CH5_time=time; 

load('triple_region_ch6.mat')
CH6data=data; 
CH6_time=time; 

clear horizontalUnits model  recordLength sampleInterval time data verticalUnits waveformType waveformSource zeroIndex

%%
N1=250000*0.7
N=250000*1
figure()
% plot(CH4_time*1e3, CH4data)
hold on; 
% plot(CH5_time*1e3, CH5data)
hold on; 
% plot(CH2_time(1:N)*1e3, CH2data(1:N))
% % % hold on; 
plot(CH3_time(N1:N)*1e3, CH3data(N1:N))
% % % hold on 
% plot(CH6_time*1e3, CH6data)

% rms(CH3data(1:N))
% rms(CH3data(N1:N))

rms(CH2data(1:N))
rms(CH2data(N1:N))

%%
figure()
plot(CH5_time*1e3, CH5data/2+12)
hold on; 
plot(CH4_time*1e3, CH4data+7)
hold on; 
plot(CH3_time*1e3, CH3data/4+6)
hold on 
plot(CH2_time*1e3, CH2data-1)
hold on; 
plot(CH6_time*1e3, CH6data/50+25)
% ylim([-5 5])
%%

figure1 = figure('Renderer', 'painters', 'Position', [10 10 1000 400]);

% Create axes
axes1 = axes('Parent',figure1);
hold(axes1,'on');

% Create multiple line objects using matrix input to plot
plot2 = plot(CH4_time*1e3, CH4data+7,'Parent',axes1);
plot1 = plot(CH5_time*1e3, CH5data/2+12,'Parent',axes1);
plot3 = plot(CH3_time*1e3, CH3data/4+6,'Parent',axes1);
plot4 = plot(CH2_time*1e3, CH2data-1,'Parent',axes1);
plot5 = plot(CH6_time*1e3, CH6data/50+25,'Parent',axes1);

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
% ylim(axes1,[-4 5]);
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
    'YGrid','on','YMinorGrid','on','YTick',[- 10-5 0 5 10 15 20 25],'YTickLabel', {});

ylim(axes1,[-6 30]);

% Create legend
% legend1 = legend(axes1,'show');
% set(legend1,...
%     'Position',[0.866443454010762 0.333134929504662 0.122767855180427 0.359523799234913],...
%     'EdgeColor','none');

