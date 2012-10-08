% vertposns: (#verts, 2 or 3) matrix
function exampleglapp(vertposns)
    global GL glc glex

    GL = getglconsts(); glc = glcall();
    glex = struct();

    try
        OCTAVE_VERSION;
        im = uint8(100*randn(256,256,3));
    catch
        % reading this particular JPG crashes Octave at exit when run under gdb
        % as of 2012-09-23.
        im = imread('../connectivity_visualization/brain_area.jpg');
    end

    if (mod(size(im,1),2)==1)
        im(end+1, :, :) = im(end, :, :);
    end
    if (mod(size(im,2),2)==1)
        im(:, end+1, :) = im(:, end, :);
    end

    glex.im = permute(im, [3 2 1]);  % swap x/y, make rgb consecutive in mem

    glex.imwh = [size(glex.im,2) size(glex.im,3)];  % width, height
    glex.adjbbox = [60 40 460 440];  % adj-mat bbox, [x1 y1 x2 y2]
    glex.mxy = [0 0];
    glex.bdown = [false false false];  % which button is down: left, middle, right
    glex.togbtnstate = false;
    glex.bdownxy = [0 0];
    glex.wh = [1100 800];
    glex.randdat = randn(1,30);

    glex.shaderid = [];
    glex.shaderuniforms = [];
    glex.bounds = single([0.0 1.0]);

    nlotsa = 256000;
    glex.lotsofverts = single(rand(2,nlotsa))*600 + 100;
    glex.lotsofverts(1, :) = glex.lotsofverts(1, :)+100;
    glex.lotsofcolors = rand(3,nlotsa);
    glex.lotsofidxs64 = uint8(floor(rand(1,nlotsa)*64));

    if (nargin >= 1)
        posmin = min(vertposns);
        posmax = max(vertposns);
        glex.DBG_posminmax = [posmin; posmax];
        for i=1:3
            vertposns(:, i) = (vertposns(:, i)-posmin(i))./(posmax(i)-posmin(i));
        end
        glex.posns = vertposns(:, 1:2).';
        glex.zposns = vertposns(:, 3).';
    else
        glex.posns = rand(2, 16);
        glex.zposns = zeros(1, 16);
    end

    t = linspace(0,2*pi, 17);
    glex.circ17 = [0 cos(t(1:end))/2; 0 sin(t(1:end))/2];

    % menu test
    subsubmenu = struct('label','Very deep', 'cbfunc','ex_menucb3', ...
                        'entries',{{'Hello', 'world'}});
    submenu = struct('label','SubX ------> ', ...
                     'entries',{{'Sub1 qweeee', subsubmenu, 'Sub2 blaaaaaarg'}});
    menus = struct('cbfunc','ex_menucb');
    menus.entries = { 'Qwe Lupus', 'Asd Gallus', submenu, 'Gee Sus' };

    % init!
    winid = glcall(glc.newwindow, [20 20], glex.wh, 'GLCALL test 1', ...
                   struct('menus',menus));

    % XXX: treat errors between newwindow and entermainloop properly
    glex.tex = glcall(glc.newtexture, glex.im);
    glcall(glc.colormap, uint8(jet(256)' * 255));
    glcall(glc.setcallback, glc.cb_reshape, 'ex_reshape');
    glcall(glc.setcallback, glc.cb_display, 'ex_display');
    glcall(glc.setcallback, glc.cb_keyboard, 'ex_keyboard');
    glcall(glc.setcallback, glc.cb_motion, 'ex_motion');
    glcall(glc.setcallback, glc.cb_mouse, 'ex_mouse');

    glcall(glc.entermainloop);
end


function ex_menucb(label)
    % for root menu and submenu
    fprintf('Clicked menu entry %s.\n', label);
end

function ex_menucb3(label)
    % for subsubmenu
    fprintf('3: Clicked menu entry %s.\n', label);
end

function ex_reshape(w, h)
    global GL glc glex

%            '  gray = bounds[0] + gray*(bounds[1]-bounds[0]);' 10 ...
    if (isempty(glex.shaderid))
        shadersrc = [ ...
            '#version 120' 10 ...
            'uniform sampler1D cmap;' 10 ...
            'uniform sampler2D tex;' 10 ...
            'uniform float bounds[2] = {0.0, 1.0};' 10 ...
            'void main(void) {' 10 ...
            '  float gray;' 10 ...
            '  vec3 rgb;' 10 ...
            '  if (gl_TexCoord[0].s > 0.5 || gl_TexCoord[0].t > 0.5) {' 10 ...
            '    gray = texture2D(tex, gl_TexCoord[0].st).r;' 10 ...
            '    gray = (gray - bounds[0])/(bounds[1]-bounds[0]);' 10 ...
            '    rgb = texture1D(cmap, gray).rgb;' 10 ...
            '    gl_FragColor = vec4(rgb.r, rgb.g, rgb.b, 1.0);' 10 ...
            '  } else {' 10 ...
            '    rgb = texture2D(tex, gl_TexCoord[0].st).rgb;' 10 ...
            '    gray = (rgb.r+rgb.g+rgb.b)/3;' 10 ...
            '    gl_FragColor = vec4(gray, gray, gray, 1.0);' 10 ...
            '  }' 10 ...
            '}' 10];

        [glex.shaderid, glex.shaderuniforms] = glcall(glc.newfragprog, shadersrc);
    end

    glex.wh = [w h];

    glc_setup2d(w, h);

%    aspect = w/h;
%    if (aspect >= 1)
%        glcall(glc.setmatrix, GL.PROJECTION, [-aspect aspect, -1 1, -1 1]);
%    else
%        glcall(glc.setmatrix, GL.PROJECTION, [-1 1, -1/aspect 1/aspect, -1 1]);
%    end
end

function ex_display()
    global GL glc glex

    %% resize check
    wh = glcall(glc.get, GL.WINDOW_SIZE);
    w = wh(1);
    h = wh(2);
    if (w < 800 || h < 600)
        % doesn't work very well
        w = max(800, glex.wh(1));
        h = max(600, glex.wh(2));
        glcall(glc.set, GL.WINDOW_SIZE, [w h]);
    end

    glcall(glc.clear, 1-[0 0 0]);

    % scissor test
    glcall(glc.toggle, [GL.SCISSOR_TEST 1]);
    glcall(glc.scissor, int32([64 64 128 128]));
    glcall(glc.clear, [0.2 0.2 0.8]);
    glcall(glc.toggle, [GL.SCISSOR_TEST 0]);

    % depth test
    glcall(glc.toggle, [GL.DEPTH_TEST 1, GL.POLYGON_SMOOTH 1, GL.BLEND 1]);
    glcall(glc.draw, GL.TRIANGLES, [80 190 280; 100 340 230; 0.5 0.5 0.5], struct('colors',[0.9 0.5 0.5]));
    glcall(glc.draw, GL.TRIANGLES, [180 220 260; 140 140 330; -0.5 -0.5 -0.5], struct('colors',[0.5 0.9 0.5]));
    glcall(glc.toggle, [GL.DEPTH_TEST 0, GL.POLYGON_SMOOTH 0, GL.BLEND 0]);

    numchans = 16;

    %% blue rect
    mxy = glex.mxy;

    if (glc_pointinrect(mxy, glex.adjbbox))
        xydif = glc_rectdiffs(glex.adjbbox)/numchans;

        ij = min(numchans-1, floor((mxy-glex.adjbbox([1 2]))./xydif));

        verts = glex.adjbbox(1:2) + xydif.*ij;
        glc_drawrect(verts, verts+xydif, struct('colors',[0.4 0.4 0.8]));
    end

    %% grid lines
    glc_draw_grid(glex.adjbbox, numchans+1);

    %% brain image
    texcoords = [1 0 0 1; ...
                 1 1 0 0];
    verts = [0 glex.imwh(1) glex.imwh(1) 0; ...
             0 0 glex.imwh(2) glex.imwh(2)];

    verts(1, :) = verts(1, :) + glex.adjbbox(1);
    verts(2, :) = verts(2, :) + glex.adjbbox(4) + 40;

    if (glex.togbtnstate)
        glcall(glc.usefragprog, glex.shaderid);
        glcall(glc.setuniform, glex.shaderuniforms.bounds, glex.bounds);
    end

    glcall(glc.draw, GL.QUADS, verts, struct(...
        'colors',[1 1 1], 'tex',glex.tex, 'texcoords',texcoords));

    if (glex.togbtnstate)
        glcall(glc.usefragprog);
    end

    %% vertex points
    vertposns = round(glex.posns * 256);
    vertposns(1, :) = vertposns(1, :) + glex.adjbbox(3) + 80;
    vertposns(2, :) = vertposns(2, :) + glex.adjbbox(2) + 30;

    nslices = 17;
    indices = uint32([ones(1,nslices); 1:nslices; 2:nslices+1]-1);
    indices = indices(:);

    glcall(glc.toggle, [GL.POINT_SMOOTH 1, GL.BLEND 1]);
    glcall(glc.set, GL.POINT_SIZE, 18);
    glcall(glc.draw, GL.POINTS, vertposns);
    glcall(glc.set, GL.POINT_SIZE, 1);
    glcall(glc.toggle, [GL.POINT_SMOOTH 0, GL.BLEND 0]);
%{
    for i=1:size(vertposns,2)
        vpos = glex.circ17 * 21;
        vpos(1, :) = vpos(1, :) + vertposns(1, i);
        vpos(2, :) = vpos(2, :) + vertposns(2, i);
        glcall(glc.draw, GL.TRIANGLES, vpos, struct(...
            'colors',[0.5 0.5 0.5] + glex.zposns(i)/2, 'indices',indices));
    end
%}

    glcall(glc.text, [64 460], 18, 'ABrainiac0123456789!@#$%^&*()_+', [-1 -1], struct('colors',[0 0 1]));

    for ang=0:30:180
        glcall(glc.push, GL.MODELVIEW);
        %    glcall(glc.mulmatrix, GL.MODELVIEW, [90, 0 0 1]);
        glcall(glc.mulmatrix, GL.MODELVIEW, [560 310 0]);  % translate text origin (lower left) to screen coords
        glcall(glc.mulmatrix, GL.MODELVIEW, [ang, 0 0 1]);  % rotate wrt origin
        glcall(glc.text, [20 0], 8+8*ang/180, sprintf('bounds: [%.01f %.01f]', glex.bounds(1), glex.bounds(2)));
        glcall(glc.pop, GL.MODELVIEW);
    end

    glc_drawbutton([80 400 120 20], 'Test Button', glex.mxy, glex.bdown(1));
    glc_drawbutton([80 430 120 20], 'Toggle Btn', glex.togbtnstate, false);

    %% 'axes' test
    tmpxywh = [450 540 200 180];
    glc_axes_setup(tmpxywh, [0 1, 0 1, -1 1]);  % {{{
    glcall(glc.toggle, [GL.LINE_SMOOTH 1, GL.BLEND 1]);

    glcall(glc.clear, [0.8 0.8 0.4]);
    glcall(glc.draw, GL.LINE_STRIP, [linspace(0,1,32); [0.5, 0.5+glex.randdat/5, 0.5]]);
    glex.randdat = [glex.randdat(2:30) randn(1)];

    glc_axes_finish([0 0 0]);  % }}}

    ocz_xy = tmpxywh(1:2)+[2 tmpxywh(4)-16];
    tmpargs = { ocz_xy, 14, 'OCz', [-1 -1], struct('xgap',1) };

    len = glcall(glc.text, tmpargs{:});  % get length of text
    glc_drawrect(ocz_xy+[-1 -2], ocz_xy+[len+2 14+2], struct('colors',[.6 .6 .6]));
    glcall(glc.text, tmpargs{:})  % draw text

    glcall(glc.text, ocz_xy-[0 16], 14, 'A monospaced text', ...
           [-1 -1], struct('mono',true, 'xgap',0));

    % torture test 1
%    glcall(glc.draw, GL.LINES, glex.lotsofverts, struct('colors', glex.lotsofcolors));
    
    % torture test 2
%    glcall(glc.draw, GL.LINES, glex.lotsofverts(:, 1:64), ...
%           struct('colors', glex.lotsofcolors(:, 1:64), 'indices',glex.lotsofidxs64));
end

function ex_keyboard(key, x, y, mods)
    global GL glc glex

    if (key=='q')
        glcall(glc.leavemainloop);
        return
    end

    dir = 0;
    idx = 2;

    if (key==GL.KEY_UP)
        dir = 1;
    elseif (key==GL.KEY_DOWN)
        dir = -1;
    end

    if (bitand(mods, GL.MOD_CTRL))
        idx = 1;
    end

    if (dir)
        obounds = glex.bounds;
        glex.bounds(idx) = glex.bounds(idx) + dir/10;
        if (glex.bounds(2) < glex.bounds(1))
            glex.bounds = obounds;
        else
            glex.bounds(1) = max(0, glex.bounds(1));
            glex.bounds(2) = min(1, glex.bounds(2));
        end
    end

    glcall(glc.redisplay);
end

function ex_motion(buttonsdown, x, y)
    global glc glex

    if (buttonsdown)
        % texture reuploading
        ov = (glex.im==245);
        glex.im = glex.im+1;
        glex.im(ov) = 0;

        glcall(glc.newtexture, glex.im, glex.tex);
    else
        glex.mxy = [x y];
        glex.mxy(2) = glex.wh(2)-glex.mxy(2);
    end

    glcall(glc.redisplay);
end

%function toggle_button()
%    global glex
%    glex.togbtnstate=~glex.togbtnstate;
%end

function ex_mouse(button, downp, x, y, mods)
    global GL glex

    ex_motion(0, x, y);

    glex.bdown(log2(button)+1) = downp;
    glex.bdownxy(1:2) = glex.mxy;

    if (button==GL.BUTTON_LEFT && downp)
        glc_callbutton([80 400 120 20], glex.mxy, 'glc_listdlg(''ListString'', {''qwe'',''asd'',''QWEQWE'',''ASDASD''}, ''Name'',''SDFG'');');
%        @uigetfile);
%        glc_callbutton([80 430 120 20], glex.mxy, @toggle_button);
        glc_callbutton([80 430 120 20], glex.mxy, 'global glex; glex.togbtnstate=~glex.togbtnstate;');
    end
end
