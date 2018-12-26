#include <Windows.h>
#include <locale.h>
#include <d3d12.h>

#include "Log.h"
#include "DX12.h"
#include "HeapFactory.h"
#include "GameObject.h"

using N1D::Graphic::DX12;

static DX12* pGraphic = nullptr;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	

	switch (msg)
	{
	case WM_DESTROY:
		{
			//MessageBox(hwnd, TEXT("Exit"), TEXT("Exit"), MB_ICONINFORMATION);
			PostQuitMessage(0);


#ifdef _DEBUG
			IDXGIDebug1* pDebug = nullptr;
			if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
			{
				pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
				pDebug->Release();
			}
#endif
		}
		return 0;

	case WM_CREATE:
		{
			return 0;
			LPCREATESTRUCT windowSettings = (LPCREATESTRUCT)lp;
			int result = MessageBox(hwnd,
				TEXT("ウィンドウを生成しますか？"), windowSettings->lpszName,
				MB_YESNO | MB_ICONQUESTION);
			return (IDYES == result) ? 0 : -1;
		}

	case WM_PAINT:
		{
			if (pGraphic)
			{
				pGraphic->Update();
				pGraphic->Render();
			}
		}
		break;

	default:
		return DefWindowProc(hwnd, msg, wp, lp);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	N1D::Log::Write("test %d %s\n", 10, "ねむすぎわろた");
	int bookmark = N1D::HeapFactory::GetMemoryBookmark();

	N1D::GameObject* obj = new N1D::GameObject;

	WNDCLASS windowClass;
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WndProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = hInstance;
	windowClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	windowClass.lpszMenuName = nullptr;
	windowClass.lpszClassName = TEXT("TETRIS2018");

	if (!RegisterClass(&windowClass))
	{
		return 0;
	}

	HWND windowHandle = CreateWindow(
		TEXT("TETRIS2018"), TEXT("TETRIS2018"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
		nullptr, nullptr, hInstance, nullptr);

	if (nullptr == windowHandle)
	{
		return 0;
	}

	DX12 graphic(windowHandle, 800, 600, nullptr);
	pGraphic = &graphic;
	pGraphic->Initialize();

	ShowWindow(windowHandle, SW_SHOW);
	UpdateWindow(windowHandle);

	MSG message;
	while (1)
	{
		if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
		{
			if (WM_QUIT == message.message)
			{
				break;
			}
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
	}
	
	pGraphic->Destroy();

	SafeDelete(obj);
	N1D::HeapFactory::PrintInfo();

	N1D::HeapFactory::ReportMemoryLeaks(bookmark);

	return (UINT)message.wParam;
}