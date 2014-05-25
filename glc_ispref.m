% IS = glc_ispref(GROUP [, PREF])
function is = glc_ispref(group, varargin)
    glc_assert(nargin == 1 || nargin == 2, 'Usage: glc_ispref(GROUP [, PREF])')
    glc_checkpref(group, varargin{:});

    is = false;

    filename = glc_prefmatfile(group);

    if (~exist(filename, 'file'))
        return
    end

    if (nargin == 1)
        is = true;  % GROUP exists
        return
    end

    pref = varargin{1};

    ws = whos('-file', filename);
    for i=1:numel(ws)
        if (strcmp(ws(i).name, pref))
            is = true;  % GROUP:PREF exists
            return
        end
    end
end
