clear
clc
close all

load('right.mat')

%Parameters
global IMGWIDTH DEGMAXS DEGSPACING INTERPOLATION GOAL MAX_SPEED;
GOAL = [0 50 0];
INTERPOLATION = 'bicubic';
IMGWIDTH = 40; %pixel width 
MAX_SPEED = 40;
DEGMAXS = ceil(linspace(10, 20, MAX_SPEED+1));
DEGSPACING = 1;

x = 0;
y = 0;
theta = 0;
position = [x y theta];

img1 = images{1};
img1 = imresize(img1, [IMGWIDTH NaN]);
img1 = imadjust(img1);
img1 = single(img1);
img1 = img1 - mean(mean(img1));

eps = 2;
joy_x = 168;

for i = 2:length(images)
    img2 = images{i};
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
    move = GOAL(1:2) - position(1:2)
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
        if joy_send(2) >= 128 
            joy_send = joy_send + 128
        else 
            joy_send = [128+MAX_SPEED 128]
        end
    end 
    toc
end

function del = getDel(img1, img2, joy_x)
    global DEGMAXS DEGSPACING INTERPOLATION;
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
