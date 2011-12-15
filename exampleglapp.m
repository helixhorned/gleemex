% vertposns: (#verts, 2 or 3) matrix
function exampleglapp(vertposns)
    global GL glc glex

    GL = getglconsts(); glc = glcall();
    glex = struct();

    im = imread('../connectivity_visualization/brain_area.jpg');
    if (mod(size(im,1),2)==1)
        im(end+1, :, :) = im(end, :, :);
    end
    if (mod(size(im,2),2)==1)
        im(:, end+1, :) = im(:, end, :);
    end

    glex.im = permute(im, [3 2 1]);  % swap x/y, make rgb consecutive in mem

    glex.imwh = [size(glex.im,2) size(glex.im,3)];  % width, height
    glex.xbord = [60 460];
    glex.ybord = [40 440];
    glex.mxy = [0 0];
    glex.wh = [1100 800];

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

    % init!
    winid = glcall(glc.newwindow, [20 20], glex.wh, 'GLCALL test 1', false);

    % XXX: treat errors between newwindow and entermainloop properly
    glex.tex = glcall(glc.newtexture, glex.im);
    glcall(glc.setcallback, glc.cb_reshape, 'ex_reshape');
    glcall(glc.setcallback, glc.cb_display, 'ex_display');
    glcall(glc.setcallback, glc.cb_passivemotion, 'ex_passivemotion');
    glcall(glc.setcallback, glc.cb_mouse, 'ex_mouse');
    glcall(glc.entermainloop);
end


function ex_reshape(w, h)
    global GL glc glex

    glex.wh = [w h];

    aspect = w/h;

    glcall(glc.viewport, [0 0 w h]);
    glcall(glc.setmatrix, GL.PROJECTION, [0 w, 0 h, -1 1]+0.5);

%    if (aspect >= 1)
%        glcall(glc.setmatrix, GL.PROJECTION, [-aspect aspect, -1 1, -1 1]);
%    else
%        glcall(glc.setmatrix, GL.PROJECTION, [-1 1, -1/aspect 1/aspect, -1 1]);
%    end

    glcall(glc.setmatrix, GL.MODELVIEW, []);
end

function ex_display()
    global GL glc glex

    %% resize check
    wh = glcall(glc.getwindowsize);
    w = wh(1);
    h = wh(2);
    if (w < 800 || h < 600)
        % doesn't work very well
        w = max(800, glex.wh(1));
        h = max(600, glex.wh(2));
        glcall(glc.setwindowsize, [w h]);
    end

    glcall(glc.clear, 1-[0 0 0]);

    % scissor test
    glcall(glc.toggle, [GL.SCISSOR_TEST 1]);
    glcall(glc.scissor, int32([64 64 128 128]));
    glcall(glc.clear, [0.2 0.2 0.8]);
    glcall(glc.toggle, [GL.SCISSOR_TEST 0]);

    % depth test
    glcall(glc.toggle, [GL.DEPTH_TEST 1]);
    glcall(glc.draw, GL.TRIANGLES, [80 190 280; 100 340 230; 0.5 0.5 0.5], struct('colors',[0.9 0.5 0.5]));
    glcall(glc.draw, GL.TRIANGLES, [180 220 260; 140 140 330; -0.5 -0.5 -0.5], struct('colors',[0.5 0.9 0.5]));
    glcall(glc.toggle, [GL.DEPTH_TEST 0]);

    numchans = 16;

    %% blue rect
    mxy = glex.mxy;
    mxy(2) = glex.wh(2) - mxy(2);

    if (mxy(1)>=glex.xbord(1) && mxy(1)<=glex.xbord(2) ...
        && mxy(2)>=glex.ybord(1) && mxy(2)<=glex.ybord(2))

        xdif = (glex.xbord(2)-glex.xbord(1))/numchans;
        ydif = (glex.ybord(2)-glex.ybord(1))/numchans;

        i = min(numchans-1, floor((mxy(1)-glex.xbord(1))./xdif));
        j = min(numchans-1, floor((mxy(2)-glex.ybord(1))./ydif));

        verts = zeros(2, 4);

        verts(:, 1) = [glex.xbord(1) + i*xdif; glex.ybord(1) + j*ydif];
        verts(:, 2) = verts(:, 1) + [xdif; 0];
        verts(:, 3) = verts(:, 1) + [xdif; ydif];
        verts(:, 4) = verts(:, 1) + [0; ydif];

        glcall(glc.draw, GL.QUADS, verts, struct('colors',[0.4 0.4 0.8]));
    end

    %% grid lines
    xx = linspace(glex.xbord(1), glex.xbord(2), numchans+1);
    yy = linspace(glex.ybord(1), glex.ybord(2), numchans+1);

    vlines = zeros(2, 2*(numchans+1));
    vlines(1, :) = repmat(glex.xbord, 1,(numchans+1));
    vlines(2, 1:2:end-1) = yy;
    vlines(2, 2:2:end) = yy;

    hlines = zeros(2, 2*(numchans+1));
    hlines(2, :) = repmat(glex.ybord, 1,(numchans+1));
    hlines(1, 1:2:end-1) = xx;
    hlines(1, 2:2:end) = xx;

    glcall(glc.draw, GL.LINES, [vlines, hlines]);

    %% brain image
    texcoords = [1 0 0 1; ...
                 1 1 0 0];
    verts = [0 glex.imwh(1) glex.imwh(1) 0; ...
             0 0 glex.imwh(2) glex.imwh(2)];

    verts(1, :) = verts(1, :) + glex.xbord(1);
    verts(2, :) = verts(2, :) + glex.ybord(2) + 40;

    glcall(glc.draw, GL.QUADS, verts, struct(...
        'colors',[1 1 1], 'tex',glex.tex, 'texcoords',texcoords));

    %% vertex points
    vertposns = round(glex.posns * 256);
    vertposns(1, :) = vertposns(1, :) + glex.xbord(2) + 80;
    vertposns(2, :) = vertposns(2, :) + glex.ybord(1) + 30;

    nslices = 17;
    indices = uint32([ones(1,nslices); 1:nslices; 2:nslices+1]-1);
    indices = indices(:);

    for i=1:size(vertposns,2)
        vpos = glex.circ17 * 21;
        vpos(1, :) = vpos(1, :) + vertposns(1, i);
        vpos(2, :) = vpos(2, :) + vertposns(2, i);
        glcall(glc.draw, GL.TRIANGLES, vpos, struct(...
            'colors',[0.5 0.5 0.5] + glex.zposns(i)/2, 'indices',indices));
    end

    glcall(glc.rendertext, [64 460 0], 18, 'ABrainiac0123456789!@#$%^&*()_+');

    % torture test 1
%    glcall(glc.draw, GL.LINES, glex.lotsofverts, struct('colors', glex.lotsofcolors));
    
    % torture test 2
%    glcall(glc.draw, GL.LINES, glex.lotsofverts(:, 1:64), ...
%           struct('colors', glex.lotsofcolors(:, 1:64), 'indices',glex.lotsofidxs64));
end

function ex_passivemotion(x, y)
    global glc glex

    glex.mxy = [x y];

%    glex.im = glex.im+1;
%    glcall(glc.newtexture, glex.im, glex.tex);

    glcall(glc.postredisplay);
end

function ex_mouse(button, downp, x, y, mods)
    ex_passivemotion(x, y);
end
