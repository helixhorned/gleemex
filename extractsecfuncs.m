% EXTRACTSECFUNCS(FILENAME): Extract secondary functions from an M-file
%  into separate M-files.
function [match,tokens]=extractsecfuncs(filename)

str = readfilestr(filename);

[match, tokens] = ...
    regexp(str, '^function +([A-Za-z][A-Za-z0-9_]*).*?^end$', ...
           'match','tokens', ...
           'dotall','lineanchors');

[tmpdir, tmpfn] = fileparts(filename);  % tmpfn strips extension

try
    badi = zeros(1,length(tokens), 'logical');
catch
    % MATLAB can't do the above
    badi = (zeros(1,length(tokens), 'uint8')~=0);
end

for i=1:length(tokens)
    if (strcmpi(tmpfn, tokens{i}))
        badi(i) = true;
    end
end

match(badi) = [];
tokens(badi) = [];

try
    tokens = cell2mat(tokens);  % {}{} -> {}
catch
    % MATLAB: Cannot support cell arrays containing cell arrays or objects.
    for i=1:length(tokens)
        tokens{i} = tokens{i}{1};
    end
end

if (isempty(tokens))
    disp('No functions extracted.');
    return;
end

fnames = tokens;

prompt = sprintf('Create the following files? (y/[n])\n');
for i=1:length(tokens)
    tokens{i} = sprintf('  %s.m\n', tokens{i});
    prompt = [prompt tokens{i}];
end

prompt = [prompt '> '];

try
    fflush(stdout);
catch
end

s = input(prompt, 's');
if (strcmp(s, 'y'))
    for i=1:length(fnames)
        [fid,msg] = fopen([fnames{i} '.m'], 'w+');
        if (fid < 0)
            warning(sprintf('%s: %s', fnames{i}, msg));
            continue;
        end
        fwrite(fid, sprintf('%s\n', match{i}), 'char');
        fclose(fid);
    end
end
