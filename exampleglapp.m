function exampleglapp()
    global GL glc glex
    GL = getglconsts(); glc = glcall();
    glex = struct();

    glex.xbord = [60 460];
    glex.ybord = [40 440];
    glex.mxy = [0 0];
    glex.wh = [800 600];

    winid = glcall(glc.newwindow, [20 20], glex.wh, 'GLCALL test 1');

    glcall(glc.setcallback, glc.cb_reshape, 'reshape');
    glcall(glc.setcallback, glc.cb_display, 'display');
    glcall(glc.setcallback, glc.cb_passivemotion, 'passivemotion');
    glcall(glc.setcallback, glc.cb_mouse, 'mouse');

    glcall(glc.entermainloop);
end

function reshape(w, h)
    global GL glc glex

    glex.wh = [w h];

    aspect = w/h;

    glcall(glc.viewport, [0 0 w h]);
    glcall(glc.setmatrix, GL.PROJECTION, [0 w, 0 h, -1 1]);
%{
    if (aspect >= 1)
        glcall(glc.setmatrix, GL.PROJECTION, [-aspect aspect, -1 1, -1 1]);
    else
        glcall(glc.setmatrix, GL.PROJECTION, [-1 1, -1/aspect 1/aspect, -1 1]);
    end
%}
    glcall(glc.setmatrix, GL.MODELVIEW, []);
end

function display()
    global GL glc glex

    glcall(glc.clear, [0 0 0]);

    numchans = 16;

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
        verts(:, 2) = verts(:, 1) + [xdif-1; 0];
        verts(:, 3) = verts(:, 1) + [xdif-1; ydif-1];
        verts(:, 4) = verts(:, 1) + [0; ydif-1];

        glcall(glc.draw, GL.QUADS, verts, struct('colors',[0.4 0.4 0.8]));
    end
end

function passivemotion(x, y)
    global glc glex

    glex.mxy = [x y];
    glcall(glc.postredisplay);
end

function mouse(button, downp, x, y)
    passivemotion(x, y);
end
