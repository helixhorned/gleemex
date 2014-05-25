% glc_setpref(GROUP, PREF, VAL)
function glc_setpref(group, pref, val)
    glc_assert(nargin == 3, 'Usage: glc_setpref(GROUP, PREF, VAL)')
    glc_checkpref(group, pref);

    s = struct();
    s.(pref) = val;

    filename = glc_prefmatfile(group);
    save(filename, '-v7', '-append', '-struct', 's');
end
