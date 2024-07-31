% load('wideband.mat')
load('narrowband.mat')
%%
sample_time=time(2)-time(1);
N=1428;
N=250000;
theta=linspace(0,360,N);
figure1 = figure('Renderer','painters', 'Position', [10 10 700 400]);

% Create axes
axes1 = axes('Parent',figure1);
hold(axes1,'on');

% Create plot
plot(theta,data(1:N),...
    'Color',[0.00784313725490196 0.545098039215686 0.901960784313726]);

% Create ylabel
ylabel({'Voltage (V)'});

% Create xlabel
xlabel({'Angle (^o)'});

% Uncomment the following line to preserve the X-limits of the axes
xlim(axes1,[0 360]);
% Uncomment the following line to preserve the Y-limits of the axes
ylim(axes1,[-80 80]);
box(axes1,'on');
hold(axes1,'off');
% Set the remaining axes properties
set(axes1,'FontName','Times New Roman','FontSize',20,'GridAlpha',0.25,...
    'GridLineWidth',0.8,'MinorGridAlpha',0.15,'MinorGridLineStyle','-','XGrid',...
    'on','XTick',[0 60 120 180 240 300 360],'XTickLabel',...
    {'0','60','120','180','240','300','360'},'YGrid','on');


%%
recordLength2=recordLength;
 Fs = 1/(time(2)-time(1));    % Sampling frequency                   

 Y = fft(data);

P2 = abs(Y/recordLength2);
P1 = P2(1:recordLength2/2+1);
P1(2:end-1) = 2*P1(2:end-1);
P1=P1;
f = Fs/recordLength2*(0:(recordLength2/2));
f=f/1000;

% index= find(P1<0.05);

% P1(index)=[]; 
% f(index)=[];

bar(f,P1)
xlim([0 1200])
%%

% figure1 = figure('Renderer', 'painters', 'Position', [10 10 700 400]);
% % Create axes
% axes1 = axes('Parent',figure1);
% hold(axes1,'on');
% 
% % Create bar
% bar(f,P1,'LineWidth',1,'FaceColor',[0 0 0]);
% 
% % Create ylabel
% ylabel('Normalized Magnitude','FontName','Times New Roman');
% 
% % Create xlabel
% xlabel('Harmonic Number (n)','FontName','Times New Roman');
% 
% % Uncomment the following line to preserve the X-limits of the axes
% xlim(axes1,[0 80]);
% % Uncomment the following line to preserve the Y-limits of the axes
% ylim(axes1,[0 1]);
% box(axes1,'on');
% hold(axes1,'off');
% % Set the remaining axes properties
% set(axes1,'FontName','Times New Roman','FontSize',20);
% % Create textarrow
% % Create textarrow
% annotation(figure1,'textbox',...
%     [0.672428571428571 0.7275 0.297571428571428 0.1475],...
%     'String',{'\Delta\phi_{C} = 60 ^o','\Delta\phi_{R} = 120 ^o'},...
%     'FontSize',20,...
%     'FontName','Times New Roman',...
%     'FitBoxToText','off',...
%     'EdgeColor','none');
% 
% % Create textarrow
% annotation(figure1,'textarrow',[0.415714285714286 0.367142857142857],...
%     [0.4775 0.405],'String',{'0.29 (expected 0.30)'},'FontSize',20,...
%     'FontName','Times New Roman');
% 
% % Create textarrow
% annotation(figure1,'textarrow',[0.338571428571429 0.342857142857143],...
%     [0.6175 0.42],'String',{'0.31 (expected 0.32)'},'FontSize',20,...
%     'FontName','Times New Roman');


