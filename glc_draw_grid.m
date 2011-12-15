% GLC_DRAW_GRID(GRIDEXTENT_RECT, SUBDIV_XY, ...)
%
%  Draws a 2D grid of (xsubdivs x ysubdivs) lines in the z=0 plane
%
%  GRIDEXTENT_RECT: the grid x,y extents, a matrix that is linearly
%   indexable up to index 4:  (1): x1, (2): y1; (3): x2, (4): y2
%
%  SUBDIV_XY: a 2-vector [xsubdivs ysubdivs] specifying the number of
%   'fenceposts' in each direction, or a scalar specifying them both
%
%  variable args are passed as trailing arguments to glcall(glc.draw, ...)
function glc_draw_grid(gridextent_rect, subdiv_xy, varargin)
    global glc GL

    if (numel(subdiv_xy)==1)
        subdiv_xy(2) = subdiv_xy;
    end

    xx = linspace(gridextent_rect(1), gridextent_rect(3), subdiv_xy(1));
    yy = linspace(gridextent_rect(2), gridextent_rect(4), subdiv_xy(2));

    % y=const lines
    vlines = zeros(2, 2*subdiv_xy(2));
    vlines(1, :) = repmat(gridextent_rect([1 3]), 1,subdiv_xy(2));
    vlines(2, 1:2:end-1) = yy;
    vlines(2, 2:2:end) = yy;

    % x=const lines
    hlines = zeros(2, 2*subdiv_xy(1));
    hlines(2, :) = repmat(gridextent_rect([2 4]), 1,subdiv_xy(1));
    hlines(1, 1:2:end-1) = xx;
    hlines(1, 2:2:end) = xx;

    glcall(glc.draw, GL.LINES, [vlines, hlines], varargin{:});
end
