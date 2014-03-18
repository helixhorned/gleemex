% OBJ = glc_appdata()
% Retrieves application data for the current window, previously registered with
% GLCApplicationData.register().
function obj = glc_appdata()
    global glc_appdata

    winid = glcall(int32(17), int32(-100));  % glc.get, GL.WINDOW_ID
    if (winid > numel(glc_appdata))
        error('No Gleemex application data registered for window ID %d', winid)
    end

    obj = glc_appdata{winid};
end
