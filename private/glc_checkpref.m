function glc_checkpref(group, pref)
    glc_assert(isvarname(group), 'GROUP must be a valid variable name')
    if (nargin >= 2)
        glc_assert(isvarname(pref), 'PREF must be a valid variable name')
    end
end
