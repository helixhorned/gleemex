% glc_setpref(GROUP, PREF, VAL)
function glc_setpref(group, pref, val)
    glc_assert(nargin == 3, 'Usage: glc_setpref(GROUP, PREF, VAL)')
    glc_checkpref(group, pref);

    s = struct();
    s.(pref) = val;

    filename = glc_prefmatfile(group);
    if (~exist(filename, 'file'))
        % MATLAB errors on -append when file doesn't exist.
        save(filename, '-v7', '-struct', 's');
    else
        save(filename, '-v7', '-append', '-struct', 's');
    end
end
