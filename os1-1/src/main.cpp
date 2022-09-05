#ifndef UNICODE
#define UNICODE
#endif 

#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <regex>
#include <queue>

#pragma comment(lib, "ComCtl32.lib")

#define ID_PATH_BUTTON1 1
#define ID_PATH_BUTTON2 2
#define ID_START_BUTTON1 3
#define ID_START_BUTTON2 4
#define ID_PRIORITY_COMBOBOX1 5
#define ID_PRIORITY_COMBOBOX2 6
#define ID_MODE_COMBOBOX1 7
#define ID_MODE_COMBOBOX2 8
#define ID_MAX_DEPTH_EDIT1 9
#define ID_MAX_DEPTH_EDIT2 10
#define ID_REGEX_EDIT1 11
#define ID_REGEX_EDIT2 12
#define ID_RECURSIVE_BUTTON1 13
#define ID_RECURSIVE_BUTTON2 14
#define ID_RESULT_LISTVIEW1 15
#define ID_RESULT_LISTVIEW2 16

enum Mode {
    MODE_FILE_ONLY,
    MODE_DIR_ONLY,
    MODE_DIR_AND_FILE
};

struct HMenuGroup {
    HMENU hPathButton;
    HMENU hStartButton;
    HMENU hPriorityField;
    HMENU hMaxDepthField;
    HMENU hModeSelect;
    HMENU hRecursiveButton;
    HMENU hRegexField;
    HMENU hResultList;
};
struct ThreadConfig {
    TCHAR *szPath;
    std::wregex *regex;
    bool recursive;
    int maxDepth;
    int priority;
    Mode mode;
    std::vector<LPWSTR> results;
} threadConfig1, threadConfig2;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int addUISplit(HWND hwnd, HINSTANCE hInstance, HMenuGroup group, int x, int y);
void messageConfig(HWND hwnd, ThreadConfig *config);
DWORD WINAPI procThread(LPVOID lpParam);
void startThread(HWND hwnd, ThreadConfig *config, int startButtonId, int maxDepthEditId, int resultListViewId, int regexEditId);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    // Register the window class.
    const wchar_t CLASS_NAME[]  = L"Sample Window Class";
    
    WNDCLASS wc = { };

    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    // Create the window.

    HWND m_hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        L"File Search",                 // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
        );

    // Create a button

    if (m_hwnd == NULL)
    {
        return 0;
    }

    // Init configs

    threadConfig1.szPath = new TCHAR[MAX_PATH];
    ua_tcscpy_s(threadConfig1.szPath, MAX_PATH, L"C:\\");
    threadConfig1.regex = new std::wregex(L".*");
    threadConfig1.recursive = false;
    threadConfig1.maxDepth = 0;
    threadConfig1.priority = THREAD_PRIORITY_NORMAL;
    threadConfig1.mode = MODE_DIR_AND_FILE;
    threadConfig1.results = std::vector<LPWSTR>();

    threadConfig2.szPath = new TCHAR[MAX_PATH];
    ua_tcscpy_s(threadConfig2.szPath, MAX_PATH, L"C:\\");
    threadConfig2.regex = new std::wregex(L".*");
    threadConfig2.recursive = false;
    threadConfig2.maxDepth = 0;
    threadConfig2.priority = THREAD_PRIORITY_NORMAL;
    threadConfig2.mode = MODE_DIR_AND_FILE;
    threadConfig2.results = std::vector<LPWSTR>();

    ShowWindow(m_hwnd, nCmdShow);
    UpdateWindow(m_hwnd);

    if (addUISplit(m_hwnd, hInstance, { 
        (HMENU) ID_PATH_BUTTON1,
        (HMENU) ID_START_BUTTON1,
        (HMENU) ID_PRIORITY_COMBOBOX1,
        (HMENU) ID_MAX_DEPTH_EDIT1,
        (HMENU) ID_MODE_COMBOBOX1,
        (HMENU) ID_RECURSIVE_BUTTON1,
        (HMENU) ID_REGEX_EDIT1,
        (HMENU) ID_RESULT_LISTVIEW1
    }, 10, 10)) {
        return 1;
    }

    if (addUISplit(m_hwnd, hInstance, { 
        (HMENU) ID_PATH_BUTTON2,
        (HMENU) ID_START_BUTTON2,
        (HMENU) ID_PRIORITY_COMBOBOX2,
        (HMENU) ID_MAX_DEPTH_EDIT2,
        (HMENU) ID_MODE_COMBOBOX2,
        (HMENU) ID_RECURSIVE_BUTTON2,
        (HMENU) ID_REGEX_EDIT2,
        (HMENU) ID_RESULT_LISTVIEW2
    }, 300, 10)) {
        return 1;
    }

    // Run the message loop.

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // All painting occurs here, between BeginPaint and EndPaint.

            FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));

            EndPaint(hwnd, &ps);
        }
        return 0;


    case WM_COMMAND:
        {
            switch (wParam)
            {
                case ID_PATH_BUTTON1:
                    {
                        TCHAR path[MAX_PATH];
                        BROWSEINFO bi = { 0 };
                        bi.lpszTitle  = L"Browse for folder...";
                        bi.ulFlags    = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
                        LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
                        if ( pidl != 0 )
                        {
                            //get the name of the folder and put it in path
                            SHGetPathFromIDList ( pidl, path );

                            //free memory used
                            IMalloc * imalloc = 0;
                            if ( SUCCEEDED( SHGetMalloc ( &imalloc )) )
                            {
                                imalloc->Free ( pidl );
                                imalloc->Release ( );
                            }

                            if (threadConfig1.szPath != NULL) {
                                free(threadConfig1.szPath);
                            }
                            threadConfig1.szPath = new TCHAR[MAX_PATH];
                            ua_tcscpy_s(threadConfig1.szPath, MAX_PATH, path);
                        }
                    }
                    break;
                case ID_PATH_BUTTON2:
                    {
                        TCHAR path[MAX_PATH];
                        BROWSEINFO bi = { 0 };
                        bi.lpszTitle  = L"Browse for folder...";
                        bi.ulFlags    = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
                        LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
                        if ( pidl != 0 )
                        {
                            //get the name of the folder and put it in path
                            SHGetPathFromIDList ( pidl, path );

                            //free memory used
                            IMalloc * imalloc = 0;
                            if ( SUCCEEDED( SHGetMalloc ( &imalloc )) )
                            {
                                imalloc->Free ( pidl );
                                imalloc->Release ( );
                            }

                            if (threadConfig2.szPath != NULL) {
                                free(threadConfig2.szPath);
                            }
                            threadConfig2.szPath = new TCHAR[MAX_PATH];
                            ua_tcscpy_s(threadConfig2.szPath, MAX_PATH, path);
                        }
                    }
                    break;

                case ID_RECURSIVE_BUTTON1:
                    {
                        threadConfig1.recursive = !threadConfig1.recursive;
                        CheckDlgButton(hwnd, ID_RECURSIVE_BUTTON1, threadConfig1.recursive ? BST_CHECKED : BST_UNCHECKED);
                    }
                    break;

                case ID_RECURSIVE_BUTTON2:
                    {
                        threadConfig2.recursive = !threadConfig2.recursive;
                        CheckDlgButton(hwnd, ID_RECURSIVE_BUTTON2, threadConfig2.recursive ? BST_CHECKED : BST_UNCHECKED);
                    }
                    break;

                case ID_MODE_COMBOBOX1:
                    {
                        MessageBox(hwnd, L"Mode 1", L"Mode 1", MB_OK);
                    }
                    break;

                case ID_START_BUTTON1:
                    {
                        startThread(
                            hwnd,
                            &threadConfig1,
                            ID_START_BUTTON1,
                            ID_MAX_DEPTH_EDIT1,
                            ID_RESULT_LISTVIEW1,
                            ID_REGEX_EDIT1
                        );
                    }
                    break;

                case ID_START_BUTTON2:
                    {
                        startThread(
                            hwnd,
                            &threadConfig2,
                            ID_START_BUTTON2,
                            ID_MAX_DEPTH_EDIT2,
                            ID_RESULT_LISTVIEW2,
                            ID_REGEX_EDIT2
                        );
                    }
                    break;
            }

            if(HIWORD(wParam) == CBN_SELCHANGE)
            { 
                switch (LOWORD(wParam))
                {
                    case ID_MODE_COMBOBOX1:
                        {
                            threadConfig1.mode = (Mode) SendMessage(GetDlgItem(hwnd, ID_MODE_COMBOBOX1), CB_GETCURSEL, 0, 0);
                        }
                        break;

                    case ID_MODE_COMBOBOX2:
                        {
                            threadConfig2.mode = (Mode) SendMessage(GetDlgItem(hwnd, ID_MODE_COMBOBOX2), CB_GETCURSEL, 0, 0);
                        }
                        break;
                    
                    case ID_PRIORITY_COMBOBOX1:
                        {
                            switch (SendMessage(GetDlgItem(hwnd, ID_PRIORITY_COMBOBOX1), CB_GETCURSEL, 0, 0)) {
                                case 0:
                                    threadConfig1.priority = THREAD_PRIORITY_IDLE;
                                    break;
                                case 1:
                                    threadConfig1.priority = THREAD_PRIORITY_BELOW_NORMAL;
                                    break;
                                case 2:
                                    threadConfig1.priority = THREAD_PRIORITY_NORMAL;
                                    break;
                                case 3:
                                    threadConfig1.priority = THREAD_PRIORITY_ABOVE_NORMAL;
                                    break;
                                case 4:
                                    threadConfig1.priority = THREAD_PRIORITY_HIGHEST;
                                    break;
                                case 5:
                                    threadConfig1.priority = THREAD_PRIORITY_TIME_CRITICAL;
                                    break;
                            }
                        }
                        break;

                    case ID_PRIORITY_COMBOBOX2:
                        {
                            switch (SendMessage(GetDlgItem(hwnd, ID_PRIORITY_COMBOBOX2), CB_GETCURSEL, 0, 0)) {
                                case 0:
                                    threadConfig2.priority = THREAD_PRIORITY_IDLE;
                                    break;
                                case 1:
                                    threadConfig2.priority = THREAD_PRIORITY_BELOW_NORMAL;
                                    break;
                                case 2:
                                    threadConfig2.priority = THREAD_PRIORITY_NORMAL;
                                    break;
                                case 3:
                                    threadConfig2.priority = THREAD_PRIORITY_ABOVE_NORMAL;
                                    break;
                                case 4:
                                    threadConfig2.priority = THREAD_PRIORITY_HIGHEST;
                                    break;
                                case 5:
                                    threadConfig2.priority = THREAD_PRIORITY_TIME_CRITICAL;
                                    break;
                            }
                        }
                }
            }
        }
        return FALSE;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int addUISplit(HWND hwnd, HINSTANCE hInstance, HMenuGroup group, int x, int y)
{
    HWND hwndPathButton = CreateWindow( 
        L"BUTTON",  // Predefined class; Unicode assumed 
        L"Pick path",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
        x,         // x position 
        y,         // y position 
        100,        // Button width
        25,        // Button height
        hwnd,     // Parent window
        group.hPathButton,
        hInstance, 
        NULL);      // Pointer not needed.

    HWND hwndRecBurron = CreateWindow( 
        L"BUTTON",  // Predefined class; Unicode assumed 
        L"Recursive",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX,  // Styles 
        x + 110,         // x position 
        y,         // y position 
        100,        // Button width
        25,        // Button height
        hwnd,     // Parent window
        group.hRecursiveButton,
        hInstance, 
        NULL);      // Pointer not needed.
    
    HWND hwndRegExEdit = CreateWindowEx(
        WS_EX_CLIENTEDGE, 
        L"EDIT", 
        L"", 
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 
        x, 
        y + 45, 
        200, 
        25, 
        hwnd, 
        group.hRegexField, 
        hInstance, 
        NULL);

    HWND hwndMaxDepth = CreateWindow( 
        L"EDIT",  // Predefined class; Unicode assumed 
        L"0",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | ES_NUMBER,  // Styles 
        x,         // x position 
        y + 80,     // y position 
        100,        // Button width
        25,        // Button height
        hwnd,     // Parent window
        group.hMaxDepthField,
        hInstance, 
        NULL);      // Pointer not needed.

    HWND hwndModeSelect = CreateWindow( 
        L"COMBOBOX",  // Predefined class; Unicode assumed 
        L"",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST,  // Styles 
        x,         // x position 
        y + 115,     // y position 
        200,        // Button width
        100,        // Button height
        hwnd,     // Parent window
        group.hModeSelect,
        hInstance, 
        NULL);      // Pointer not needed.

    SendMessage(hwndModeSelect, CB_ADDSTRING, (WPARAM) 0, (LPARAM) L"Files Only");
    SendMessage(hwndModeSelect, CB_ADDSTRING, (WPARAM) 0, (LPARAM) L"Directories Only");
    SendMessage(hwndModeSelect, CB_ADDSTRING, (WPARAM) 0, (LPARAM) L"Files and Directories");
    SendMessage(hwndModeSelect, CB_SETCURSEL, (WPARAM) 2, (LPARAM) 0);

    HWND hwndPrioritySelect = CreateWindow( 
        L"COMBOBOX",  // Predefined class; Unicode assumed 
        L"",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST,  // Styles 
        x,         // x position 
        y + 150,     // y position 
        200,        // Button width
        200,        // Button height
        hwnd,     // Parent window
        group.hPriorityField,
        hInstance, 
        NULL);      // Pointer not needed.
    
    SendMessage(hwndPrioritySelect, CB_ADDSTRING, (WPARAM) 0, (LPARAM) L"Idle");
    SendMessage(hwndPrioritySelect, CB_ADDSTRING, (WPARAM) 0, (LPARAM) L"Below Normal");
    SendMessage(hwndPrioritySelect, CB_ADDSTRING, (WPARAM) 0, (LPARAM) L"Normal");
    SendMessage(hwndPrioritySelect, CB_ADDSTRING, (WPARAM) 0, (LPARAM) L"Above Normal");
    SendMessage(hwndPrioritySelect, CB_ADDSTRING, (WPARAM) 0, (LPARAM) L"High");
    SendMessage(hwndPrioritySelect, CB_ADDSTRING, (WPARAM) 0, (LPARAM) L"Realtime");
    SendMessage(hwndPrioritySelect, CB_SETCURSEL, (WPARAM) 2, (LPARAM) 0);

    HWND hwndStartButton = CreateWindow( 
        L"BUTTON",  // Predefined class; Unicode assumed 
        L"Start",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
        x,         // x position 
        y + 200,         // y position 
        100,        // Button width
        25,        // Button height
        hwnd,     // Parent window
        group.hStartButton,
        hInstance, 
        NULL);      // Pointer not needed.

    HWND hwndResutsListView = CreateWindow(
        WC_LISTVIEW,
        L"Results",
        WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SHOWSELALWAYS,
        x, 
        y + 250, 
        200, 
        200, 
        hwnd, 
        group.hResultList,
        hInstance, 
        NULL);

    return 0;
}                       

