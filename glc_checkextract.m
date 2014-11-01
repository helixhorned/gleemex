% GLC_CHECKEXTRACT([FORCE])
function glc_checkextract(force)
    % XXX: CODEDUP addabspaths.m

    if (nargin < 1)
        force = false;
    else
        assert(islogical(force) && numel(force)==1, 'FORCE must be a logical scalar')
    end

    st = dbstack('-completenames');
    if (numel(st) < 2)
        error('This function must be called from another function')
    end

    % numel(st) == 2 check: because of this (from the Gleemex manual):
    %  "In Octave, it is possible to call secondary functions by name when the
    %   primary function is active, but only when the latter (...) has been
    %   invoked from the top level!"
    if (~force && exist('OCTAVE_VERSION','builtin') && numel(st) == 2)
        return
    end

    [callerdir, fn, ext] = fileparts(st(2).file);
    filename = st(2).file;

    fnames = extractsecfuncs(filename, 0, callerdir, 2);

    d = dir(filename);
    assert(numel(d) == 1);
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
