clear
clc
close all

%Parameters
global IMGWIDTH DEGMAXS DEGSPACING INTERPOLATION GOAL MAX_SPEED;
GOAL = [0 50 0];
INTERPOLATION = 'bicubic';
IMGWIDTH = 40; %pixel width 
MAX_SPEED = 40;
DEGMAXS = ceil(linspace(10, 20, MAX_SPEED+1));
DEGSPACING = 1;

% close any open ports
sopen = instrfind;      % checks for already open serial ports
if max(size(sopen))>0   % if there are open serial ports
    fclose(sopen);      % they are closed
end
x = 0;
y = 0;
theta = 0;
position = [x y theta];

cam = webcam(1);
cam.Resolution = '160x120';

arduino=serial('COM7','BaudRate',115200); % create serial communication object on port COM4
fopen(arduino); % initiate arduino communication 
UNO_prompt= fscanf(arduino,'%c'); % fscanf waits until data terminated with new line occurs
disp(UNO_prompt)

img1 = snapshot(cam);
img1 = rgb2gray(img1);
images = {img1};
img1 = imresize(img1, [IMGWIDTH NaN]);
img1 = imadjust(img1);
img1 = single(img1);
img1 = img1 - mean(mean(img1));

eps = 3;
joy_x = 128;

while norm(position(1:2) - GOAL(1:2)) > eps
    img2 = snapshot(cam);
    img2 = rgb2gray(img2);
    images = {images img2};
    img2 = imresize(img2, [IMGWIDTH NaN]);
    img2 = imadjust(img2);
    img2 = single(img2);
    img2 = img2 - mean(mean(img2));

    tic
    del = getDel(img1, img2, joy_x);
    x = x + del(1);
    y = y + del(2);
    theta = mod(theta + del(3), 360);
    img1 = img2;
    
    position = [x y theta]
    move = GOAL - position
    rad_t = deg2rad(-theta);
    joy_send(1) = move(1)*cos(rad_t) - move(2)*sin(rad_t);
    joy_send(2) = move(1)*sin(rad_t) + move(2)*cos(rad_t);
    if norm(joy_send) == 0 
        joy_send = [128 128] %you're at the goal! don't move
    else
        joy_send = joy_send / norm(joy_send);    
        [m i] = max(abs(move(1:2)));
        if m > abs(GOAL(i))
            speedratio = 1;
        else
            speedratio = m/GOAL(i);
        end
        joy_send = round(joy_send * MAX_SPEED * speedratio);
        joy_send = joy_send + 128
        if joy_send(2) < 128 %don't go backwards
            joy_send = [140 128]
        end
    end
    %SEND JOY_SEND to UNO
	fprintf(arduino,'%d\n', joy_send(1)); 
    fprintf(arduino, '%d\n', joy_send(2));
    %RETRIEVE JOY_X VALUE
    %joy_x = fscanf(arduino,'%d/n');
    joy_x = joy_send(1); %if i'm the only one driving
    %joy_y
    toc
end
fclose(arduino);

function del = getDel(img1, img2, joy_x)
    global IMGWIDTH DEGMAXS DEGSPACING INTERPOLATION;
    %determining the degree search space based on joystick signal
    if joy_x > 128 %right turn
        DEGMAX = 0;
        DEGMIN = -DEGMAXS(joy_x - 128 + 1);
    elseif joy_x < 128 %left turn
        DEGMAX = DEGMAXS(128 - joy_x + 1);
        DEGMIN = 0;
    else %forward
        DEGMIN = -5;
        DEGMAX = 5;
    end
    err_min = inf;
    thetadel = 0;
    xdel = 0;
    ydel = 0;
    
    for deg = DEGMIN:DEGSPACING:DEGMAX 
        rotimg = imrotate(img2,-deg, INTERPOLATION, 'crop');
        C = xcorr2(img1, rotimg); 
        [~, ind] = max(C(:));
        [ypeak, xpeak] = ind2sub(size(C),ind);
        xdel_temp = -(xpeak-size(rotimg,2)); 
        ydel_temp = -(ypeak-size(img1,1));
        rotimg = imtranslate(imrotate(img2, -deg, INTERPOLATION, 'crop'), [-xdel_temp, -ydel_temp], INTERPOLATION); %translate the image to that spot
        mask = rotimg == 0;
        err = norm(img1(~mask) - rotimg(~mask)) / length(img1(~mask));
        if err < err_min
            err_min = err;
            thetadel = deg;
            xdel = xdel_temp;
            ydel = ydel_temp;
        end
    end
    del = [-xdel, ydel, thetadel];
end
