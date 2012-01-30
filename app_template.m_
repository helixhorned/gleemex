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

    simpl.mxy = [0 0];
    simpl.wh = [1100 800];
    % more constant init...


    % create the window!
    winid = glcall(glc.newwindow, [20 20], simpl.wh, 'Simple plot');


    % Initialization requiring a GL state goes here...

    % e.g.
%    glcall(glc.colormap, uint8(jet(256)' * 255));


    glcall(glc.setcallback, glc.cb_reshape, 'sp_reshape');
    glcall(glc.setcallback, glc.cb_display, 'sp_display');
    glcall(glc.setcallback, glc.cb_keyboard, 'sp_keyboard');
    glcall(glc.setcallback, glc.cb_motion, 'sp_motion');
    glcall(glc.setcallback, glc.cb_mouse, 'sp_mouse');
    glcall(glc.entermainloop);
end


function sp_reshape(w, h)
    global simpl GL glc

    simpl.wh = [w h];

    glc_setup2d(w, h);
end

function sp_display()
    global simpl GL glc

    glcall(glc.clear, 1-[0 0 0]);
end

function sp_keyboard(key, x, y, mods)
    global simpl GL glc

    % ...

    glcall(glc.postredisplay);
end

function sp_motion(buttonsdown, x, y)
    global simpl GL glc

    % invert y
    simpl.mxy = [x simpl.wh(2)-y];

    % ...

    glcall(glc.postredisplay);
end

function sp_mouse(button, downp, x, y, mods)
    global simpl GL glc

    % ...
end
