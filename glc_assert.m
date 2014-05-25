% glc_assert(COND [, ERRMSG])
% "Light" assertion check.
function glc_assert(cond, errmsg)
    if (~cond)
        if (nargin == 1)
            error('assertion failed')
        else
            error(errmsg)
        end
    end
end
