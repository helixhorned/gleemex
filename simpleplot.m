% SIMPLEPLOT(DATA [, IDXS [, COLORS [, KEYCB [, DISPLAYCB [, UD]]]]])
% Simple interactive plot.
%
% Legacy "simple" calling convention
% ----------------------------------
% DATA: 3 x numverts or length-#datasets cell with 3-by-numverts_i arrays each
% IDXS: zero-based, 3 x numfaces, matrix of triangle indices. If IDXS(1) is
%  negative, -IDXS is taken as a permutation on the dimensions of the data else.
%
% New "display list" calling convention
% -------------------------------------
% DATA: a cell of size N-by-3, with N > 1 and rows like
%  { GL.<PRIMITIVE_TYPE>, vertex_coords, struct(...) }
% Thus, each row contains arguments to glcall(glc.draw, ...).
function simpleplot(data, idxs, colors, keycb, displaycb, ud)
    % 'simpleplot' can be run with both classdef-enabled M-implementations or those
    % that don't have classdef.
    %  For classdef: 'simpl' is a cell array: {winid} -> GLCSimplePlotData handle
    %  For legacy: 'simpl' is a struct, making the plot a singleton

    global GL glc

    if (nargin < 1)
        % IDXS is 3 x #tris
        disp(['Usage: simpleplot(data [, idxs [, colors [, keycb]]]), ', ...
              'data is DIM x N, where DIM is either 2 or 3'])
        return
    end

    havelist = (iscell(data) && size(data, 1)>1);

    data_dims_perm = [];

    if (havelist)
        % Display list trumps all.
        assert(nargin == 1, 'When passing cell DATA with >1 rows, must have only this inarg')
    else
        if (nargin < 2)
            idxs = zeros(3,0,'uint32');
        else
            if (~isempty(idxs))
                if (idxs(1) < 0)
                    data_dims_perm = -idxs;
                    idxs = zeros(3,0,'uint32');
                else
                    if (~strcmp(class(idxs), 'uint32') && ~strcmp(class(idxs), 'uint8'))
                        error('IDXS must have class uint32 or uint8')
                    end

                    if (size(idxs, 1)~=3)
                        error('IDXS must have 3 rows')
                    end
                end
            end
        end

        if (nargin < 3)
            colors = zeros(3, 0);
        else
            if (~isempty(colors))
                if (~strcmp(class(colors), 'double') && ~iscell(colors))
                    error('COLORS must have class double or be a cell')
                end

                % size check: left to glcall
            end
        end

        if (nargin >= 4)
            if (~isa(keycb, 'function_handle'))
                error('KEYCB must be a function handle');
            end
        end

        if (nargin >= 5)
            if (~isa(keycb, 'function_handle'))
                error('DISPLAYCB must be a function handle');
            end
        end
    end

    glc = glcall();
    GL = glconstants();

    if (simpl_have_classdef())
        simpl = GLCSimplePlotData();
        va = { simpl };
    else
        global simpl
        simpl = struct();
        va = {};
    end

    simpl.havelist = havelist;

    if (~havelist)
        %% Argument pre-processing (convenience calling conventions)
        if (iscell(data))
            assert(isvector(data), 'Cell DATA must be a vector');

            if (iscell(colors))
                assert(isvector(colors), 'Cell COLORS must be a vector');
                assert(numel(data) == numel(colors), '#DATA must macht #COLORS');

                if (isvector(colors{1}))
                    cc = cell(1, numel(colors));
                    for i=1:numel(colors)
                        cc{i} = repmat(colors{i}(:), 1, size(data{i}, 2));
                    end
                    colors = [cc{:}];
                else
                    colors = [colors{:}];
                end
            end

            data = [data{:}];
        end

        if (~isempty(data_dims_perm))
            if (numel(data_dims_perm) ~= size(data, 1))
                error('DATA_DIMS_PERM (negated IDXS) must have as many elements as DATA has rows (2 or 3)')
            end
            data = data(data_dims_perm, :);
        end

        %% data-data
        simpl_setup_data(data, idxs, colors, va{:});
    else
        simpl_setup_data(data, [], [], va{:});
    end

    %% app data
    simpl.omx = -1;  % old mouse-x position, -1: invalid
    simpl.mxy = [0 0];
    simpl.wh = [1100 800];

