function exampleglapp()
    global GL glc glex

    GL = getglconsts(); glc = glcall();
    glex = struct();

    glex.xbord = [60 460];
    glex.ybord = [40 440];
    glex.mxy = [0 0];
    glex.wh = [800 600];
    glex.posns = rand(2, 16);

    t = linspace(0,2*pi, 17);
    glex.circ17 = [0 cos(t(1:end))/2; 0 sin(t(1:end))/2];

    winid = glcall(glc.newwindow, [20 20], glex.wh, 'GLCALL test 1', true);
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

function ex_display(qwe, asd)
    global GL glc glex
    glcall(glc.clear, [0 0 0]);

    numchans = 16;

    % blue rect
    mxy = glex.mxy;
    mxy(2) = glex.wh(2) - mxy(2);

    if (mxy(1)>=glex.xbord(1) && mxy(1)<=glex.xbord(2) ...
        && mxy(2)>=glex.ybord(1) && mxy(2)<=glex.ybord(2))

        xdif = (glex.xbord(2)-glex.xbord(1))/numchans;
        ydif = (glex.ybord(2)-glex.ybord(1))/numchans;

        i = floor((mxy(1)-glex.xbord(1))./xdif);
        j = floor((mxy(2)-glex.ybord(1))./ydif);

        verts = zeros(2, 4);

        verts(:, 1) = [glex.xbord(1) + i*xdif; glex.ybord(1) + j*ydif];
        verts(:, 2) = verts(:, 1) + [xdif; 0];
        verts(:, 3) = verts(:, 1) + [xdif; ydif];
        verts(:, 4) = verts(:, 1) + [0; ydif];

        glcall(glc.draw, GL.QUADS, verts, struct('colors',[0.4 0.4 0.8]));
    end

    % grid lines
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

    % vertex points
    vertposns = round(glex.posns * 256);
    vertposns(1, :) = vertposns(1, :) + glex.xbord(2) + 40;
    vertposns(2, :) = vertposns(2, :) + glex.ybord(1);

    nslices = 17;
    indices = uint32([ones(1,nslices); 1:nslices; 2:nslices+1]-1);
    indices = indices(:);

    for i=1:size(vertposns,2)
        vpos = glex.circ17 * 21;
        vpos(1, :) = vpos(1, :) + vertposns(1, i);
        vpos(2, :) = vpos(2, :) + vertposns(2, i);
        glcall(glc.draw, GL.TRIANGLES, vpos, struct(...
            'colors',[0.9 0.9 0.9], 'indices',indices));
    end
end

function ex_passivemotion(x, y)
    global glc glex

    glex.mxy = [x y];
    glcall(glc.postredisplay);
end

function ex_mouse(button, downp, x, y)
    ex_passivemotion(x, y);
end
