% glc_drawticks(XYXY, LIMS, VALS [, TICKLEN [, TEXTSIZE [, TICKFACTORS [, OPTS_DATA]]]])
function glc_drawticks(xyxy, lims, vals, ticklen, textsize, tickfactors, opts_data)
    assert(isnumeric(xyxy) && numel(xyxy)==4, 'XYXY must be a numeric 4-array')
    assert(isnumeric(lims) && numel(lims)==2, 'LIMS must be a numeric pair')
    assert(isnumeric(vals) && isvector(vals) && numel(vals) >= 2, ...
           'VALS must be a numeric vector of length at least 2')
    assert(lims(1) < lims(2), 'LIMS(2) must be greater than LIMS(1)');

    if (nargin < 4)
        ticklen = 1;
    else
        assert(isnumeric(ticklen) && numel(ticklen)==1, 'TICKLEN must be a numeric scalar')
    end

    numvals = numel(vals);

    if (nargin < 5)
        textsize = 0;
    else
        assert(isnumeric(textsize) && numel(textsize)==1, 'TEXTSIZE must be a numeric scalar')
    end

    if (nargin < 6)
        tickfactors = ones(1, numvals);
    else
        assert(isnumeric(tickfactors) && isvector(tickfactors) && numel(tickfactors)==numvals, ...
               'TICKFACTORS must be a numeric vector of length numel(VALS)')
        tickfactors = tickfactors(:).';
    end

    opts_line = struct('colors', [.1 .1 .1]);
    if (nargin < 7)
        opts_data = opts_line;
    else
        assert(isstruct(opts_data) && numel(opts_data)==1, 'OPTS_DATA must be a scalar struct')
    end

    %% ----------
    global glc GL

    xyxy = reshape(xyxy, 2, 2);
    vals = vals(:).';

    p1 = xyxy(1:2).';  % col vec
    vec = xyxy(3:4).'-p1;
    rotvecn = ticklen.*[vec(2); -vec(1)] ./ hypot(vec(1), vec(2));

    ff = (vals - lims(1)) ./ (lims(2) - lims(1));
    tickposns = repmat(p1, 1, numvals) + repmat(ff, 2, 1).*repmat(vec, 1, numvals);
    tickposns(3:4, :) = tickposns + repmat(rotvecn, 1, numvals).*repmat(tickfactors, 2, 1);

    glcall(glc.draw, GL.LINES, reshape(tickposns, 2, []), opts_data);
    glcall(glc.draw, GL.LINES, xyxy, opts_line);

    if (textsize == 0)
        return
    end

    s = sign(textsize);
    textsize = abs(textsize);
    ofs = [abs(ticklen)*max(abs(tickfactors)) + 2, 0];
    ang = (180/pi)*atan2(s*rotvecn(2), s*rotvecn(1));

    if (ang == 180)
        % TODO: handle range (90, 270)?
        ang = 0;
        xalign = 1;
        ofs = -ofs;
    else
        xalign = -1;
    end

    for i=1:numvals
        str = sprintf('%.3f', vals(i));
        glc_rotatetext(tickposns(1:2, i), ang, ofs, textsize, str, [xalign 0]);
    end
end
