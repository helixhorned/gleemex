function have = simpl_have_classdef()
    persistent havep
    if (~isempty(havep))
        have = havep;
        return
    end

    try
        el = GLCEditableLine(false);
        clear el;
        have = true;
    catch
        have = false;
    end

    havep = have;
end
