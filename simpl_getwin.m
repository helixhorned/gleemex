function winid = simpl_getwin()
    global GL glc
    winid = glcall(glc.get, GL.WINDOW_ID);
end
