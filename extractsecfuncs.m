% FNAMES = EXTRACTSECFUNCS(FILENAME [, NUMSPACE [, THEDIR [, ASKP]]]) Extract
% secondary functions from an M-file into separate M-files.
%
% NUMSPACE: TODO. default 0
% THEDIR: the directory in which to extract the files
% ASKP - if true (the default), ask for confirmation when creating files.
%  if ==2, then no files are created, but FNAMES is returned.
%
% FNAMES: a cell array of length #secfuncs, each element containing the
%  secondary function's name. If ASKP is true, only if user confirmed, else {}.
function fnames = extractsecfuncs(filename, numspace, thedir, askp)
    if (nargin < 1)
        error('Must pass FILENAME as first argument')
    elseif (~ischar(filename) || ~isvector(filename))
        error('FILENAME must be a string')
    end

    if (~exist('numspace', 'var'))
        numspace = 0;
    else
        if (ischar(numspace))
            numspace = str2double(numspace);
        end
        assert(isnumeric(numspace) && numel(numspace)==1, 'NUMSPACE must be a numeric scalar')
    end

    if (numspace <= 0)
        SP = '';
    else
        SP = repmat(' ', 1,numspace);
    end

    if (~exist('thedir', 'var'))
        thedir = '.';
    else
        assert(ischar(thedir) && isvector(thedir), 'THEDIR must be a nonempty string')
    end

    if (~exist('askp', 'var'))
        askp = true;
    else
        if (~(isscalar(askp) && (islogical(askp) || isequal(askp, 2))))
            error('ASKP must be one of false, true, or 2')
        end
    end

    %%
    str = readfilestr(filename);
    if (isequal(str, -1))
        fprintf('coudn''t read ''%s''\n', filename);
        fnames = {};
        return;
    end

    RE = ['^' SP 'function +(?:[^=' char(10) ']*?= *)?([A-Za-z][A-Za-z0-9_]*).*?^' SP 'end' char(13) '?$'];

    [match, tokens] = ...
        regexp(str, RE, ...
               'match','tokens', ...
               'dotall','lineanchors');

    [tmpdir, tmpfn] = fileparts(filename);  % tmpfn strips extension

    % We have only one entry per tokens{i}, the function name.
    for i=1:length(tokens)
        tokens{i} = tokens{i}{1};
    end

    badi = strcmpi(tmpfn, tokens);
    match(badi) = [];
    tokens(badi) = [];

    % The names of the secondary functions.
    fnames = tokens;

    if (isequal(askp, 2))
        return
    end

    if (isempty(tokens))
        disp('No functions extracted.');
        return;
    end

    if (askp)
        prompt = sprintf('Create the following files? (y/[n])\n');
        for i=1:length(tokens)
            tokens{i} = sprintf('  %s.m\n', fullfile(thedir, tokens{i}));
            prompt = [prompt tokens{i}];
        end

        prompt = [prompt '> '];

        try
            fflush(stdout);
        catch
        end

        prompt = strrep(prompt, '\', '\\');

        s = input(prompt, 's');
    else
        s = 'y';
    end

    if (strcmp(s, 'y'))
        for i=1:length(fnames)
            [fid,msg] = fopen(fullfile(thedir, [fnames{i} '.m']), 'w+');
            if (fid < 0)
                warning(sprintf('%s: %s', fnames{i}, msg));
                continue;
            end
            fwrite(fid, sprintf('%s\n', match{i}), 'char');
            fclose(fid);
        end
    else
        fnames = {};
    end

    if (~askp)
        fprintf('Extracted %d secondary functions from %s.\n', ...
                length(fnames), filename);
    end
end
