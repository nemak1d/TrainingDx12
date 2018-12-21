


#include "DX12.h"
#include "HrException.h"

namespace N1D::Graphic
{
	DX12::DX12(HWND hWnd, UINT width, UINT height, TCHAR* name) :
		_hWnd(hWnd),
		_useWarpDevice(false),
		_frameIndex(0),
		_renderTargetViewDescriptorSize(0) {}

	DX12::~DX12()
	{
	}

	void DX12::Initialize()
	{
		LoadPipeline();
		LoadAssets();
	}

	void DX12::Update()
	{

	}

	void DX12::Render()
	{
		PopulateCommandList();

		ID3D12CommandList* ppCommandLists[] = { _commandList.Get() };
		_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		ThrowIfFailed(_swapChain->Present(1, 0));

		WaitForPreviousFrame();
	}

	void DX12::Destroy()
	{
		WaitForPreviousFrame();
		CloseHandle(_fenceEvent);

		ReportLiveObjects();
	}

	void DX12::LoadPipeline()
	{
		UINT dxgiFactoryFlags = 0;

		EnableDebugAssist(dxgiFactoryFlags);

		ComPtr<IDXGIFactory4> factory;
		ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

		if (_useWarpDevice)
		{
			ComPtr<IDXGIAdapter> warpAdapter;
			ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
			ThrowIfFailed(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&_device)));
		}
		else
		{
			ComPtr<IDXGIAdapter1> hardwareAdapter = nullptr;
			GetHardwareAdapter(factory.Get(), &hardwareAdapter);
			ThrowIfFailed(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&_device)));
		}

		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		ThrowIfFailed(_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&_commandQueue)));

		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.BufferCount = FrameCount;
		swapChainDesc.Width = _width;
		swapChainDesc.Height = _height;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;

		ComPtr<IDXGISwapChain1> swapChain;
		ThrowIfFailed(factory->CreateSwapChainForHwnd(
			_commandQueue.Get(),
			_hWnd,
			&swapChainDesc,
			nullptr,
			nullptr,
			&swapChain));

		ThrowIfFailed(factory->MakeWindowAssociation(_hWnd, DXGI_MWA_NO_ALT_ENTER));
		ThrowIfFailed(swapChain.As(&_swapChain));
		_frameIndex = _swapChain->GetCurrentBackBufferIndex();

		{
			D3D12_DESCRIPTOR_HEAP_DESC renderTargetViewHeapDesc = {};
			renderTargetViewHeapDesc.NumDescriptors = FrameCount;
			renderTargetViewHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			renderTargetViewHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			ThrowIfFailed(_device->CreateDescriptorHeap(
				&renderTargetViewHeapDesc, IID_PPV_ARGS(&_renderTargetViewHeap)));

			_renderTargetViewDescriptorSize =
				_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		}

		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE renderTargetViewHandle(
				_renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart());
			for (UINT i = 0; i < FrameCount; ++i)
			{
				ThrowIfFailed(_swapChain->GetBuffer(i, IID_PPV_ARGS(&_renderTargets[i])));
				_device->CreateRenderTargetView(_renderTargets[i].Get(), nullptr, renderTargetViewHandle);
				renderTargetViewHandle.Offset(1, _renderTargetViewDescriptorSize);
			}
		}

		ThrowIfFailed(_device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_commandAllocator)));
	}

	void DX12::LoadAssets()
	{
		ThrowIfFailed(_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
			_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&_commandList)));

		ThrowIfFailed(_commandList->Close());

		{
			ThrowIfFailed(_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence)));
			_fenceValue = 1;

			_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			if (nullptr == _fenceEvent)
			{
				ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
			}
		}
	}

	void DX12::GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter)
	{
		ComPtr<IDXGIAdapter1> adapter;
		*ppAdapter = nullptr;

		for (UINT adapterIndex = 0;
			DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);

			if (DXGI_ADAPTER_FLAG_SOFTWARE & desc.Flags)
			{
				continue;
			}
			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
			{
				break;
			}
		}
		*ppAdapter = adapter.Detach();
	}

	void DX12::PopulateCommandList()
	{
		ThrowIfFailed(_commandAllocator->Reset());

		ThrowIfFailed(_commandList->Reset(_commandAllocator.Get(), _pipelineState.Get()));

		_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			_renderTargets[_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

		CD3DX12_CPU_DESCRIPTOR_HANDLE renderTargetViewHandle(
			_renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart(),
			_frameIndex, _renderTargetViewDescriptorSize);

		const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		_commandList->ClearRenderTargetView(renderTargetViewHandle, clearColor, 0, nullptr);

		_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			_renderTargets[_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

		ThrowIfFailed(_commandList->Close());
	}

	void DX12::WaitForPreviousFrame()
	{
		const UINT64 fence = _fenceValue;
		ThrowIfFailed(_commandQueue->Signal(_fence.Get(), fence));
		++_fenceValue;

		if (_fence->GetCompletedValue() < fence)
		{
			ThrowIfFailed(_fence->SetEventOnCompletion(fence, _fenceEvent));
			WaitForSingleObject(_fenceEvent, INFINITE);
		}
		_frameIndex = _swapChain->GetCurrentBackBufferIndex();
	}
	
	void DX12::EnableDebugAssist(UINT& dstFactoryFlags)
	{
#ifdef _DEBUG
		{
			ComPtr<ID3D12Debug> debugController;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
			{
				debugController->EnableDebugLayer();
				dstFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
			}
		}
#endif
	}

	void DX12::ReportLiveObjects()
	{
#ifdef _DEBUG
		IDXGIDebug1* pDebug = nullptr;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
		{
			pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
			pDebug->Release();
		}
#endif
	}
}


