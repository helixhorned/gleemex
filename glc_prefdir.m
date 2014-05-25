% DIRNAME = glc_prefdir()
function dirname = glc_prefdir()
    if (exist('OCTAVE_VERSION', 'builtin'))
        homedir = getenv('HOME');
        assert(~isempty(homedir), '$HOME must be set to use glc_*pref functions')

        dirname = fullfile(homedir, '.octave');
    else
        dirname = prefdir();
    end
end
