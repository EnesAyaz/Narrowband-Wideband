% spice_data_reader.m

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
    'Position',[0.159321462802681 0.486224677036159 0.159999997764827 0.268199226497027],...
    'EdgeColor','none',...
    'Color','none');

% Create textarrow
annotation(figure1,'textarrow',[0.430013404825738 0.498378016085793],...
    [0.830249042145594 0.830249042145594],'Color',[0 0 1],'TextColor',[0 0 1],...
    'String',{'1.29'},...
    'FontSize',15,...
    'FontName','Times New Roman');

% Create textarrow
annotation(figure1,'textarrow',[0.669052725647903 0.622135835567474],...
    [0.830766283524902 0.834597701149424],'Color',[1 0 0],'TextColor',[1 0 0],...
    'String',{'1.25'},...
    'FontSize',15,...
    'FontName','Times New Roman');

% Create textarrow
annotation(figure1,'textarrow',[0.686407506702415 0.627426273458447],...
    [0.472950191570881 0.46911877394636],'Color',[0 0 1],'TextColor',[0 0 1],...
    'String',{'0.38'},...
    'FontSize',15,...
    'FontName','Times New Roman');

% Create textarrow
annotation(figure1,'textarrow',[0.435630026809652 0.514718498659519],...
    [0.462298850574713 0.458467432950192],'Color',[1 0 0],'TextColor',[1 0 0],...
    'String',{'0.32'},...
    'FontSize',15,...
    'FontName','Times New Roman');
