function main
% BitBang.m works with BitBang_M.ino and BitBang_P.ino
clear                   % clears variable memory
format compact          % removes extra line feeds
global M P      %global variables
init_ML2UNO
Mstr = fscanf(M,'%s');
%disp(Mstr)  % read but do not display
MD_out = input('Sent character= ','s');
fprintf(M,'%s',MD_out);  % NOTE \n not needed and should not be used
Pstr = fscanf(P,'%s');
PD_in = fscanf(P,'%s');
fprintf(1,'Received char= %s\n',PD_in);
fclose(M);       % close serial port
fclose(P);       % close serial port
fclose('all');    %close figure
end

function init_ML2UNO
global M P
%global variables
% close any open ports
sopen = instrfind;      % checks for already open serial ports
if max(size(sopen))>0   % if there are open serial ports
    fclose(sopen);      % close them
end
BR = 115200; %input('baud rate= ');
% windows instruction
%M
comNum = 401111;
comStr = ['COM' num2str(comNum)]; % concatenate strings
% Mac OS instruction
%comStr = ['/dev/cu.usbmodem' num2str(comNum)]; % concatenate strings
M = serial(comStr,'BaudRate',BR);  % same COM# and BR as Monitor
fopen(M); % open serial port to UNO

%P
comNum = 401131;
%comStr = ['COM' num2str(comNum)]; % concatenate strings
% Mac OS instruction
comStr = ['/dev/cu.usbmodem' num2str(comNum)]; % concatenate strings
P = serial(comStr,'BaudRate',BR);  % same COM# and BR as Monitor
fopen(P); % open serial port to UNO
% opening a new serial connection restarts UNO - need 5 sec delay before communication starts
 fprintf(1,'Connecting *');
for i=1:4
    pause(1) % 4 seconds
    fprintf(1,'*');
end
fprintf(1,'\n');    % end delay - Connected to UNO
end