%    sp_setup_axes_rect();
    % XXX: CODEDUP ^ v
    simpl.axxywh = [20 20 simpl.wh-40];
    simpl.axxywh(3:4) = max(simpl.axxywh(3:4), [400 300]);  % minimum 400 x 300 axes


    simpl.zoom = 1;
    simpl.ang = 0;  % angle around y axis in degrees
    simpl.fog = false;  % show fog?

    simpl.lineidxs = [1 1];  % beginning and end indices of show-as-line   XXX: blah
    simpl.keycb = [];
    if (nargin >= 4)
        simpl.keycb = keycb;
    end
    simpl.displaycb = [];
    if (nargin >= 5)
        simpl.displaycb = displaycb;
    end
    simpl.moretext = 'Test';

    if (nargin < 6)
        % no userdata
        simpl.ud = [];
    else
        simpl.ud = ud;
    end

    % create the window!
    winid = glcall(glc.newwindow, [20 20], simpl.wh, 'Simple plot');

    if (simpl_have_classdef())
        simpl_ = simpl;
        clear simpl;

        global simpl
        if (isempty(simpl))
            simpl = {};
        end

        assert(isequal(winid, simpl_getwin()), 'INTERNAL ERROR');
        simpl{winid} = simpl_;
    end

    glcall(glc.setcallback, glc.cb_reshape, 'sp_reshape');
    glcall(glc.setcallback, glc.cb_display, 'sp_display');
    glcall(glc.setcallback, glc.cb_keyboard, 'sp_keyboard');
    glcall(glc.setcallback, glc.cb_motion, 'sp_motion');
    glcall(glc.setcallback, glc.cb_mouse, 'sp_mouse');
    glcall(glc.entermainloop);
end


function sp_setup_axes_rect()
    if (simpl_have_classdef())
        simpl = simpl_getobj();
    else
        global simpl
    end

    % [x y w h]
    simpl.axxywh = [20 20 simpl.wh-40];
    simpl.axxywh(3:4) = max(simpl.axxywh(3:4), [400 300]);  % minimum 400 x 300 axes
end

function sp_reshape(w, h)
    global GL glc

    if (simpl_have_classdef())
        simpl = simpl_getobj();
    else
        global simpl
    end

    simpl.wh = [w h];
    sp_setup_axes_rect();

    glc_setup2d(w, h);
end