void messageConfig(HWND hwnd, ThreadConfig *config)
{
    WCHAR szMsg[200];
    wsprintf(
        szMsg, 
        L"Path: %s\nRecursive: %s\nMax depth: %d\nMode: %s\nPriority: %s",
        config->szPath,
        config->recursive ? L"true" : L"false",
        config->maxDepth,
        (
            config->mode == MODE_FILE_ONLY ? L"Files Only" : 
            config->mode == MODE_DIR_ONLY ? L"Directories Only" : 
            L"Files and Directories"
        ),
        (
            config->priority == THREAD_PRIORITY_IDLE ? L"Idle" : 
            config->priority == THREAD_PRIORITY_BELOW_NORMAL ? L"Below Normal" : 
            config->priority == THREAD_PRIORITY_NORMAL ? L"Normal" : 
            config->priority == THREAD_PRIORITY_ABOVE_NORMAL ? L"Above Normal" : 
            config->priority == THREAD_PRIORITY_HIGHEST ? L"High" : 
            L"Realtime"
        )
        ); 
    MessageBox(hwnd, szMsg, L"Thread config", MB_OK);
}

void scanDir(ThreadConfig *config, std::queue<std::wstring> *paths)
{
    WIN32_FIND_DATAW findData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WCHAR szPath[MAX_PATH];
    WCHAR szPathPure[MAX_PATH];
    wcscpy(szPath, paths->back().c_str());
    wcscat(szPath, L"\\*");
    wcscpy(szPathPure, paths->back().c_str());
    paths->pop();

    hFind = FindFirstFile(szPath, &findData);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        MessageBox(NULL, L"Invalid path", L"Error", MB_OK);
        return;
    }

    while (FindNextFile(hFind, &findData) != 0)
    {
        if (wcscmp(findData.cFileName, L".") == 0 || wcscmp(findData.cFileName, L"..") == 0)
        {
            continue;
        }

        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if (config->mode == MODE_FILE_ONLY)
            {
                continue;
            }

            if (config->recursive && config->maxDepth > 0)
            {
                auto newPath = std::wstring(szPathPure) + L"\\" + findData.cFileName;
                paths->emplace(newPath);
            }
        }
        else
        {
            if (config->mode == MODE_DIR_ONLY)
            {
                continue;
            }
        }
        
        if (!std::regex_search(std::wstring(findData.cFileName), *config->regex)) {
            continue;
        }
        WCHAR *szFileName = (WCHAR *) malloc(sizeof(WCHAR) * (wcslen(findData.cFileName) + 1));
        wcscpy(szFileName, findData.cFileName);
        config->results.push_back(szFileName);
    }

    FindClose(hFind);
}

