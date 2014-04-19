% GLC_CHECKEXTRACT([FORCE])
function glc_checkextract(force)
    % XXX: CODEDUP addabspaths.m

    if (nargin < 1)
        force = false;
    else
        assert(islogical(force) && numel(force)==1, 'FORCE must be a logical scalar')
    end

    if (~force && exist('OCTAVE_VERSION','builtin'))
        return
    end

    st = dbstack('-completenames');
    if (numel(st) < 2)
        error('This function must be called from another function')
    end

    [callerdir, fn, ext] = fileparts(st(2).file);
    filename = [fn ext];

    fnames = extractsecfuncs(filename, 0, callerdir, 2);

    d = dir(filename);
    modtime = d.datenum;

    needupdate = false;

    for i=1:numel(fnames)
        fn = fullfile(callerdir, [fnames{i} '.m']);

        d = dir(fn);
        if (isempty(d) || modtime > d.datenum)
            needupdate = true;
            break
        end
    end

    if (needupdate)
        extractsecfuncs(filename, 0, callerdir, true);
    end
end
