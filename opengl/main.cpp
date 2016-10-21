#include "platform_win_posix.h"
#include "mp_wrap.h"

HWND		hWnd=NULL;		// Holds Our Window Handle
HINSTANCE	hInstance;		// Holds The Instance Of The Application

BOOL fDone=false;
HM_EMITTER cur=0;

// Global Variables:
HINSTANCE hInst;						// current instance
char* szTitle="Magic Particles (www.astralax.com) wrapper for OpenGL";// The title bar text
char* szWindowClass="EXAMPLE";			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;

	// Initialize global strings
	MyRegisterClass(hInstance);

	int window_wi=806;
	int window_he=632;

	int width=GetSystemMetrics(SM_CXSCREEN);
	int height=GetSystemMetrics(SM_CYSCREEN);

	int bx=(width-window_wi)/2;
	int by=(height-window_he)/2;

	hInst = hInstance; // Store instance handle in our global variable
	hWnd=CreateWindow(szWindowClass, szTitle, WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, bx, by, window_wi, window_he, NULL, NULL, hInstance, NULL);

	if (!hWnd)
		return FALSE;
	
	RECT rect;
	GetClientRect(hWnd,&rect);
	int client_wi=rect.right-rect.left;
	int client_he=rect.bottom-rect.top;

	MP_Device_WRAP device(client_wi, client_he, hWnd, 32);
	device.Create();

	MP_Manager& MP=MP_Manager::GetInstance();

	MP_Platform* platform=new MP_Platform_WIN_POSIX;
	#ifdef MAGIC_3D
	MAGIC_AXIS_ENUM axis=MAGIC_pXpYnZ;
	#else
	MAGIC_AXIS_ENUM axis=MAGIC_pXnY;
	#endif

	#ifdef SHADER_WRAP
	bool filters[MAGIC_RENDER_STATE__MAX];
	for (int i=0;i<MAGIC_RENDER_STATE__MAX;i++)
		filters[i]=false;
	filters[MAGIC_RENDER_STATE_BLENDING]=true;
	filters[MAGIC_RENDER_STATE_TEXTURE_COUNT]=true;
	filters[MAGIC_RENDER_STATE_TEXTURE]=true;
	filters[MAGIC_RENDER_STATE_ADDRESS_U]=true;
	filters[MAGIC_RENDER_STATE_ADDRESS_V]=true;
	filters[MAGIC_RENDER_STATE_ZENABLE]=true;
	filters[MAGIC_RENDER_STATE_ZWRITE]=true;
	#ifndef SHADER_ALPHATEST_WRAP
	filters[MAGIC_RENDER_STATE_ALPHATEST_INIT]=true;
	filters[MAGIC_RENDER_STATE_ALPHATEST]=true;
	#endif
	#else
	bool* filters=NULL;
	#endif
	MP.Initialization(filters, true, axis, platform, MAGIC_INTERPOLATION_ENABLE, MAGIC_CHANGE_EMITTER_DEFAULT, 1024, 1024, 1, 1.f, 0.1f, true);

	// eng: find of all ptc-files in folder
	// rus: поиск всех ptc-файлов в папке
	MP.LoadAllEmitters();

	MP.RefreshAtlas();

	MP.CloseFiles();

	MP.Stop();

	cur=MP.GetFirstEmitter();

	MP_Emitter* emitter=MP.GetEmitter(cur);
	emitter->SetState(MAGIC_STATE_UPDATE);

	#ifdef MAGIC_3D
	bool perspective=true;
	#else
	bool perspective=false;
	#endif
	device.SetScene3d(perspective);
	
	#ifndef MAGIC_3D
	// eng: locate emitters the same as editor
	// rus: расставляем эмиттеры также, как они стояли в редакторе
	HM_EMITTER hmEmitter=MP.GetFirstEmitter();
	while (hmEmitter)
	{
		Magic_CorrectEmitterPosition(hmEmitter, client_wi, client_he);
		hmEmitter=MP.GetNextEmitter(hmEmitter);
	}
	#endif


	ShowWindow(hWnd,SW_SHOW);

	// Main message loop:
	while (!fDone)
	{
		if (PeekMessage(&msg,hWnd,0,0,PM_REMOVE))
		{
			if (msg.message!=WM_QUIT)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				fDone=TRUE;
			}
		}
		else
		{
			if (GetFocus()==hWnd)
			{
				MP.UpdateByTimer();

				MP.Render();

				emitter=MP.GetEmitter(cur);
				if (emitter->GetState()==MAGIC_STATE_STOP)
				{
					cur=MP.GetNextEmitter(cur);
					if (!cur)
						cur=MP.GetFirstEmitter();

					emitter=MP.GetEmitter(cur);
					emitter->SetState(MAGIC_STATE_UPDATE);
				}
			}
		}
	}

	// eng: Delete particle manager
	// rus: Удаляем менеджер частиц
	device.Destroy();
	MP.Destroy();

	DestroyWindow(hWnd);

	return 0;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
	{
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	}

	case WM_ERASEBKGND:
		break;

	case WM_CLOSE:
		fDone=TRUE;
		break;
	
	case WM_KEYDOWN:
		if (wParam==VK_ESCAPE)
		{
			fDone=TRUE;
			break;
		}

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(0, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
}
