figure1 = figure('Renderer', 'painters', 'Position', [0 0 600 200])

% Create axes
axes1 = axes('Parent',figure1);
hold(axes1,'on');

% Create multiple line objects using matrix input to plot
plot1 = plot(Freq/1e3,db2mag(Vout1),'LineWidth',2);
plot2 = plot(Freq/1e3,db2mag(Vout2),'LineWidth',2);
set(plot1,'DisplayName','Vout1','Color',[0 0 1]);
set(plot2,'DisplayName','Vout2','Color',[1 0 0]);

% Create ylabel
ylabel({'Normalized Gain'});

% Create xlabel
xlabel({'Frequency (kHz)'});

% Uncomment the following line to preserve the X-limits of the axes
xlim(axes1,[0 2000]);
ylim(axes1,[0 1.5])
box(axes1,'on');
hold(axes1,'off');
% Set the remaining axes properties
set(axes1,'FontName','Times New Roman','FontSize',15,'XTick',...
    [0 250 500 750 1000 1250 1500 1750 2000]);
% Create legend
legend1 = legend(axes1,'show');
set(legend1,...
    'Position',[0.694762292430806 0.626224677036159 0.135785005175241 0.268199226497027],...
    'EdgeColor','none',...
    'Color','none');

% Create legend
legend1 = legend(axes1,'show');
set(legend1,...
    'Position',[0.682654796136014 0.626224677036159 0.159999997764826 0.268199226497027],...
    'EdgeColor','none',...
    'Color','none');

% % Create textarrow
% annotation(figure1,'textarrow',[0.213333333333333 0.18],[0.37 0.31],...
%     'Color',[1 0 0],...
%     'TextColor',[1 0 0],...
%     'String',{'0.32'},...
%     'FontSize',15,...
%     'FontName','Times New Roman');

% % Create textarrow
% annotation(figure1,'textarrow',[0.521666666666667 0.605],[0.385 0.32],...
%     'Color',[0 0 1],...
%     'TextColor',[0 0 1],...
%     'String',{'0.38'},...
%     'FontSize',15,...
%     'FontName','Times New Roman');

% Create textarrow
annotation(figure1,'textarrow',[0.516666666666667 0.583333333333333],...
    [0.785 0.785],'Color',[1 0 0],'TextColor',[1 0 0],'String',{'1.16'},...
    'FontSize',15,...
    'FontName','Times New Roman');

% Create textarrow
annotation(figure1,'textarrow',[0.231666666666667 0.178333333333334],...
    [0.715 0.715],'Color',[0 0 1],'TextColor',[0 0 1],'String',{'0.94'},...
    'FontSize',15,...
    'FontName','Times New Roman');
