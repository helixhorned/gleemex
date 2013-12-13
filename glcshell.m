% Gleemex shell.
%
function glcshell(data)
    global glcsh GL glc

    glc = glcall();
    GL = glconstants();

    glcsh = struct();

    glcsh.mxy = [0 0];
    glcsh.wh = [400 600];

    % App data
    glcsh.el = GLCEditableLine(true);

    % create the window!
    winid = glcall(glc.newwindow, [20 20], glcsh.wh, 'Gleemex shell');

    glcall(glc.setcallback, glc.cb_reshape, 'sh_reshape');
    glcall(glc.setcallback, glc.cb_display, 'sh_display');
    glcall(glc.setcallback, glc.cb_keyboard, 'sh_keyboard');
    glcall(glc.setcallback, glc.cb_motion, 'sh_motion');
    glcall(glc.setcallback, glc.cb_mouse, 'sh_mouse');
    glcall(glc.entermainloop);
end


function sh_reshape(w, h)
    global glcsh GL glc

    glcsh.wh = [w h];

    glc_setup2d(w, h);
end

function sh_display()
    global glcsh GL glc

    glcall(glc.clear, 1-[0 0 0]);

    w = glcsh.wh(1);
    glcsh.el.display([20 w-20], 20+16, struct('lineheight',16, 'colors',[.9 .9 .9]));
end

function sh_keyboard(key, x, y, mods)
    global glcsh GL glc

    doquit = glcsh.el.handleKey(key, mods);
    if (doquit)
        if (doquit == 1)
            glcall(glc.closewindow);
        else
            glcall(glc.leavemainloop);
        end
        return
    end

    glcall(glc.redisplay);
end

function sh_motion(buttonsdown, x, y)
    global glcsh GL glc

    % invert y
    glcsh.mxy = [x glcsh.wh(2)-y];

    % ...

    glcall(glc.redisplay);
end

function sh_mouse(button, downp, x, y, mods)
    global glcsh GL glc

    % ...
end
