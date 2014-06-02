% TICKS = glc_genticks(LIMS [, MAXTICKS])
% Generates tick values between two limit endpoints spaced such that their
% decimal representations look "round".
%
% LIMS: the [start, ending] vector of the two value endpoints to generate the
%  ticks between. LIMS(1) must be strictly less than LIMS(2).
%
% MAXTICKS: the maximum number of tick values to generate. At least two are
%  always returned, though.
%  Default: 10
%
% Examples:
% ---------
%
% > glc_genticks([-3.627, 17.167], 10)
% ans =
%   -2    0    2    4    6    8   10   12   14   16
%
% > glc_genticks([4.459655e2 4.460666e2])
% ans =
%   446.00   446.01   446.02   446.03   446.04   446.05   446.06
function ticks = glc_genticks(lims, maxticks)
    glc_assert(isnumeric(lims) && numel(lims)==2, 'LIMS must be a numeric pair')
    glc_assert(all(isfinite(lims)), 'LIMS(1) and LIMS(2) must be finite')

    glc_assert(lims(1) < lims(2), 'LIMS(1) must be strictly less than LIMS(2)')

    if (nargin < 2)
        maxticks = 10;
    else
        glc_assert(isnumeric(maxticks) && numel(maxticks)==1, ...
                   'MAXTICKS must be a numeric scalar')
    end

    % Back up original limits. Calling lo := oilms(1) and hi := olims(2), ...
    olims = lims;

    if (lims(2) < 0)  % both lo and hi are negative
        lims = -lims([2 1]);
        preproc = -1;
    elseif (lims(1) < 0)  % lo < 0 and hi >= 0
        abslo = abs(lims(1));

        if (abslo >= lims(2))
            lims = [0 abslo];  % working limits are [0 abs(lo)]
            preproc = 1;
        else
            lims = [0 lims(2)];  % working limits are [0 hi]
            preproc = 2;
        end
    else
        preproc = 0;
    end

    % Print the number in the format
    % <leading digit>.<15 decimal digits>e+<2 or 3 exponent digits>
    % (Thus, 21 or 22 characters in total.)
    strlo = sprintf('%.15e', lims(1));
    strhi = sprintf('%.15e', lims(2));

    explo = get_numstr_exp(strlo);
    exphi = get_numstr_exp(strhi);
    dexp = exphi - explo;

    if (dexp > 0)
        % Move the decimal point dexp places to the left.
        exphistr = sprintf('e%+03d', exphi);

        if (dexp >= 16)
            strlo = ['0.000000000000000' exphistr];
        else
            strlo = ['0.' repmat('0', 1,dexp-1) strlo([1 3:17-dexp]) exphistr];
        end
    end

    glc_assert(numel(strlo) == numel(strhi));

    % Trailing [2] at end is to append a same char ('.') to both.
    diglo = strlo([1, 3:17, 2]);
    dighi = strhi([1, 3:17, 2]);

    % Find the lowest position where either
    % the digits differ, and the next digits are not 9 and 0, respectively; or
    i1 = find(diglo(1:end-1) ~= dighi(1:end-1) & ...
              dighi(2:end) - diglo(2:end) ~= -9, 1);
    % the digits differ by more than two
    i2 = find(dighi - diglo >= 2, 1);

    di = min([i1 i2]);

    % Example:
    %  4.459955
    %  4.460066
    %       ^
    % di = 5

    if (isempty(di) || di == 16)
        % isempty can happen e.g. when olims is [1+eps, 1+2*eps].
        % di == 16 means that the difference is only in the last digit.
        ticks = olims;
        return
    end

    if (any(diglo(di+2:end-1) ~= '0'))
        % "Add 1 to the digit at position di+1"
        diglo(di+2:end-1) = '9';
    end

    strlo([1, 3:17, 2]) = diglo;
    strhi([1, 3:17, 2]) = dighi;

    step = 10^(exphi-di);

    ticks = str2double(strlo):step:str2double(strhi);

    if (preproc <= 0)
        backticks = 0;  % dummy
    elseif (preproc == 1)
        backticks = 0:step:olims(2);
    elseif (preproc == 2)
        backticks = 0:step:abslo;
    end

    nticks = numel(ticks);
    nbacks = numel(backticks);

    for istep = [1 2 5 10 20 50 100]
        if (numel(1:istep:nticks) + numel(istep+1:istep:nbacks) <= maxticks)
            break
        end
    end

    ticks = ticks(1:istep:nticks);

    neednegate = (preproc == -1 || preproc == 1);
    if (neednegate && ticks(1) == 0)
        % So that in the output ticks, it's positive zero again
        ticks(1) = -0;
    end

    if (preproc > 0)
        backticks = -backticks(istep+1:istep:nbacks);
        ticks = [backticks(end:-1:1) ticks];
    end

    if (neednegate)
        ticks = -ticks(end:-1:1);
    end
end

function exp = get_numstr_exp(str)
    if (str(end-3) == 'e')
        exp = str2double(str(end-2:end));
    else
        glc_assert(str(end-4) == 'e');
        exp = str2double(str(end-3:end));
    end
end
