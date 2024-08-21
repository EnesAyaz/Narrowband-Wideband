 load('inverter_far.mat')

figure1 = figure('Renderer', 'painters', 'Position', [10 10 700 400]);
% Create axes
axes1 = axes('Parent',figure1);
hold(axes1,'on');

% Create bar
bar(time/1e3,data*0.0196,'LineWidth',1,'FaceColor',[0 0 0]);

% Create ylabel
ylabel('Normalized Magnitude','FontName','Times New Roman');

% Create xlabel
xlabel('Frequency (khZ)','FontName','Times New Roman');

% Uncomment the following line to preserve the X-limits of the axes
xlim(axes1,[0 2000]);
% Uncomment the following line to preserve the Y-limits of the axes
% ylim(axes1,[0 1]);
% box(axes1,'on');
% hold(axes1,'off');
% Set the remaining axes properties
set(axes1,'FontName','Times New Roman','FontSize',20);
% Create textarrow
% Create textarrow
hold(axes1,'off');
% Set the remaining axes properties
set(axes1,'FontName','Times New Roman','FontSize',20);
% Create textbox
annotation(figure1,'textbox',...
    [0.672428571428571 0.7275 0.297571428571428 0.1475],...
    'String',{'\Delta\theta_{C} = 60 ^o','\Delta\theta_{R} = 120 ^o'},...
    'FontSize',20,...
    'FontName','Times New Roman',...
    'FitBoxToText','off',...
    'EdgeColor','none');

% Create textarrow
annotation(figure1,'textarrow',[0.25 0.175714285714286],[0.7875 0.7875],...
    'String',{'0.81 (expected 0.80)'},...
    'FontSize',20,...
    'FontName','Times New Roman');

% Create textarrow
annotation(figure1,'textarrow',[0.521354166666667 0.4765625],...
    [0.494288681204569 0.4932502596054],'String',{'0.42 (expected 0.41)'},...
    'FontSize',20,...
    'FontName','Times New Roman');



