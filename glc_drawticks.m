% glc_drawticks(XYXY, LIMS, VALS [, TICKLEN [, TICKFACTORS]])
function glc_drawticks(xyxy, lims, vals, ticklen, tickfactors)
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
        tickfactors = ones(1, numvals);
    else
        assert(isnumeric(tickfactors) && isvector(tickfactors) && numel(tickfactors)==numvals, ...
               'TICKFACTORS must be a numeric vector of length numel(VALS)')
        tickfactors = tickfactors(:).';
    end

    %% ----------
    global glc GL

    xyxy = reshape(xyxy, 2, 2);
    vals = vals(:).';

    opts = struct('colors', [.1 .1 .1]);

    glcall(glc.draw, GL.LINES, xyxy, opts);

    p1 = xyxy(1:2).';  % col vec
    vec = xyxy(3:4).'-p1;
    rotvecn = ticklen.*[vec(2); -vec(1)] ./ hypot(vec(1), vec(2));

    ff = (vals - lims(1)) ./ (lims(2) - lims(1));
    tickposns = repmat(p1, 1, numvals) + repmat(ff, 2, 1).*repmat(vec, 1, numvals);
    tickposns(3:4, :) = tickposns + repmat(rotvecn, 1, numvals).*repmat(tickfactors, 2, 1);

    glcall(glc.draw, GL.LINES, reshape(tickposns, 2, []), opts);
end
