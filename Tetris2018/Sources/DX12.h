#pragma once

#include <windows.h>
#include <d3d12.h>
#pragma comment(lib, "d3d12.lib")
#include <dxgi1_6.h>
#pragma comment(lib, "dxgi.lib")
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")
#include <DirectXMath.h>
#include "d3dx12.h"
#include <wrl.h>
#include <shellapi.h>
#include <string>
#include <exception>
#include <stdexcept>

#include <initguid.h>
#include <dxgidebug.h>

using namespace std;
using namespace DirectX;
using namespace Microsoft::WRL;
using Microsoft::WRL::ComPtr;

class DX12
{
public:
	static const UINT FrameCount = 2;

	DX12(HWND hWnd, UINT width, UINT height, TCHAR* name);
	virtual ~DX12();

	void Initialize();
	void Update();
	void Render();
	void Destroy();

private:
	HWND _hWnd;
	bool _useWarpDevice;
	UINT _width;
	UINT _height;

	ComPtr<IDXGISwapChain4> _swapChain;
	ComPtr<ID3D12Device> _device;
	ComPtr<ID3D12Resource> _renderTargets[FrameCount];
	ComPtr<ID3D12CommandAllocator> _commandAllocator;
	ComPtr<ID3D12CommandQueue> _commandQueue;
	ComPtr<ID3D12GraphicsCommandList> _commandList;
	ComPtr<ID3D12DescriptorHeap> _renderTargetViewHeap;
	ComPtr<ID3D12PipelineState> _pipelineState;
	UINT _renderTargetViewDescriptorSize;

	UINT _frameIndex;
	HANDLE _fenceEvent;
	ComPtr<ID3D12Fence> _fence;
	UINT64 _fenceValue;
	


	void LoadPipeline();
	void LoadAssets();

	void PopulateCommandList();
	void WaitForPreviousFrame();
	void GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter);
};

inline string HrToString(HRESULT hr)
{
	char s_str[64] = {};
	sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
	return string(s_str);
}

class HrException : public runtime_error
{
public:
	HrException(HRESULT hr) : runtime_error(HrToString(hr)),_hr(hr){}
	HRESULT Error() const { return _hr; }

private:
	const HRESULT _hr;
};

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw HrException(hr);
	}
}

template<typename T>
void SafeDelete(T*& p)
{
	if (nullptr != p)
	{
		delete(p);
		p = nullptr;
	}
}


#define SAFE_RELEASE(p) if (p) (p)->Release()