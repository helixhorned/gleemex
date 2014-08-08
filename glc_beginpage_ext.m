% [STATUS, FILENAME] = glc_beginpage_ext(FORMAT, SORTMODE, OPTIONS, BUFSIZE, FILEPREFIX)
%
% Calls glcall(glc.beginpage, ...) with the provided arguments, expect that
% FILEPREFIX automatically gets an extention appended based on the value of
% FORMAT and the GL.PS_COMPRESS bit of OPTIONS.
%
% STATUS: one of the glcall(glc.beginpage, ...) return values (0, 1 or 2), or
%  -1 if Gleemex was compiled without GL2PS support.
% FILENAME: In case of success (STATUS == 0), FILEPREFIX with an appended
%  extension.
function [status, filename] = glc_beginpage_ext(format, sortmode, options, bufsize, filename)
    global GL glc

    assert(ischar(filename) && isvector(filename), 'FILENAME must be a char vector')
    filename = filename(:)';

    assert(strcmp(class(options), 'int32') && numel(options)==1, ...
           'OPTIONS must be a scalar int32')

    if (~isfield(glc, 'beginpage'))
        status = -1;
        return
    end

    EXT = { 'ps', 'eps', 'tex', 'pdf', 'svg', 'pgf' };
    filename = [filename '.' EXT{format+1}];

    if (bitand(options, GL.PS_COMPRESS))
        if (format == GL.PS_SVG)
            filename = [filename 'z'];
        else
            filename = [filename '.gz'];
        end
    end

    status = glcall(glc.beginpage, format, sortmode, options, bufsize, filename);
end
