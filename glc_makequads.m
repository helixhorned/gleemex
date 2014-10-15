% QUADS = glc_makequads(RECTS)
%
% RECTS: a numeric 4-by-NUMRECTS matrix of (x1, y1, x2, y2) entries
%
% QUADS: a 2-by-(4*NUMRECTS) matrix with successive entries
%  (x1, y1)
%  (x2, y1)
%  (x2, y2)
%  (x1, y2)
function quads = glc_makequads(rects)
    glc_assert(isnumeric(rects) && ndims(rects)==2, 'RECTS must be a numeric matrix')
    glc_assert(size(rects, 1) == 4, 'RECTS must have 4 rows')

    numrects = size(rects, 2);

    % We have: 1st points (1:2), and 3rd points but at the positions of the 2nd (3:4).

    rects = rects.';
    % Fourth points. x1 and y2.
    rects(:, 7:8) = rects(:, [1 4]);

    % Third points. x2 and y2.
    rects(:, 5:6) = rects(:, 3:4);

    % Tweak y of second points from y2 to y1.
    rects(:, 4) = rects(:, 2);
    rects = rects.';

%{
    % Fourth points. x1 and y2.
    rects(7:8, :) = rects([1 4], :);

    % Third points. x2 and y2.
    rects(5:6, :) = rects(3:4, :);

    % Tweak y of second points from y2 to y1.
    rects(4, :) = rects(2, :);
%}
    % Reshape to the result size.
    quads = reshape(rects, 2, 4*numrects);
end
