% simpl_setup_data(DATA, IDXS, COLORS [, MAYBE_SIMPL])
function simpl_setup_data(data, idxs, colors, maybe_simpl)
    if (nargin < 4)
        % Legacy
        global simpl
    else
        % classdef-enabled simpleplot
        simpl = maybe_simpl;
    end

    if (simpl.havelist)
        alldata = single(cat(2, data{:, 2}));

        simpl.numdims = 3;

        simpl.data = data;
        simpl.idxs = [];
        simpl.colors = [];
    else
        %% validation
        if (~isnumeric(data))
            error('DATA must be numeric');
        end

        numdims = size(data, 1);
        if (numdims ~= 2 && numdims ~= 3)
            error('DATA must have 2 or 3 ''dimensions'' (i.e. length of 1st dim must be 2 or 3)');
        end

        alldata = single(data);

        if (numdims == 2)
            alldata(3, :) = 0;  % pad 3rd dim with zeros
        end
        simpl.numdims = numdims;

        simpl.data = alldata;
        simpl.idxs = idxs;
        simpl.colors = colors;
    end

    simpl.numsamples = size(alldata, 2);

    % some data stats
    simpl.mean = mean(alldata, 2);
    simpl.min = min(alldata, [], 2);
    simpl.max = max(alldata, [], 2);
end
