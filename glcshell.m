% Gleemex shell.
%
function glcshell(data)
    global GL glc

    glc = glcall();
    GL = glconstants();

    glcsh = GLCShellData();

    % create the window!
    winid = glcall(glc.newwindow, [20 20], glcsh.wh, 'Gleemex shell');

    GLCApplicationData.register(winid, glcsh);
    nset = GLCApplicationData.setCallbacks('sh_');
    assert(nset == 4);

    glcall(glc.entermainloop);
end


function sh_reshape(w, h)
    global GL glc
    glcsh = glc_appdata();

    glcsh.updateWindowPos(w, h, true);
end

function sh_display()
    global GL glc
    glcsh = glc_appdata();

    glcall(glc.clear, 1-[0 0 0]);

    w = glcsh.wh(1);
    glcsh.el.display([20 w-20], 20+16, struct('lineheight',16, 'colors',[.9 .9 .9]));
end

function sh_keyboard(key, x, y, mods)
    global GL glc
    glcsh = glc_appdata();

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
    global GL glc
    glcsh = glc_appdata();

    glcsh.updateMousePos(x, y);

    glcall(glc.redisplay);
end

%{
function sh_mouse(button, downp, x, y, mods)
    global glcsh GL glc

    % ...
end
%}
