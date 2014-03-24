% GLC_CHECKEXTRACT()
function glc_checkextract()
    % XXX: CODEDUP addabspaths.m

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
