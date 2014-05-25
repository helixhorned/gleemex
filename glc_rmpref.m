% glc_rmpref(GROUP [, PREF])
function glc_rmpref(group, varargin)
    glc_assert(nargin == 1 || nargin == 2, 'Usage: glc_rmpref(GROUP [, PREF])')
    glc_checkpref(group, varargin{:});

    filename = glc_prefmatfile(group, true);

    if (nargin == 1)
        delete(filename);
        return
    end

    s = load(filename);
    pref = varargin{1};

    if (~isfield(s, pref))
        error('Preference group "%s" does not contain entry "%s"', group, pref)
    end

    s = rmfield(s, pref);

    save(filename, '-v7', '-struct', 's');
end
