% EXTRACTSECFUNCS(FILENAME): Extract secondary functions from an M-file
%  into separate M-files.
function [match,tokens]=extractsecfuncs(filename)

str = readfilestr(filename);

[match, tokens] = ...
    regexp(str, '^function +([A-Za-z][A-Za-z0-9_]*).*?end$', ...
           'match','tokens', ...
           'dotall','lineanchors');

[tmpdir, tmpfn] = fileparts(filename);  % tmpfn strips extension

badi = zeros(1,length(tokens), 'logical');

for i=1:length(tokens)
    if (strcmpi(tmpfn, tokens{i}))
        badi(i) = true;
    end
end

match(badi) = [];
tokens(badi) = [];
tokens = cell2mat(tokens);  % {}{} -> {}

if (isempty(tokens))
    disp('No functions extracted.');
    return;
end

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
    for i=1:length(tokens)
        fid = fopen(tokens{i}, 'w+');
        fwrite(fid, sprintf('%s\n', match{i}), 'char');
        fclose(fid);
    end
end
