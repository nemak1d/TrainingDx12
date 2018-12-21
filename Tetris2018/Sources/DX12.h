//************************************************************************
/*
 *	DX12.h
 */
 //************************************************************************
#ifndef DX12_H
#define DX12_H

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"
#include <wrl.h>
#include <shellapi.h>

#ifdef _DEBUG
#include <initguid.h>
#include <dxgidebug.h>
#endif // _DEBUG

using namespace std;
using namespace DirectX;
using namespace Microsoft::WRL;

namespace N1D::Graphic
{
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

		void EnableDebugAssist(UINT& dstFactoryFlags);
		void ReportLiveObjects();
	};
}
#endif // DX12_H

