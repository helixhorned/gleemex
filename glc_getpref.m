% VAL = glc_getpref(GROUP [, PREF [, DEFAULT]])
function val = glc_getpref(group, pref, default)
    glc_assert(nargin >= 1 && nargin <= 3, 'Usage: glc_getpref(GROUP [, PREF [, DEFAULT]])')
    glc_checkpref(group);

    filename = glc_prefmatfile(group, true);

    if (nargin == 1)
        val = load(filename);
        return
    end

    glc_checkpref(group, pref);

    ws = whos('-file', filename);
    names = { ws(:).name };

    if (~any(strcmp(names, pref)))
        if (nargin == 2)
            error('Preference group "%s" does not contain entry "%s"', group, pref)
        end

        val = default;
        return
    end

    s = load(filename, pref);
    val = s.(pref);
end
