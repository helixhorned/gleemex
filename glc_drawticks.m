% glc_drawticks(XYXY, LIMS, VALS [, TICKLEN [, TEXTSIZE [, TICKFACTORS [, OPTS_DATA]]]])

% Draws tick marks and optionally accompanying text at specified positions
% between two limits. This function expects the rendering to have been set up
% with, or equivalently to, glc_setup2d().
%
% XYXY: The [x1 y1, x2 y2] position of the "ruler" in screen
%  coordinates. Currently, it is expected that it is drawn parallel to an axes,
%  i.e. either x1==x2 or y1==y2.
%
% LIMS: The [start, ending] numeric limits presumed at (x1, y1) and
%  (x2, y2). LIMS(1) must be strictly less than LIMS(2).
%
% VALS: A non-empty vector of values between LIMS(1) and LIMS(2). Tick marks
%  will be drawn at positions corresponding to these values.
%
% TICKLEN: the base length of each tick mark in pixels. If non-negative, tick
%  marks are drawn to the right side of the vector (x1, y1)->(x2, y2).
%  Otherwise, they are drawn with length -TICKLEN at the opposite side.
%  Default: 1
%
% TEXTSIZE: the height of the label text to display along with each tick
%  mark. If zero, no text is drawn. Currently, the labels are generated using
%  the sprintf() format '%.3f'. If non-negative, the labels are shown at the
%  same side as the tick marks. Otherwise, they are shown at the opposite side.
%  Default: 0
%
% TICKFACTORS: factors for the length of the individual tick marks multiplied
%  with the base TICKLEN.
%  Default: ones(1, numel(VALS))
%
% OPTS_DATA: a struct permissible to the glcall(glc.draw, ...) call for the
%  actual tick marks, to e.g. change their colors. They are drawn as
%  numel(VALS) GL.LINES.
%  Default: struct('colors', [.1 .1 .1])
function glc_drawticks(xyxy, lims, vals, ticklen, textsize, tickfactors, opts_data)
    glc_assert(isnumeric(xyxy) && numel(xyxy)==4, 'XYXY must be a numeric 4-array')
    glc_assert(isnumeric(lims) && numel(lims)==2, 'LIMS must be a numeric pair')
    glc_assert(isnumeric(vals) && isvector(vals) && numel(vals) >= 2, ...
           'VALS must be a numeric vector of length at least 2')
    glc_assert(lims(1) < lims(2), 'LIMS(2) must be greater than LIMS(1)');

    glc_assert(isnumeric(vals) && isvector(vals), 'VALS must be a numeric vector')

    numvals = numel(vals);

    % TODO: add a way of specifying the offset of the text from the tick mark end
    if (nargin < 4)
        ticklen = 1;
    else
        glc_assert(isnumeric(ticklen) && numel(ticklen)==1, 'TICKLEN must be a numeric scalar')
    end

    if (nargin < 5)
        textsize = 0;
    else
        glc_assert(isnumeric(textsize) && numel(textsize)==1, 'TEXTSIZE must be a numeric scalar')
    end

    if (nargin < 6)
        tickfactors = ones(1, numvals);
    else
        glc_assert(isnumeric(tickfactors) && isvector(tickfactors) && numel(tickfactors)==numvals, ...
                   'TICKFACTORS must be a numeric vector of length numel(VALS)')
        tickfactors = tickfactors(:).';
    end

    opts_line = struct('colors', [.1 .1 .1]);
    if (nargin < 7)
        opts_data = opts_line;
    else
        glc_assert(isstruct(opts_data) && numel(opts_data)==1, 'OPTS_DATA must be a scalar struct')
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
