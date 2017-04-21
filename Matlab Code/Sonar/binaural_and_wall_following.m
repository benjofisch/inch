% Binaural sonar and wall-following sonar data reception
% A. Arkebauer
% 4/19/2017
% EENG 481

% %%% This program yields four current values to work with:
% rightWallDist (distance from the right edge of INCH to the wall on the right side)
% leftWallDist (distance from the left edge of INCH to the wall on the left side)
% position (whether the closest obstacle in front of INCH is to the left (=0), center (=1) or right(=2)
% range (distance to the closest obstacle in front of INCH)

clear all
format compact

% closes any open ports
sopen = instrfind;      % checks for already open serial ports
if max(size(sopen))>0   % if there are open serial ports
    fclose(sopen);      % they are closed
end
disp('Connect UNO.  Note port # and baudrate')
comNum = input('Enter COM# (1411 for left side, 1421 for right side). # = ');
comStr = ['COM' num2str(comNum)]; % concatenate strings
BR = 115200; %input('baud rate= (Open Monitor, observe baud value, CLOSE Monitor, enter value) ');
%
% Connects to UNO - use either Windows or Mac OS instruction
% windows instruction
% comStr = ['COM' num2str(comNum)]; % concatenate strings
% s = serial(comStr,'BaudRate',BR,'InputBufferSize',4096*4, ...
%     'Timeout',6000);  % Increase wait time before timeout error
% Mac OS instruction
comStr = ['/dev/cu.usbmodem' num2str(comNum)]; % concatenate strings
s = serial(comStr,'BaudRate',BR,'InputBufferSize',4096*4, 'Timeout',6000);  % same COM# and BR as Monitor
fopen(s) % open serial port to UNO
% opening a new serial connection restarts UNO
% need approx 3 second delay before communication starts
flushinput(s);          % clear previous inputs
fprintf(1,'Connecting *');
for i=1:3
    pause(1) % 3 seconds
    fprintf(1,'*');
end
fprintf(1,'\n');    % end delay
% Connected to UNO
%
% Transmit data to UNO
% read string from UNO
while(1) % this loop will run forever
    unoStr = fscanf(s);        % read entire char str terminated by LF from UNO
    disp(unoStr);              % display ASCII string in Command Window
    
    vals = regexp(unoStr,'\s\d+\s','match');
    
    rightWallDist = vals{1}(2:end-1); % distance of right wall from right edge of INCH
    leftWallDist = vals{2}(2:end-1); % distance of right wall from left edge of INCH
    position = vals{3}(2:end-1); % whether closest obstacle in front of INCH is to the left (0), center (1), or right (2)
    range = vals{4}(2:end-1); % distance of closest obstacle in front of INCH
    disp([rightWallDist, ' ', leftWallDist, ' ', position, ' ', range]);
    
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    
    % %%% At this point, have the four current values to work with:
    % rightWallDist (distance from the right edge of INCH to the wall on the right side)
    % leftWallDist (distance from the left edge of INCH to the wall on the left side)
    % position (whether the closest obstacle in front of INCH is to the left (=0), center (=1) or right(=2)
    % range (distance to the closest obstacle in front of INCH)
    
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
end

% release port when finished
fclose(s)       % close serial port
% if Cntrl-c terminates program - TYPE "fclose(s)" in the Command Window