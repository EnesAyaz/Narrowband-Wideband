% load('wideband.mat')
 load('inverter_far.mat')
 % load('inverter_close.mat')
%%
sample_time=time(2)-time(1);
%%
% for i=1:recordLength
% if data(i)< -36
%     data(i)=-72;
% elseif data(i) > -36 && data(i)<32
%         data(i)=0;
% elseif data(i) > 32
%         data(i)=72;
% end
% end
%%
figure1 = figure('Renderer','painters', 'Position', [10 10 700 400]);
time2=0:sample_time:sample_time*(length(time)-1);
% Create axes
axes1 = axes('Parent',figure1);
hold(axes1,'on');

% Create plot
plot(time2*1e3,data,...
    'Color',[0 0 1]);

% Create ylabel
ylabel({'Voltage (V)'});

% Create xlabel
xlabel({'Time (ms)'});

% Uncomment the following line to preserve the X-limits of the axes
% xlim(axes1,[0 360]);
% Uncomment the following line to preserve the Y-limits of the axes
ylim(axes1,[-120 120]);
box(axes1,'on');
hold(axes1,'off');
% Set the remaining axes properties
set(axes1,'FontName','Times New Roman','FontSize',20,'GridAlpha',0.25,...
    'GridLineWidth',0.8,'MinorGridAlpha',0.15,'MinorGridLineStyle','-','XGrid',...
    'on','YGrid','on');


% %%
% ffund=100000;
% N1=round(1/sample_time/ffund);
% % 
% ffund=100000;
% N2=round(1/sample_time/ffund);
% N=N2;
% %%
% theta=linspace(0,360,N);
% figure1 = figure('Renderer','painters', 'Position', [10 10 700 400]);
% 
% % Create axes
% axes1 = axes('Parent',figure1);
% hold(axes1,'on');
% 
% % Create plot
% plot(theta,data(1:N),...
%     'Color',[0 0 1]);
% 
% % Create ylabel
% ylabel({'Voltage (V)'});
% 
% % Create xlabel
% xlabel({'Angle (^o)'});
% 
% % Uncomment the following line to preserve the X-limits of the axes
% xlim(axes1,[0 360]);
% % Uncomment the following line to preserve the Y-limits of the axes
% ylim(axes1,[-120 120]);
% box(axes1,'on');
% hold(axes1,'off');
% % Set the remaining axes properties
% set(axes1,'FontName','Times New Roman','FontSize',20,'GridAlpha',0.25,...
%     'GridLineWidth',0.8,'MinorGridAlpha',0.15,'MinorGridLineStyle','-','XGrid',...
%     'on','XTick',[0 60 120 180 240 300 360],'XTickLabel',...
%     {'0','60','120','180','240','300','360'},'YGrid','on');


%%
% i=1;
% N=recordLength-i-1;
% data2=data(i:N+i-1);
% data2=data(i:end);

% N2=100000;
% i=1;
% data2=data(i:N2+i-1);
% N=N2;
% recordLength2=N;

recordLength2=recordLength;
data2=data;

% recordLength2=recordLength;
% N=6250*10.2;
% i=1;
% data2=data(i:N);
% recordLength2=N-i+1;

 Fs = 1/(time(2)-time(1));    % Sampling frequency                   

 Y = fft(data2);

P2 = abs(Y/recordLength2);
P1 = P2(1:recordLength2/2+1);
P1(2:end-1) = 2*P1(2:end-1);
P1=P1/72;
f = Fs/recordLength2*(0:(recordLength2/2));
f=f/1000;

index= find(P1<0.02);

P1(index)=[]; 
f(index)=[];

% bar(f,P1)
% xlim([0 1200])

figure1 = figure('Renderer', 'painters', 'Position', [10 10 700 400]);
% Create axes
axes1 = axes('Parent',figure1);
hold(axes1,'on');

% Create bar
bar(f,P1,'LineWidth',1,'FaceColor',[0 0 0]);

% Create ylabel
ylabel('Normalized Magnitude','FontName','Times New Roman');

% Create xlabel
xlabel('Frequency (khZ)','FontName','Times New Roman');

% Uncomment the following line to preserve the X-limits of the axes
xlim(axes1,[0 2000]);
% Uncomment the following line to preserve the Y-limits of the axes
ylim(axes1,[0 1]);
box(axes1,'on');
hold(axes1,'off');
% Set the remaining axes properties
set(axes1,'FontName','Times New Roman','FontSize',20,'Xtick', [0, 400 :400: 2000 ]);
% Create textarrow
% Create textarrow
annotation(figure1,'textbox',...
    [0.672428571428571 0.7275 0.297571428571428 0.1475],...
    'String',{'\Delta\theta_{C} = 60 ^o','\Delta\theta_{R} = 120 ^o'},...
    'FontSize',20,...
    'FontName','Times New Roman',...
    'FitBoxToText','off',...
    'EdgeColor','none');

% Create textarrow
annotation(figure1,'textarrow',[0.532857142857143 0.585714285714285],...
    [0.4925 0.4925],'String',{'0.42 (expected 0.41)'},'FontSize',20,...
    'FontName','Times New Roman');

% Create textarrow
annotation(figure1,'textarrow',[0.216145833333333 0.177142857142857],...
    [0.664589823468328 0.665],'String',{'0.65 (expected 0.69)'},'FontSize',20,...
    'FontName','Times New Roman');


