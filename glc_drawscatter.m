% glc_drawscatter(XYWH, LIMS_XXYY, VALUES [, POINTSZ [, OPTS]])
% Draws a scatter plot in the axes defined by XYWH with given data limits.
%
% XYWH: a 4-vector (x,y, w,h) denoting the position and extent of the axes
% LIMS_XXYY: data limits (x1,x2, y1,y1)
% VALUES: a matrix of size 2-by-#values. VALUES(1,:) will be plotted on the x
%  axis against VALUES(2,:) on the y axis as separate GL.POINTS
% POINTSZ: the point size in pixels
% OPTS: a struct passed to the glcall(glc.draw, ...) call for the VALUES.
function glc_drawscatter(xywh, lims_xxyy, values, pointsz, varargin)
    global GL glc

    glc_assert(isnumeric(lims_xxyy) && numel(lims_xxyy)==4, ...
               'LIMS_XYXY must be a numeric 4-vector')
    glc_assert(isnumeric(values) && size(values,1)==2, ...
               'VALUES must be a numeric matrix with two rows')

    if (nargin < 4)
        pointsz = 1;
    else
        glc_assert(isnumeric(pointsz) && numel(pointsz)==1, ...
                   'POINTSZ must be a numeric scalar')
    end

    glc_axes_setup(xywh, [lims_xxyy(:).' -1 1]);

    if (pointsz ~= 1)
        opointsz = glcall(glc.get, GL.POINT_SIZE);
        glcall(glc.set, GL.POINT_SIZE, pointsz);
        otog = glcall(glc.toggle, [GL.POINT_SMOOTH 1, GL.BLEND 1]);
    end

    glcall(glc.draw, GL.POINTS, values, varargin{:});

    if (pointsz ~= 1)
        glcall(glc.set, GL.POINT_SIZE, opointsz);
        glcall(glc.toggle, otog);
    end

    glc_axes_finish([0.2 0.2 0.2]);
end
