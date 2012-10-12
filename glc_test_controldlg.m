function vals=glc_test_controldlg()
    cvals = struct('numval', pi, ...
                   'strval', 'Hello, world!', ...
                   'boolval', true);
    cdesc = struct('key', {'numval','strval','boolval'}, ...
                   'name', {'A great constant', 'A greeting', 'To be or not to be'}, ...
                   'extra',[]);
    % numeric values need some extra info
    cdesc(1).extra = struct('format','%.03f', ...
                            'updownfunc',@(ov,dir)min(max(0, ov+dir/10), 4));

    [vals,ok]=glc_listdlg('ControlVals',cvals, 'ControlDesc',cdesc, 'ListSize',[320 200]);
end
