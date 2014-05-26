% glc_drawscatter(XYWH, LIMS_XXYY, VALUES [, POINTSZ [, OPTS]])
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
        glcall(glc.toggle, [GL.POINT_SMOOTH 1]);
    end

    glcall(glc.draw, GL.POINTS, values, varargin{:});

    if (pointsz ~= 1)
        glcall(glc.set, GL.POINT_SIZE, opointsz);
    end

    glc_axes_finish([0.2 0.2 0.2]);
end
