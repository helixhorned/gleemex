% FILENAME = glc_prefmatfile(GROUP [, CHECKEXIST])
% INTERNAL.
function filename = glc_prefmatfile(group, checkExist)
    filename = fullfile(glc_prefdir(), sprintf('glc_pref_%s.mat', group));

    if (nargin >= 2 && checkExist)
        if (~exist(filename, 'file'))
            error('Preference group "%s" does not exist', group)
        end
    end
end
