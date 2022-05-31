#include "FileOpenUtils.h"

#include <windows.h>
#include "../../DebugOut.h"


std::string ShowFileWindow()
{
    WCHAR filename[MAX_PATH];

    OPENFILENAME ofn;
    ZeroMemory(&filename, sizeof(filename));
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;  // If you have a window to center over, put its HANDLE here
    ofn.lpstrFilter = L"Text Files\0*.txt\0Any File\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = L"File selected";
    ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn))
    {
        dout::printf("[FileOpenUtils] You chose the file %s.\n", filename);
    }
    else
    {
        // All this stuff below is to tell you exactly how you messed up above. 
        // Once you've got that fixed, you can often (not always!) reduce it to a 'user cancelled' assumption.
        // CommDlgExtendedError();
    }

    std::wstring ret(filename);

    return std::string(ret.begin(), ret.end());
}