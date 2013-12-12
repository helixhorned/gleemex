function have = simpl_have_classdef()
    % TODO: test on MATLAB
    persistent have
    if (~isempty(have))
        return
    end

    try
        el = GLCEditableLine(false);
        clear el;
        have = true;
    catch
        have = false;
    end
end
