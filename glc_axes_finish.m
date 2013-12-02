% GLC_AXES_FINISH([BORDERCOLOR])
%
%
function glc_axes_finish(bordercolor)
    global glc GL

    glcall(glc.pop, [GL.PROJECTION GL.MODELVIEW GL.VIEWPORT_BIT+GL.SCISSOR_BIT+GL.ENABLE_BIT]);

    if (nargin > 0 && ~isempty(bordercolor))
        global glc_viewport_xywh
        xywh = glc_viewport_xywh;

        glcall(glc.draw, GL.LINE_LOOP, ...
               [xywh(1) xywh(1)+xywh(3) xywh(1)+xywh(3) xywh(1); ...
                xywh(2) xywh(2) xywh(2)+xywh(4) xywh(2)+xywh(4)], ...
               struct('colors',bordercolor));
    end
end
