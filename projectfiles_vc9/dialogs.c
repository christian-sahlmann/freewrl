
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <Commdlg.h>
#include <Cderr.h>
#include <malloc.h>
char *strBackslash2fore(char *str);
char* esPickFile(HWND hwnd)
{
//	http://msdn.microsoft.com/en-us/library/windows/desktop/ms646927(v=vs.85).aspx
	char * fname;
	OPENFILENAME *LPOPENFILENAME;
	//               123456789 123456789 123456 789 1234567 8 9
	char * filter = "web3d Files (*.x3d;*.wrl) \0*.x3d;*.wrl\0\0";
	fname = (char *)malloc(4096);
	memset(fname,0,4096);
	LPOPENFILENAME = (OPENFILENAME *)malloc(sizeof(OPENFILENAME));
	memset(LPOPENFILENAME,0,sizeof(OPENFILENAME));

	LPOPENFILENAME->lStructSize = sizeof(OPENFILENAME);
	LPOPENFILENAME->lpstrFilter = filter;
	LPOPENFILENAME->lpstrFile = (LPSTR)fname;
	LPOPENFILENAME->nMaxFile = (DWORD)(1024 - 1);

	if(GetOpenFileName(LPOPENFILENAME))
	{
		fname = strBackslash2fore(fname);
	}else{
		DWORD error = CommDlgExtendedError();
		switch(error){
			case CDERR_DIALOGFAILURE: printf("CDERR_DIALOGFAILURE");break;
			case CDERR_FINDRESFAILURE: printf("CDERR_FINDRESFAILURE");break;
			case CDERR_INITIALIZATION: printf("CDERR_INITIALIZATION");break;
			case CDERR_LOADRESFAILURE: printf("CDERR_LOADRESFAILURE");break;
			case CDERR_LOADSTRFAILURE: printf("CDERR_LOADSTRFAILURE");break;
			case CDERR_LOCKRESFAILURE: printf("CDERR_LOCKRESFAILURE");break;
			case CDERR_MEMALLOCFAILURE: printf("CDERR_MEMALLOCFAILURE");break;
			case CDERR_MEMLOCKFAILURE: printf("CDERR_MEMLOCKFAILURE");break;
			case CDERR_NOHINSTANCE: printf("CDERR_NOHINSTANCE");break;
			case CDERR_NOHOOK: printf("CDERR_NOHOOK");break;
			case CDERR_NOTEMPLATE: printf("CDERR_NOTEMPLATE");break;
			case CDERR_STRUCTSIZE: printf("CDERR_STRUCTSIZE");break;
			case FNERR_BUFFERTOOSMALL: printf("FNERR_BUFFERTOOSMALL");break;
			case FNERR_INVALIDFILENAME: printf("FNERR_INVALIDFILENAME");break;
			case FNERR_SUBCLASSFAILURE: printf("FNERR_SUBCLASSFAILURE");break;
			default: printf("default");break;
		}
		free(fname);
		fname = 0;
	}
	return fname;
}
#include "resource.h"
//message handler for URL dialog
//you need the winGLES2.rc in the freeWRL (main program) > Resource Files to get the popup
static char szItemName[2048]; // receives name of item to delete. 
 
BOOL CALLBACK URLItemProc(HWND hwndDlg, 
                             UINT message, 
                             WPARAM wParam, 
                             LPARAM lParam) 
{ 
    switch (message) 
    { 
		case WM_INITDIALOG:
			SetDlgItemText(hwndDlg,IDC_EDIT_URL,szItemName);
			return TRUE;

        case WM_COMMAND: 
            switch (LOWORD(wParam)) 
            { 
                case IDOK: 
                    if (!GetDlgItemText(hwndDlg, IDC_EDIT_URL, szItemName, 2048))
					{
                         szItemName[0]=0; 
						 return FALSE;
					}
                    EndDialog(hwndDlg, wParam); 
					return IDOK;
 
                case IDCANCEL: 
                    EndDialog(hwndDlg, wParam); 
                    return IDCANCEL; 
            } 
    } 
    return FALSE; 
} 

char * esPickURL(HWND hWnd)
{
	HINSTANCE hInst;
	char *fname;
    hInst = (HINSTANCE)GetModuleHandle(NULL); 

	fname = NULL;
	if(DialogBox(hInst, MAKEINTRESOURCE(IDD_ENTER_URL), hWnd, (DLGPROC)URLItemProc)==IDOK)
	{
		fname = malloc(strlen(szItemName)+1);
		strcpy(fname,szItemName);
	}else{
		fname = NULL;
	}
	//printf("hi from esPickURL URL=%s\n",fname);
	return fname;
}
HWND fw_window32_hwnd();

char *frontend_pick_file()
{
	return esPickFile(fw_window32_hwnd());
}
char *frontend_pick_URL()
{
	return esPickURL(fw_window32_hwnd());
}