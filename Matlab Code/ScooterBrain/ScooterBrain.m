% ScooterBrain.m
% A function to synthesize data from sensors and control the scooter

function ScooterBrain
global DEBUG;
DEBUG = 1;

brainInit



end

function brainInit %connect to devices and determine their function
global DEBUG;

BR = 115200; % set baud rate

% close any open ports
sopen = instrfind; % checks for already open serial ports
if max(size(sopen)) > 0 % if there are open serial ports
 fclose(sopen); % they are closed
end

comPorts = seriallist; % determine connected serial devices 

% use only the USB-connected Unos
if ismac
    index = find(contains(comPorts,'/dev/cu.usbmodem'));
elseif isunix
    index = find(contains(comPorts,'/dev/cu.usbmodem'));
elseif ispc
    index = find(contains(comPorts,'COM'));
end
comPorts = comPorts(index);

% Connect to devices, query to determine which is which
for i = 1:length(index)
    fopen(serial(comPorts(i),'BaudRate',BR));  %#ok<TNMLP>
end

if(DEBUG)
    instrfind('status','open')
end

end % end brainInit