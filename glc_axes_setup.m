% T = GLC_AXES_SETUP(VIEWPORT_XYWH, PROJECTION)
%
% TODO: write doc
%
%  VIEWPORT_XYWH should be [X Y W H] of the viewport
%  PROJECTION is passed to glcall(glc.setmatrix, ...)
%
%  T -- true.
function t = glc_axes_setup(viewport_xywh, projection)
    global glc GL

    global glc_viewport_xywh
    glc_viewport_xywh = viewport_xywh;

    % pushing ENABLE_BIT: convenience
    glcall(glc.push, [GL.PROJECTION GL.MODELVIEW GL.VIEWPORT_BIT+GL.SCISSOR_BIT+GL.ENABLE_BIT]);

    % setup
    glcall(glc.viewport, viewport_xywh);
    glcall(glc.setmatrix, GL.PROJECTION, projection);
    glcall(glc.setmatrix, GL.MODELVIEW, []);
    glcall(glc.toggle, [GL.SCISSOR_TEST 1]);
    glcall(glc.scissor, int32(viewport_xywh));

    t = true;
end
