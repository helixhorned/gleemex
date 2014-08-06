% READFILESTR(FILENAME, READAS)  Reads whole file contents and returns
% a string as row vector. Optional argument READAS specifies the type
% passed to fopen(), the default is '*char'.
% -1 (a scalar double) is returned if file couldn't be opened.
function str=readfilestr(filename, readas)
    str = -1;

    % BrainVision EEG VHDR files have some silly characters in them,
    % therefore open in ISO encoding...
    if (~exist('OCTAVE_VERSION', 'builtin'))
        [fid,msg] = fopen(filename, 'r', 'n', 'ISO-8859-1');
    else
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
end
