function simpl_set_text(str)
    if (simpl_have_classdef())
        simpl = simpl_getobj();
    else
        global simpl
    end

    if (~ischar(str) && ~isvector(str))
        error('STR must be a char vector');
    end
    simpl.moretext = str;
end