function sp_display()
    global GL glc

    if (simpl_have_classdef())
        simpl = simpl_getobj();
    else
        global simpl
    end

    glcall(glc.clear, 1-[0 0 0]);

    %% draw axes
    axwh = simpl.axxywh([3 4]);
    wbyhmpone = [-1 1]*axwh(1)/axwh(2);  % width-by-height minus-plus one
    glc_axes_setup(simpl.axxywh, [wbyhmpone, -1 1, 10*wbyhmpone], GL.DEPTH_BUFFER_BIT);  % {{{

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

    if (simpl.fog)
        glcall(glc.fog, GL.EXP2, single(0.7), single([1 1 1 0]));
        glcall(glc.toggle, [GL.FOG 1]);
    end

    if (simpl.havelist)
        % Successively draw all elements from the list.
        for i=1:size(simpl.data, 1)
            glcall(glc.draw, simpl.data{i, :});
        end
    elseif (isa(simpl.displaycb, 'function_handle'))
        % Run user-provided display callback.
        simpl.displaycb();
    elseif (~simpl.havelist)
        % Run our hard-coded (legacy) drawing code.
        glcall(glc.toggle, [GL.DEPTH_TEST 1]);
        if (simpl.lineidxs(2) > simpl.lineidxs(1))
            glcall(glc.draw, GL.LINE_STRIP, simpl.data(:, simpl.lineidxs(1):simpl.lineidxs(2)), ...
                   struct('colors',[.2 .2 .2]));
        end

        if (~isempty(simpl.idxs))
            opts = struct('indices',simpl.idxs(:));
            if (~isempty(simpl.colors))
                opts.colors = simpl.colors;
            end
            glcall(glc.draw, GL.TRIANGLES+16, simpl.data, opts);
        else
            if (~isempty(simpl.colors))
                va = { struct('colors', simpl.colors) };
            else
                va = {};
            end
            glcall(glc.draw, GL.POINTS, simpl.data, va{:});
        end
    end

    glc_axes_finish([0 0 0]); % }}}

    %% Display some text

    top = simpl.axxywh(2)+simpl.axxywh(4)-4;

    pretty = @(vec)regexprep(num2str(vec(:)'), ' +', ', ');

    glcall(glc.text, [28 top-14], 14, ...
           sprintf('means: %s | mins: %s | maxs: %s | fog: %d', pretty(simpl.mean), ...
                   pretty(simpl.min), pretty(simpl.max), simpl.fog));

    str = sprintf('zoom=%.02f, ang=%.02f', simpl.zoom, simpl.ang);
    if (~simpl.havelist)
        li = simpl.lineidxs;
        glcall(glc.text, [28 top-14-(8+14)], 14, ...
               sprintf('[%d  %d..%d  %d], %s', 1, li(1), li(2), simpl.numsamples, str));
    else
        glcall(glc.text, [28 top-14-(8+14)], 14, str);
    end

    glcall(glc.text, [28 top-14-2*(8+14)], 14, simpl.moretext);

    %% Coordinates of mouse pointer

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
    global GL glc

    if (simpl_have_classdef())
        simpl = simpl_getobj();
    else
        global simpl
    end

    % Common
    if (key==double('f'))
        simpl.fog = ~simpl.fog;
    end

    if (isa(simpl.keycb, 'function_handle'))
        simpl.keycb(key, x, y, mods);
    elseif (~simpl.havelist)
        mul = (1+9*(~~bitand(mods,GL.MOD_CTRL) + 6*(~~bitand(mods,GL.MOD_SHIFT))));
        switch key
          case { GL.KEY_RIGHT, GL.KEY_LEFT }
            simpl.lineidxs(2) = simpl.lineidxs(2) + mul*(1-2*(key==GL.KEY_LEFT));
            simpl.lineidxs(2) = sp_clamp(simpl.lineidxs(2), simpl.lineidxs(1),simpl.numsamples);
          case { GL.KEY_UP, GL.KEY_DOWN }
            simpl.lineidxs(1) = simpl.lineidxs(1) + mul*(1-2*(key==GL.KEY_DOWN));
            simpl.lineidxs(1) = sp_clamp(simpl.lineidxs(1), 1,simpl.lineidxs(2));
        end
    end

    glcall(glc.redisplay);
end

function sp_motion(buttonsdown, x, y)
    global GL glc

    if (simpl_have_classdef())
        simpl = simpl_getobj();
    else
        global simpl
    end

    % invert y
    simpl.mxy = [x simpl.wh(2)-y];

    if (simpl.numdims==3 && bitand(buttonsdown, GL.BUTTON_LEFT))
        if (simpl.omx == -1)
            simpl.omx = simpl.mxy(1);
        else
            dx = simpl.omx - simpl.mxy(1);
            newang = simpl.ang - dx/6;

            if (bitand(buttonsdown, GL.BUTTON_RIGHT))
                % RMB: rotate by whole degrees
                ok = round(simpl.ang) ~= round(newang);
                if (ok)
                    newang = round(newang);
                end
            else
                ok = true;
            end

            if (ok)
                simpl.omx = simpl.mxy(1);
                simpl.ang = mod(newang, 360);
            end
        end
    else
        simpl.omx = -1;
    end

    glcall(glc.redisplay);
end

function sp_mouse(button, downp, x, y, mods)
    global GL glc

    if (simpl_have_classdef())
        simpl = simpl_getobj();
    else
        global simpl
    end

    if (downp)
        switch button
          case GL.MWHEEL_UP,
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
