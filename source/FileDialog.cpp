#include <windows.h>
#include <commdlg.h>

#include "Filedialog.h"
#include "EditorState.h"

#pragma comment(lib, "Comdlg32.lib")

std::string OpenLEDFile() {
    char filename[MAX_PATH] = {};

    OPENFILENAMEA dialog = {};

    dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = g_mainWindow;
    dialog.lpstrFile = filename;
    dialog.nMaxFile = MAX_PATH;
    dialog.lpstrFilter = "LED files (*.led)\0*.led\0" "All files (*.*)\0*.*\0";

    dialog.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

    dialog.lpstrTitle = "Open LED File";

    if (GetOpenFileNameA(&dialog))
        return filename;

    return "";
}

std::string OpenMapFile() {
    char filename[MAX_PATH] = {};

    OPENFILENAMEA dialog = {};

    dialog.lStructSize = sizeof(dialog);
    dialog.lpstrFile = filename;
    dialog.nMaxFile = MAX_PATH;
    dialog.lpstrFilter = "Map Files (*.lwd)\0*.lwd\0" "All Files (*.*)\0*.*\0";
    dialog.nFilterIndex = 1;

    dialog.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;

    if (GetOpenFileNameA(&dialog)) {
        return filename;
    }

    return "";
}

std::string SaveMapFile() {
    char filename[MAX_PATH] = {};

    OPENFILENAMEA dialog = {};

    dialog.lStructSize = sizeof(dialog);
    dialog.lpstrFile = filename;
    dialog.nMaxFile = MAX_PATH;
    dialog.lpstrFilter = "Map Files (*.lwd)\0*.lwd\0" "All Files (*.*)\0*.*\0";
    dialog.nFilterIndex = 1;
    dialog.lpstrDefExt = "lwd";
    dialog.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (GetSaveFileNameA(&dialog)) {
        return filename;
    }

    return "";
}