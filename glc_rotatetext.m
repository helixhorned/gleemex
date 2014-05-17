% GLC_ROTATETEXT(XY_ORIGIN, ANG, OFS, HEIGHT, LABEL [, ORIGIN_INTERPR])
%
% Parameters from OFS on are passed directly to glcall(glc.text, ...)
function glc_rotatetext(xy_origin, ang, ofs, height, label, varargin)
    global glc GL

    assert(isnumeric(xy_origin) && numel(xy_origin)==2, 'XY_ORIGIN must be a numeric pair')
    assert(isnumeric(ang) && numel(ang)==1, 'ANG must be a numeric scalar')

    glcall(glc.push, GL.MODELVIEW);
    glcall(glc.mulmatrix, GL.MODELVIEW, [xy_origin(:).' 0]);  % translate text origin
    glcall(glc.mulmatrix, GL.MODELVIEW, [ang, 0 0 1]);  % rotate wrt origin
    glcall(glc.text, ofs, height, label, varargin{:});
    glcall(glc.pop, GL.MODELVIEW);
end
