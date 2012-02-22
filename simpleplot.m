% GLCALL application template, based on 'simpleplot'
%
% Usage:
%  - choose a unique global name (here, 'simpl')
%  - choose a convenient prefix (here, 'sp_')
%  - s/sp_/<your prefix>/g; s/simpl/<your global-name>/g
%
%  - if the application may have multiple running instances, follow <TODO: write doc>

function simpleplot(data)
    global simpl GL glc

    glc = glcall();
    GL = getglconsts();


    simpl = struct();

    %% data-data
    sp_setup_data(data);

    %% app data
    simpl.omx = -1;  % old mouse-x position, -1: invalid
    simpl.mxy = [0 0];
    simpl.wh = [1100 800];
    sp_setup_axes_rect();

    simpl.zoom = 1;
    simpl.ang = 0;  % angle around y axis in degrees

    simpl.lineidxs = [1 1];  % beginning and end indices of show-as-line   XXX: blah

    % create the window!
    winid = glcall(glc.newwindow, [20 20], simpl.wh, 'Simple plot');


    % Initialization requiring a GL context goes here...

    % e.g.
%    glcall(glc.colormap, uint8(jet(256)' * 255));


    glcall(glc.setcallback, glc.cb_reshape, 'sp_reshape');
    glcall(glc.setcallback, glc.cb_display, 'sp_display');
    glcall(glc.setcallback, glc.cb_keyboard, 'sp_keyboard');
    glcall(glc.setcallback, glc.cb_motion, 'sp_motion');
    glcall(glc.setcallback, glc.cb_mouse, 'sp_mouse');
    glcall(glc.entermainloop);
end


function sp_setup_data(data)
    global simpl

    %% validation
    if (~isnumeric(data))
        error('DATA must be numeric');
    end

    numdims = size(data, 1);
    if (numdims ~= 2 && numdims ~= 3)
        error('DATA must have 2 or 3 ''dimensions'' (i.e. length of 1st dim must be 2 or 3)');
    end

    simpl.numsamples = size(data, 2);
    simpl.data = single(data);

    if (numdims == 2)
        simpl.data(3, :) = 0;  % pad 3rd dim with zeros
    end
    simpl.mean = mean(simpl.data, 2);
    simpl.data = simpl.data - repmat(simpl.mean, 1,simpl.numsamples);
    % min and max of mean-subtracted data
    simpl.min = min(simpl.data, [], 2);
    simpl.max = max(simpl.data, [], 2);
end

function sp_setup_axes_rect()
    global simpl

    % [x y x y]
    simpl.axrect = [20 20 simpl.wh-40];
    simpl.axrect(3:4) = max(simpl.axrect(3:4), [420 320]);  % minimum 400 x 300 axes
end

function sp_reshape(w, h)
    global simpl GL glc

    simpl.wh = [w h];
    sp_setup_axes_rect();

    glc_setup2d(w, h);
end

function sp_display()
    global simpl GL glc

    glcall(glc.clear, 1-[0 0 0]);

    %% draw axes
    axwh = simpl.axrect([3 4])-simpl.axrect([1 2]);
    wbyhmpone = [-1 1]*axwh(1)/axwh(2);  % width-by-height minus-plus one
    glc_axes_setup(simpl.axrect, [wbyhmpone, -1 1, 10*wbyhmpone]);  % {{{

    maxmaxabs = max(max(abs(simpl.min), abs(simpl.max)));
    scalemat = simpl.zoom.*eye(4)./double(maxmaxabs);
    scalemat(4,4) = 1;
    glcall(glc.mulmatrix, GL.MODELVIEW, scalemat);

    glcall(glc.mulmatrix, GL.MODELVIEW, [simpl.ang, 0 1 0]);

    if (simpl.lineidxs(2) > simpl.lineidxs(1))
        glcall(glc.draw, GL.LINE_STRIP, simpl.data(:, simpl.lineidxs(1):simpl.lineidxs(2)), ...
               struct('colors',[.2 .2 .2]));
    end
    glcall(glc.draw, GL.POINTS, simpl.data);

    glc_axes_finish([0 0 0]); % }}}
end

function cval = sp_clamp(val, themin, themax)
    cval = min(max(themin, val), themax);
end

function sp_keyboard(key, x, y, mods)
    global simpl GL glc

    mul = (1+9*(~~bitand(mods,GL.MOD_CTRL) + 6*(~~bitand(mods,GL.MOD_SHIFT))));
    switch key
      case { GL.KEY_RIGHT, GL.KEY_LEFT }
        simpl.lineidxs(2) = simpl.lineidxs(2) + mul*(1-2*(key==GL.KEY_LEFT));
        simpl.lineidxs(2) = sp_clamp(simpl.lineidxs(2), simpl.lineidxs(1),simpl.numsamples);
      case { GL.KEY_UP, GL.KEY_DOWN }
        simpl.lineidxs(1) = simpl.lineidxs(1) + mul*(1-2*(key==GL.KEY_DOWN));
        simpl.lineidxs(1) = sp_clamp(simpl.lineidxs(1), 1,simpl.lineidxs(2));
    end

    glcall(glc.postredisplay);
end

function sp_motion(buttonsdown, x, y)
    global simpl GL glc

    % invert y
    simpl.mxy = [x simpl.wh(2)-y];

    if (buttonsdown==GL.BUTTON_LEFT)
        if (simpl.omx == -1)
            simpl.omx = simpl.mxy(1);
        else
            dx = simpl.mxy(1) - simpl.omx;
            simpl.omx = simpl.mxy(1);

            simpl.ang = simpl.ang - dx/6;

            simpl.ang = mod(simpl.ang, 360);
        end
    else
        simple.omx = -1;
    end

    glcall(glc.postredisplay);
end

function sp_mouse(button, downp, x, y, mods)
    global simpl GL glc

    if (downp)
        switch button
          case GL.MWHEEL_UP,
            % XXX: ctrl doesn't work here (linux)
            simpl.zoom = simpl.zoom * (1.2 + 0.3*(mods==GL.MOD_CTRL));
          case GL.MWHEEL_DOWN,
            simpl.zoom = simpl.zoom / (1.2 + 0.3*(mods==GL.MOD_CTRL));
        end
    else
        simpl.omx = -1;
    end

    simpl.zoom = min(max(1, simpl.zoom), 1000);

    glcall(glc.postredisplay);
end
