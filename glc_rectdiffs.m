% DIFFS = GLC_RECTDIFFS(RECT)
%
% For an array RECT indexable up to index 4, returns
% [RECT(3)-RECT(1), RECT(4)-RECT(2)].
function diffs=glc_rectdiffs(rect)
    diffs = [rect(3)-rect(1), rect(4)-rect(2)];
end
