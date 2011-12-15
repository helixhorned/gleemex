function glc_test_subwindows()

    global glc GL

    glc = glcall();
    GL = getglconsts();

    glcall(glc.newwindow, [20 20], [800 600], 'top-level');

    glc_listdlg('ListString',{'qwe','asd','zxc'}, 'subwindow',true, 'listpos',[20 20]);
    glc_listdlg('ListString',{'123','435', 'PI'}, 'subwindow',true, 'listpos',[260 20]);
    glc_listdlg('ListString',{'Ork','Hum', 'Zrg'}, 'subwindow',true, 'listpos',[500 20]);

    glcall(glc.entermainloop);
end
