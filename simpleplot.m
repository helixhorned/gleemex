% Simple plot.
function simpleplot(data, idxs, keycb)
    global simpl GL glc

    if (nargin < 1)
        % IDXS is 3 x #tris
        disp('Usage: simpleplot(data [, idxs [, keycb]]), data is DIM x N, where DIM is either 2 or 3')
        return
    end

    if (nargin < 2)
        idxs = zeros(3,0,'uint32');
    else
        if (~isempty(idxs))
            if (~strcmp(class(idxs), 'uint32') && ~strcmp(class(idxs), 'uint8'))
                error('IDXS must have class uint32 or uint8')
            end

            if (size(idxs, 1)~=3)
                error('IDXS must have 3 rows')
            end
        end
    end

    if (nargin >= 3)
        if (~isa(keycb, 'function_handle'))
            error('KEYCB must be a function handle');
        end
    end

    glc = glcall();
    GL = glconstants();


    simpl = struct();

    %% data-data
    simpl_setup_data(data, true, idxs);

    %% app data
    simpl.omx = -1;  % old mouse-x position, -1: invalid
    simpl.mxy = [0 0];
    simpl.wh = [1100 800];
    sp_setup_axes_rect();

    simpl.zoom = 1;
    simpl.ang = 0;  % angle around y axis in degrees

    simpl.lineidxs = [1 1];  % beginning and end indices of show-as-line   XXX: blah
    simpl.keycb = [];
    if (nargin >= 3)
        simpl.keycb = keycb;
    end
    simpl.moretext = 'Test';

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


function sp_setup_axes_rect()
    global simpl

    % [x y w h]
    simpl.axxywh = [20 20 simpl.wh-40];
    simpl.axxywh(3:4) = max(simpl.axxywh(3:4), [400 300]);  % minimum 400 x 300 axes
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
    axwh = simpl.axxywh([3 4]);
    wbyhmpone = [-1 1]*axwh(1)/axwh(2);  % width-by-height minus-plus one
    glc_axes_setup(simpl.axxywh, [wbyhmpone, -1 1, 10*wbyhmpone]);  % {{{

    sc = double(max((simpl.max-simpl.min)/2));
    scalemat = simpl.zoom.*eye(4)./sc;
    scalemat(4,4) = 1;
    glcall(glc.mulmatrix, GL.MODELVIEW, scalemat);

    glcall(glc.mulmatrix, GL.MODELVIEW, [simpl.ang, 0 1 0]);

    center = double((simpl.min+simpl.max)/2);
    glcall(glc.mulmatrix, GL.MODELVIEW, -center);

    mvpr = glcall(glc.get, GL.PROJECTION_MATRIX)*glcall(glc.get, GL.MODELVIEW_MATRIX);
    % inv(mvpr) :: clip coords -> object coords
    assert(mvpr(4,4)==1);


    if (simpl.lineidxs(2) > simpl.lineidxs(1))
        glcall(glc.draw, GL.LINE_STRIP, simpl.data(:, simpl.lineidxs(1):simpl.lineidxs(2)), ...
               struct('colors',[.2 .2 .2]));
    end
    glcall(glc.draw, GL.POINTS, simpl.data);

    if (~isempty(simpl.idxs))
        glcall(glc.draw, GL.TRIANGLES, simpl.data, struct('indices',simpl.idxs(:)));
    end

    glc_axes_finish([0 0 0]); % }}}

    top = simpl.axxywh(2)+simpl.axxywh(4)-4;

    pretty = @(vec)regexprep(num2str(vec(:)'), ' +', ', ');

    glcall(glc.text, [28 top-14], 14, ...
           sprintf('means: %s | mins: %s | maxs: %s', pretty(simpl.mean), ...
                   pretty(simpl.min), pretty(simpl.max)));
    glcall(glc.text, [28 top-14-(8+14)], 14, ...
           sprintf('[%d  %d..%d  %d], zoom=%.02f', 1, simpl.lineidxs(1), simpl.lineidxs(2), ...
                   simpl.numsamples, simpl.zoom));
    glcall(glc.text, [28 top-14-2*(8+14)], 14, simpl.moretext);

    [yes, fracs] = glc_pointinxywh(simpl.mxy, simpl.axxywh);
    if (simpl.numdims == 2 && yes)
        % FACTOROUT
        axcenter = simpl.axxywh([1 2])+axwh/2;
        % 2*fracs-axcenter: window coords -> normalized device coords:
        ndc = ([2*(simpl.mxy-axcenter) 0 1]);
        ndc(1:2) = ndc(1:2)./axwh;
        assert(abs(ndc) <= 1);
        mousedpos = mvpr \ ndc';

        glcall(glc.text, [28 top-14-3*(8+14)], 14, ...
               sprintf('(x, y) = (%.02f, %.02f)', mousedpos(1), mousedpos(2)));
    end
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

    if (isa(simpl.keycb, 'function_handle'))
        simpl.keycb(key, x, y, mods);
    end

    glcall(glc.redisplay);
end

function sp_motion(buttonsdown, x, y)
    global simpl GL glc

    % invert y
    simpl.mxy = [x simpl.wh(2)-y];

    if (simpl.numdims==3 && buttonsdown==GL.BUTTON_LEFT)
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

    glcall(glc.redisplay);
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

    simpl.zoom = sp_clamp(simpl.zoom, 0.1, 1000);

    glcall(glc.redisplay);
end