DWORD WINAPI procThread(LPVOID lpParameter) {
    ThreadConfig *config = (ThreadConfig *) lpParameter;
    std::queue<std::wstring> paths;
    paths.push(config->szPath);

    if (!config->recursive) {
        config->maxDepth = 1;
    }

    while(paths.size() > 0 && config->maxDepth > 0) {
        scanDir(config, &paths);
        config->maxDepth--;
    }

    return 0;
}

void startThread(
    HWND hwnd, 
    ThreadConfig *config,
    int startButtonId,
    int maxDepthEditId,
    int resultListViewId,
    int regexEditId
) {
    EnableWindow(GetDlgItem(hwnd, startButtonId), FALSE);
    WCHAR buffer[100];
    GetDlgItemText(hwnd, maxDepthEditId, buffer, 10);
    config->maxDepth = _wtoi(buffer);
    GetDlgItemText(hwnd, regexEditId, buffer, 100);
    delete config->regex;
    config->regex = new std::wregex(buffer);
    // config->regex = new std::wregex(L"\\.\\D{2}");
    messageConfig(hwnd, config);
    auto hThread = CreateThread(
        NULL, 
        0, 
        (LPTHREAD_START_ROUTINE) &procThread, 
        config, 
        0, 
        NULL);
    SetThreadPriority(hThread, config->priority);
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    LVCOLUMN lvc;
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    lvc.fmt = LVCFMT_LEFT;
    lvc.cx = 200;
    lvc.pszText = L"Path";
    lvc.iSubItem = 0;
    ListView_InsertColumn(GetDlgItem(hwnd, resultListViewId), 0, &lvc);
    for (auto it = config->results.begin(); it != config->results.end(); ++it) {
        LVITEM lvi;
        lvi.mask = LVIF_TEXT;
        lvi.iItem = ListView_GetItemCount(GetDlgItem(hwnd, ID_RESULT_LISTVIEW1));
        lvi.iSubItem = 0;
        lvi.pszText = *it;
        ListView_InsertItem(GetDlgItem(hwnd, resultListViewId), &lvi);
    }
    if (config->szPath != NULL) {
        free(config->szPath);
    }
    config->szPath = NULL;
    delete config->regex;
    config->regex = NULL;
    config->results.clear();
}
