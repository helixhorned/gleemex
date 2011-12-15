% READFILESTR(FILENAME, READAS)  Reads whole file contents and returns
% a string as row vector. Optional argument READAS specifies the type
% passed to fopen(), the default is '*char'.
% NaN is returned if file couldn't be opened.
function str=readfilestr(filename, readas)
    str = NaN;

    % BrainVision EEG VHDR files have some silly characters in them,
    % therefore open in ISO encoding...
    try
        [fid,msg] = fopen(filename, 'r', 'n', 'ISO-8859-1');
    catch
        [fid,msg] = fopen(filename, 'r', 'n');
    end

    if (fid<0)
        warning(msg);
        return
    end

    if (nargin<2)
        readas = '*char';
    end
    % ...and read it into a string
    str = fread(fid, inf, readas)';
    fclose(fid);
