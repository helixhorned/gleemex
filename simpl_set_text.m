function simpl_set_text(str)
    global simpl
    if (~ischar(str) && ~isvector(str))
        error('STR must be a char vector');
    end
    simpl.moretext = str;
end
