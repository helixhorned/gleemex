% rectborders: (2, 2)  (TODO: 2, 2*n)
function rectverts=glc_expandrect(rectborders)

    if (size(rectborders,1)==1)
        rectborders = reshape(rectborders, 2,[]);
    end

    rectverts = rectborders;

    rectverts(:, [3 4]) = rectverts(:, [2 1]);  % p1 p2 p2 p1

    rectverts(2, [2 4]) = rectverts(2, [1 3]);

%    rectverts(2, 2) = rectverts(2, 1);  % p1 (x2 y1) p2 p1
%    rectverts(2, 4) = rectverts(2, 3);  % p1 (x2 y1) p2 (x1 y2)
end
