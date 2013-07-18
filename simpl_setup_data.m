function simpl_setup_data(data, firsttime, idxs)
    global simpl

    if (nargin < 2)
        firsttime = false;
    end

    %% validation
    if (~isnumeric(data))
        error('DATA must be numeric');
    end

    numdims = size(data, 1);
    if (numdims ~= 2 && numdims ~= 3)
        error('DATA must have 2 or 3 ''dimensions'' (i.e. length of 1st dim must be 2 or 3)');
    end

    if (any(idxs==0))
        error('IDXS are one-based and thus must not contain zeros')
    end

    if (~firsttime)
        onumsamples = simpl.numsamples;
    end

    simpl.numsamples = size(data, 2);
    simpl.data = single(data);

    if (~firsttime)
        if (simpl.numsamples > simpl.lineidxs(2))
            simpl.lineidxs(2) = simpl.numsamples;
        elseif (onumsamples == simpl.lineidxs(2))
            simpl.lineidxs(2) = simpl.numsamples;
        end
    end

    if (numdims == 2)
        simpl.data(3, :) = 0;  % pad 3rd dim with zeros
    end
    simpl.numdims = numdims;

    % some data stats
    simpl.mean = mean(simpl.data, 2);
    simpl.min = min(simpl.data, [], 2);
    simpl.max = max(simpl.data, [], 2);
end
