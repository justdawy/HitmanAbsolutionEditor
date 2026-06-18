#include <filesystem>
#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#include "Editor.h"
#include "Utility/StringUtility.h"
int main()
{
    Editor& editor = Editor::GetInstance();
    if (!editor.Setup())
    {
        return 0;
    }
    editor.Start();
    return 0;
}