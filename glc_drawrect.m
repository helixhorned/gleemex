function glc_drawrect(xy1, xy2, varargin)
    global glc GL

    glcall(glc.draw, GL.QUADS, glc_expandrect([xy1(:) xy2(:)]), varargin{:});
end
