#include "OS-ImGui_Internal.h"
#include "OS-ImGui.h"

/****************************************************
* Copyright (C)	: Liv
* @file			: OS-ImGui_Internal.cpp
* @author		: Liv
* @email		: 1319923129@qq.com
* @version		: 1.1
* @date			: 2024/4/5 13:00
****************************************************/

#ifdef OSIMGUI_INTERNAL

typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
typedef HRESULT(__stdcall* Present) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef HRESULT(__stdcall* Resize)(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
typedef HRESULT(__stdcall* EndScene)(LPDIRECT3DDEVICE9);
typedef HRESULT(__stdcall* Reset)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);

// DirectX 9
inline EndScene oEndScene;
inline Reset oReSet;
// DirectX 11
inline Present oPresent;
inline Resize oResize;

//DX12:
namespace DirectX12Interface {
	ID3D12Device* Device = nullptr;
	ID3D12DescriptorHeap* DescriptorHeapBackBuffers;
	ID3D12DescriptorHeap* DescriptorHeapImGuiRender;
	ID3D12GraphicsCommandList* CommandList;
	ID3D12CommandQueue* CommandQueue;
	long g_ScreenWidth;
	long g_ScreenHeight;
	struct _FrameContext {
		ID3D12CommandAllocator* CommandAllocator;
		ID3D12Resource* Resource;
		D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHandle;
	};

	uintx_t BuffersCounts = -1;
	_FrameContext* FrameContext;
}

ExecuteCommandLists oExecuteCommandLists = NULL;
DrawIndexedInstanced oDrawIndexedInstanced = NULL;
DrawInstanced oDrawInstanced = NULL;
Present12 oPresent12 = NULL;

void hkExecuteCommandLists(ID3D12CommandQueue* queue, UINT NumCommandLists, ID3D12CommandList* ppCommandLists) {
	if (!DirectX12Interface::CommandQueue)
		DirectX12Interface::CommandQueue = queue;
	oExecuteCommandLists(queue, NumCommandLists, ppCommandLists);
}
void APIENTRY hkDrawInstanced(ID3D12GraphicsCommandList* dCommandList, UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation) {

	return oDrawInstanced(dCommandList, VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
}
void APIENTRY hkDrawIndexedInstanced(ID3D12GraphicsCommandList* dCommandList, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation) {

	/*
	//cyberpunk 2077 no pants hack (low settings)
	if (nopants_enabled)
		if (IndexCountPerInstance == 10068 || //bargirl pants near
			IndexCountPerInstance == 3576) //med range
			return; //delete texture

	if (GetAsyncKeyState(VK_F12) & 1) //toggle key
		nopants_enabled = !nopants_enabled;


	//logger, hold down B key until a texture disappears, press END to log values of those textures
	if (GetAsyncKeyState('V') & 1) //-
		countnum--;
	if (GetAsyncKeyState('B') & 1) //+
		countnum++;
	if (GetAsyncKeyState(VK_MENU) && GetAsyncKeyState('9') & 1) //reset, set to -1
		countnum = -1;

	if (countnum == IndexCountPerInstance / 100)
		if (GetAsyncKeyState(VK_END) & 1) //log
			Log("IndexCountPerInstance == %d && InstanceCount == %d",
				IndexCountPerInstance, InstanceCount);

	if (countnum == IndexCountPerInstance / 100)
		return;
	*/

	return oDrawIndexedInstanced(dCommandList, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
}

bool CreateHook(uint16_t Index, void** Original, void* Function, uintx_t* MethodsTable) {
	assert(Index >= 0 && Index != NULL && Function != NULL);
	void* target = (void*)MethodsTable[Index];
	if (MH_CreateHook(target, Function, Original) != MH_OK || MH_EnableHook(target) != MH_OK) {
		return false;
	}
	return true;
}


inline WNDPROC oWndProc = NULL;
inline WNDPROC dx_12oWndProc = NULL;
inline bool IsDx11Init = false, IsDx9Init = false,IsDx12Init=false;
HWND hWnd_dx12 = NULL;
namespace OSImGui
{
	void OSImGui_Internal::Start(HMODULE hLibModule, std::function<void()> CallBack, DirectXType DxType)
	{
		this->DxType = DxType;

		this->CallBackFn = CallBack;

		this->hLibModule = hLibModule;

		MH_Initialize();

		std::thread(&OSImGui_Internal::InitThread, this).detach();
	}

	void OSImGui_Internal::ReleaseHook()
	{
		if (this->HookData.HookList.size() > 0)
			MH_DisableHook(MH_ALL_HOOKS);

		for (auto Target : this->HookData.HookList)
		{
			MH_RemoveHook(Target);
		}
		this->HookData.HookList.clear();

		MH_Uninitialize();
	}

	bool OSImGui_Internal::CreateMyWindow()
	{
		WNDCLASSEXA wc = { sizeof(wc), CS_HREDRAW | CS_VREDRAW, DefWindowProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, "OS-ImGui_Internal", NULL };
		RegisterClassExA(&wc);

		this->Window.hInstance = wc.hInstance;

		this->Window.hWnd = ::CreateWindowA(wc.lpszClassName, "OS-ImGui_Overlay", WS_OVERLAPPEDWINDOW, 0, 0, 50, 50, NULL, NULL, wc.hInstance, NULL);

		this->Window.ClassName = wc.lpszClassName;

		return this->Window.hWnd != NULL;
	}

	void OSImGui_Internal::DestroyMyWindow()
	{
		if (this->Window.hWnd)
		{
			DestroyWindow(this->Window.hWnd);
			UnregisterClassA(this->Window.ClassName.c_str(), this->Window.hInstance);
			this->Window.hWnd = NULL;
		}
	}

	void OSImGui_Internal::InitThread()
	{
		if (this->DxType == DirectXType::AUTO)
		{
			if (GetModuleHandleA("d3d11.dll")) 
				this->DxType = DirectXType::DX11;
			else if (GetModuleHandleA("d3d9.dll")) 
				this->DxType = DirectXType::DX9;
			else if (GetModuleHandleA("d3d12.dll")) 
				this->DxType = DirectXType::DX12;
			else
				goto END;
		}

		do
		{
			if (this->DxType == DirectXType::DX11)
			{
				if (this->InitDx11Hook())break;
			}
			else if(this->DxType == DirectXType::DX9)
			{
				if (this->InitDx9Hook())break;
			}
			else if (this->DxType == DirectXType::DX12) {
				if (InitDx12Hook())break;
			}
			else goto END;
			std::this_thread::sleep_for(std::chrono::milliseconds(100));

		} while (true);

		while (true)
		{
			if (this->FreeDLL)
			{
			END:
				FreeLibrary(this->hLibModule);
				std::cerr<<"OSImgui already quit"<<std::endl;
				return;
			}
			this->Window.Size.x=(float)DirectX12Interface::g_ScreenWidth;
			this->Window.Size.y=(float)DirectX12Interface::g_ScreenHeight;
			//std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}

	LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (Gui.get().DxType == DirectXType::DX11 || Gui.get().DxType == DirectXType::DX9) {
			if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
				return true;
			return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
		}
		else {
			if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
				return true;
			return CallWindowProc(dx_12oWndProc, hWnd, uMsg, wParam, lParam);
		}
	}

	HRESULT __stdcall hkResize(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
	{
		if (g_Device.g_mainRenderTargetView)
		{
			g_Device.g_pd3dDeviceContext->OMSetRenderTargets(0, 0, 0);
			g_Device.g_mainRenderTargetView->Release();
		}

		HRESULT Result = oResize(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);

		ID3D11Texture2D* pBackBuffer = nullptr;

		pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
		g_Device.g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_Device.g_mainRenderTargetView);
		pBackBuffer->Release();

		g_Device.g_pd3dDeviceContext->OMSetRenderTargets(1, &g_Device.g_mainRenderTargetView, NULL);

		D3D11_VIEWPORT ViewPort{ 0,0,Width,Height,0.0f,1.0f };

		g_Device.g_pd3dDeviceContext->RSSetViewports(1, &ViewPort);

		return Result;
	}
	
	HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
	{
		if (!IsDx11Init)
		{
			if (OSImGui::get().InitDx11(pSwapChain))
				IsDx11Init = true;
			else
				return oPresent(pSwapChain, SyncInterval, Flags);

			oWndProc = (WNDPROC)SetWindowLongPtrA(OSImGui::get().Window.hWnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
			
			try {
				OSImGui::get().InitImGui(g_Device.g_pd3dDevice, g_Device.g_pd3dDeviceContext);
			}
			catch (OSException& e)
			{
				OSImGui::get().CleanDx11();
				IsDx11Init = false;

				std::cout << e.what() << std::endl;
				return oPresent(pSwapChain, SyncInterval, Flags);
			}
		}

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		OSImGui::get().CallBackFn();

		ImGui::Render();

		g_Device.g_pd3dDeviceContext->OMSetRenderTargets(1, &g_Device.g_mainRenderTargetView, NULL);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		if (OSImGui::get().EndFlag)
		{
			oPresent(pSwapChain, SyncInterval, Flags);

			OSImGui::get().ReleaseHook();
			OSImGui::get().CleanImGui();
			OSImGui::get().CleanDx11();

			oWndProc = (WNDPROC)SetWindowLongPtrA(OSImGui::get().Window.hWnd, GWLP_WNDPROC, (LONG_PTR)(oWndProc));

			OSImGui::get().FreeDLL = true;

			return NULL;
		}

		return oPresent(pSwapChain, SyncInterval, Flags);
	}
	HRESULT __stdcall hkPresent12(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags) {
		if (!IsDx12Init) {
			// ��ȡ���豸
			if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D12Device), (void**)&DirectX12Interface::Device))) {
				// ���� ImGui ������
				ImGui::CreateContext();
				ImGuiIO& io = ImGui::GetIO(); (void)io;
				io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

				// �� swapchain ��ȡ�����������ж�ȡ��ʵ�� OutputWindow��
				DXGI_SWAP_CHAIN_DESC Desc;
				if (FAILED(pSwapChain->GetDesc(&Desc)))
					return oPresent12(pSwapChain, SyncInterval, Flags);

				// �ؼ���ֱ��ʹ�� swapchain �ṩ�� OutputWindow ��Ϊ��Ϸ���� HWND
				hWnd_dx12 = Desc.OutputWindow;

				// ���� buffer count ������ FrameContext
				Desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
				DirectX12Interface::BuffersCounts = Desc.BufferCount;
				DirectX12Interface::FrameContext = new DirectX12Interface::_FrameContext[DirectX12Interface::BuffersCounts];

				// ���� ImGui ����� descriptor heap��shader visible��
				D3D12_DESCRIPTOR_HEAP_DESC DescriptorImGuiRender = {};
				DescriptorImGuiRender.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
				DescriptorImGuiRender.NumDescriptors = DirectX12Interface::BuffersCounts;
				DescriptorImGuiRender.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

				if (DirectX12Interface::Device->CreateDescriptorHeap(&DescriptorImGuiRender, IID_PPV_ARGS(&DirectX12Interface::DescriptorHeapImGuiRender)) != S_OK)
					return oPresent12(pSwapChain, SyncInterval, Flags);

				// command allocator / �����б�
				ID3D12CommandAllocator* Allocator = nullptr;
				if (DirectX12Interface::Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&Allocator)) != S_OK)
					return oPresent12(pSwapChain, SyncInterval, Flags);

				for (size_t i = 0; i < DirectX12Interface::BuffersCounts; i++) {
					DirectX12Interface::FrameContext[i].CommandAllocator = Allocator;
				}

				if (DirectX12Interface::Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, Allocator, NULL, IID_PPV_ARGS(&DirectX12Interface::CommandList)) != S_OK ||
					DirectX12Interface::CommandList->Close() != S_OK)
					return oPresent12(pSwapChain, SyncInterval, Flags);

				// back buffer descriptor heap
				D3D12_DESCRIPTOR_HEAP_DESC DescriptorBackBuffers = {};
				DescriptorBackBuffers.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
				DescriptorBackBuffers.NumDescriptors = DirectX12Interface::BuffersCounts;
				DescriptorBackBuffers.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
				DescriptorBackBuffers.NodeMask = 1;

				if (DirectX12Interface::Device->CreateDescriptorHeap(&DescriptorBackBuffers, IID_PPV_ARGS(&DirectX12Interface::DescriptorHeapBackBuffers)) != S_OK)
					return oPresent12(pSwapChain, SyncInterval, Flags);

				const auto RTVDescriptorSize = DirectX12Interface::Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
				D3D12_CPU_DESCRIPTOR_HANDLE RTVHandle = DirectX12Interface::DescriptorHeapBackBuffers->GetCPUDescriptorHandleForHeapStart();

				for (size_t i = 0; i < DirectX12Interface::BuffersCounts; i++) {
					ID3D12Resource* pBackBuffer = nullptr;
					DirectX12Interface::FrameContext[i].DescriptorHandle = RTVHandle;
					if (SUCCEEDED(pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer)))) {
						DirectX12Interface::Device->CreateRenderTargetView(pBackBuffer, nullptr, RTVHandle);
						DirectX12Interface::FrameContext[i].Resource = pBackBuffer;
						RTVHandle.ptr += RTVDescriptorSize;
					}
				}

				// �ô� swapchain �õ��� hWnd_dx12 ��ʼ�� ImGui Win32 �� DX12 ���
				if (!hWnd_dx12) {
					// û����Ч���ھ��ʱֱ�ӷ��У�����ʼ��
					return oPresent12(pSwapChain, SyncInterval, Flags);
				}

				ImGui_ImplWin32_Init(hWnd_dx12);
				ImGui_ImplDX12_Init(DirectX12Interface::Device, DirectX12Interface::BuffersCounts, DXGI_FORMAT_R8G8B8A8_UNORM,
					DirectX12Interface::DescriptorHeapImGuiRender,
					DirectX12Interface::DescriptorHeapImGuiRender->GetCPUDescriptorHandleForHeapStart(),
					DirectX12Interface::DescriptorHeapImGuiRender->GetGPUDescriptorHandleForHeapStart());
				ImGui_ImplDX12_CreateDeviceObjects();

				// �� IME/���봰�ھ�����ó���ʵ��Ϸ����
				ImGui::GetIO().ImeWindowHandle = hWnd_dx12;

				// ����Ϸ���ڵ� WndProc �滻Ϊ���ǵ� WndProc������ԭʼ���̵� dx_12oWndProc��
				dx_12oWndProc = (WNDPROC)SetWindowLongPtrA(hWnd_dx12, GWLP_WNDPROC, (LONG_PTR)WndProc);
			}
			IsDx12Init = true;
		}

		if (DirectX12Interface::CommandQueue == nullptr)
			return oPresent12(pSwapChain, SyncInterval, Flags);

		// ���һ��׼�����ˣ����� ImGui frame
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		RECT rect;
		if (hWnd_dx12) {
			if (GetClientRect(hWnd_dx12, &rect)) {
				DirectX12Interface::g_ScreenWidth  = rect.right  - rect.left;
				DirectX12Interface::g_ScreenHeight = rect.bottom - rect.top;
			}
		}
		Gui.get().CallBackFn();
		ImGui::EndFrame();

		DirectX12Interface::_FrameContext& CurrentFrameContext = DirectX12Interface::FrameContext[pSwapChain->GetCurrentBackBufferIndex()];
		CurrentFrameContext.CommandAllocator->Reset();

		D3D12_RESOURCE_BARRIER Barrier = {};
		Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		Barrier.Transition.pResource = CurrentFrameContext.Resource;
		Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

		DirectX12Interface::CommandList->Reset(CurrentFrameContext.CommandAllocator, nullptr);
		DirectX12Interface::CommandList->ResourceBarrier(1, &Barrier);
		DirectX12Interface::CommandList->OMSetRenderTargets(1, &CurrentFrameContext.DescriptorHandle, FALSE, nullptr);
		DirectX12Interface::CommandList->SetDescriptorHeaps(1, &DirectX12Interface::DescriptorHeapImGuiRender);

		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), DirectX12Interface::CommandList);
		Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		DirectX12Interface::CommandList->ResourceBarrier(1, &Barrier);
		DirectX12Interface::CommandList->Close();
		DirectX12Interface::CommandQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList* const*>(&DirectX12Interface::CommandList));
		return oPresent12(pSwapChain, SyncInterval, Flags);
	}


	void OSImGui_Internal::CleanDx11()
	{
		if (g_Device.g_pd3dDevice)
			g_Device.g_pd3dDevice->Release();
		if (g_Device.g_pd3dDeviceContext)
			g_Device.g_pd3dDeviceContext->Release();
		if (g_Device.g_mainRenderTargetView)
			g_Device.g_mainRenderTargetView->Release();
	}

	bool OSImGui_Internal::InitDx11(IDXGISwapChain* pSwapChain)
	{
		if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void**>(&g_Device.g_pd3dDevice))))
		{
			g_Device.g_pd3dDevice->GetImmediateContext(&g_Device.g_pd3dDeviceContext);

			DXGI_SWAP_CHAIN_DESC sd;
			pSwapChain->GetDesc(&sd);

			this->Window.hWnd = sd.OutputWindow;

			this->Window.Size = Vec2((float)sd.BufferDesc.Width, (float)sd.BufferDesc.Height);

			ID3D11Texture2D* pBackBuffer;
			pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
			g_Device.g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_Device.g_mainRenderTargetView);

			pBackBuffer->Release();

			return true;
		}
		return false;
	}

	bool OSImGui_Internal::InitDx11Hook()
	{
		if (!this->CreateMyWindow())
			return false;

		HMODULE hModule = NULL;

		void* D3D11CreateDeviceAndSwapChain;
		D3D_FEATURE_LEVEL FeatureLevel;
		D3D_FEATURE_LEVEL FeatureLevels[] = { D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_11_0 };
		DXGI_RATIONAL RefreshRate;
		DXGI_MODE_DESC ModeDesc;
		DXGI_SAMPLE_DESC SampleDesc;
		DXGI_SWAP_CHAIN_DESC SwapChainDesc;
		IDXGISwapChain* SwapChain;
		ID3D11Device* Device;
		ID3D11DeviceContext* Context;

		hModule = GetModuleHandleA("d3d11.dll");

		if (hModule == NULL)
		{
			this->DestroyMyWindow();
			return false;
		}

		D3D11CreateDeviceAndSwapChain = GetProcAddress(hModule, "D3D11CreateDeviceAndSwapChain");

		if (D3D11CreateDeviceAndSwapChain == 0)
		{
			this->DestroyMyWindow();
			return false;
		}
		// Datas init.
		RefreshRate.Numerator = 60;
		RefreshRate.Denominator = 1;
		SampleDesc.Count = 1;
		SampleDesc.Quality = 0;

		ModeDesc.Width = 100;
		ModeDesc.Height = 100;
		ModeDesc.RefreshRate = RefreshRate;
		ModeDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		ModeDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		ModeDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		SwapChainDesc.BufferDesc = ModeDesc;
		SwapChainDesc.SampleDesc = SampleDesc;
		SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		SwapChainDesc.BufferCount = 1;
		SwapChainDesc.OutputWindow = this->Window.hWnd;
		SwapChainDesc.Windowed = 1;
		SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		// Create D3D11 device and swapchain.
		if (((long(__stdcall*)(
			IDXGIAdapter*,
			D3D_DRIVER_TYPE,
			HMODULE,
			UINT,
			const D3D_FEATURE_LEVEL*,
			UINT,
			UINT,
			const DXGI_SWAP_CHAIN_DESC*,
			IDXGISwapChain**,
			ID3D11Device**,
			D3D_FEATURE_LEVEL*,
			ID3D11DeviceContext**))(D3D11CreateDeviceAndSwapChain))(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, FeatureLevels, 1, D3D11_SDK_VERSION, &SwapChainDesc, &SwapChain, &Device, &FeatureLevel, &Context) < 0)
		{
			this->DestroyMyWindow();
			return false;
		}

		Address* Vtable = reinterpret_cast<Address*>(calloc(205, sizeof(Address)));

		memcpy(Vtable, *reinterpret_cast<Address**>(SwapChain), sizeof(Address) * 18);
		memcpy(Vtable + 18, *reinterpret_cast<Address**>(Device), sizeof(Address) * 43);
		memcpy(Vtable + 61, *reinterpret_cast<Address**>(Context), sizeof(Address) * 144);

		SwapChain->Release();
		Device->Release();
		Context->Release();

		this->DestroyMyWindow();

		Address* pTarget = nullptr;

		pTarget = reinterpret_cast<Address*>(Vtable[8]);
		if (MH_CreateHook(pTarget, hkPresent, (void**)(&oPresent)) == MH_OK)
			this->HookData.HookList.push_back(pTarget);

		pTarget = reinterpret_cast<Address*>(Vtable[13]);
		if (MH_CreateHook(pTarget, hkResize, (void**)(&oResize)) == MH_OK)
			this->HookData.HookList.push_back(pTarget);

		free(Vtable);

		if (MH_EnableHook(MH_ALL_HOOKS) == MH_OK)
			return true;

		return false;
	}

	HRESULT __stdcall hkEndScene(LPDIRECT3DDEVICE9 pDevice)
	{
		if (!IsDx9Init)
		{
			if(OSImGui::get().InitDx9(pDevice))
				IsDx9Init = true;
			else
				return oEndScene(pDevice);
		}

		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		OSImGui::get().CallBackFn();

		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

		if (OSImGui::get().EndFlag)
		{
			oEndScene(pDevice);

			OSImGui::get().ReleaseHook();
			OSImGui::get().CleanDx9();

			oWndProc = (WNDPROC)SetWindowLongPtrA(OSImGui::get().Window.hWnd, GWLP_WNDPROC, (LONG_PTR)(oWndProc));

			OSImGui::get().FreeDLL = true;

			return NULL;
		}

		return oEndScene(pDevice);
	}

	HRESULT __stdcall hkReset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPrams)
	{
		if (IsDx9Init)
		{
			OSImGui::get().CleanDx9();
			IsDx9Init = false;
		}

		return oReSet(pDevice, pPrams);
	}

	BOOL CALLBACK EnumWindowsCallback(HWND hWnd, LPARAM lParam)
	{
		DWORD wndProcId;
		GetWindowThreadProcessId(hWnd, &wndProcId);

		if (GetCurrentProcessId() != wndProcId)
			return TRUE;

		OSImGui::get().Window.hWnd = hWnd;
		return FALSE;
	}

	HWND GetProcessWindow()
	{
		OSImGui::get().Window.hWnd = NULL;
		EnumWindows(EnumWindowsCallback, NULL);
		return OSImGui::get().Window.hWnd;
	}

	bool OSImGui_Internal::InitDx9(LPDIRECT3DDEVICE9 pDevice)
	{
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		ImGui::StyleColorsDark();
		io.LogFilename = nullptr;

		if (!ImGui_ImplWin32_Init(Window.hWnd))
			throw OSException("ImGui_ImplWin32_Init() call failed.");
		if (!ImGui_ImplDX9_Init(pDevice))
			throw OSException("ImGui_ImplDX9_Init() call failed.");

		return true;
	}

	void OSImGui_Internal::CleanDx9()
	{
		ImGui_ImplDX9_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	bool OSImGui_Internal::InitDx9Hook()
	{
		if (!this->CreateMyWindow())
			return false;

		HMODULE hModule = NULL;

		void* PDirect3DCreate9;
		LPDIRECT3DDEVICE9 Device;
		IDirect3D9* PDirect3D9;
		D3DDISPLAYMODE DisplayMode;
		D3DPRESENT_PARAMETERS D3dParam;

		// Get d3d module address
		hModule = GetModuleHandleA("d3d9.dll");
		if (hModule == NULL)
		{
			this->DestroyMyWindow();
			return false;
		}

		PDirect3DCreate9 = GetProcAddress(hModule, "Direct3DCreate9");
		if (PDirect3DCreate9 == NULL)
		{
			this->DestroyMyWindow();
			return false;
		}

		PDirect3D9 = ((LPDIRECT3D9(__stdcall*)(DWORD))(PDirect3DCreate9))(D3D_SDK_VERSION);
		if (PDirect3D9 == NULL)
		{
			this->DestroyMyWindow();
			return false;
		}

		if (PDirect3D9->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &DisplayMode) < 0)
		{
			this->DestroyMyWindow();
			return false;
		}

		D3dParam = { 0,0,DisplayMode.Format,0,D3DMULTISAMPLE_NONE,NULL,D3DSWAPEFFECT_DISCARD,this->Window.hWnd,1,0,D3DFMT_UNKNOWN,NULL,0,0 };

		// Create d3d device
		if (PDirect3D9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, this->Window.hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_DISABLE_DRIVER_MANAGEMENT, &D3dParam, &Device) < 0)
		{
			this->DestroyMyWindow();
			PDirect3D9->Release();
			return false;
		}

		Address* Vtable = reinterpret_cast<Address*>(calloc(119, sizeof(Address)));
		memcpy(Vtable, *reinterpret_cast<Address**>(Device), 119 * sizeof(Address));

		PDirect3D9->Release();
		Device->Release();
		this->DestroyMyWindow();

		Address* pTarget = nullptr;

		pTarget = reinterpret_cast<Address*>(Vtable[42]);

		if (MH_CreateHook(pTarget, hkEndScene, (void**)(&oEndScene)) == MH_OK)
			this->HookData.HookList.push_back(pTarget);

		pTarget = reinterpret_cast<Address*>(Vtable[16]);

		if (MH_CreateHook(pTarget, hkReset, (void**)(&oReSet)) == MH_OK)
			this->HookData.HookList.push_back(pTarget);

		free(Vtable);

		if (GetProcessWindow() == NULL)
		{
			if (this->HookData.HookList.size() > 0)
			{
				MH_RemoveHook(this->HookData.HookList.at(0));
			}
			return false;
		}

		if (MH_EnableHook(MH_ALL_HOOKS) == MH_OK)
			return true;

		return false;
	}

	bool OSImGui_Internal::InitDx12Hook()
	{
		if (!this->CreateMyWindow())
			return false;

		

		HMODULE D3D12Module = GetModuleHandle("d3d12.dll");
		HMODULE DXGIModule = GetModuleHandle("dxgi.dll");
		if (D3D12Module == NULL || DXGIModule == NULL) {
			this->DestroyMyWindow();
			throw OSException("d3d12.dll or dxgi.dll not found.");
		}

		void* CreateDXGIFactory = GetProcAddress(DXGIModule, "CreateDXGIFactory");
		if (CreateDXGIFactory == NULL) {
			this->DestroyMyWindow();
			throw OSException("CreateDXGIFactory not found.");
		}

		IDXGIFactory* Factory;
		if (((long(__stdcall*)(const IID&, void**))(CreateDXGIFactory))(__uuidof(IDXGIFactory), (void**)&Factory) < 0) {
			this->DestroyMyWindow();
			throw OSException("Failed to create DXGI Factory.");
		}

		IDXGIAdapter* Adapter;
		if (Factory->EnumAdapters(0, &Adapter) == DXGI_ERROR_NOT_FOUND) {
			this->DestroyMyWindow();
			throw OSException("DXGI Adapter not found.");
		}

		void* D3D12CreateDevice = GetProcAddress(D3D12Module, "D3D12CreateDevice");
		if (D3D12CreateDevice == NULL) {
			this->DestroyMyWindow();
			throw OSException("D3D12CreateDevice not found.");
		}

		ID3D12Device* Device;
		if (((long(__stdcall*)(IUnknown*, D3D_FEATURE_LEVEL, const IID&, void**))(D3D12CreateDevice))(
			Adapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), (void**)&Device) < 0)
		{
			this->DestroyMyWindow();
			throw OSException("Failed to create D3D12 Device.");
		}

		D3D12_COMMAND_QUEUE_DESC QueueDesc{};
		QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		QueueDesc.Priority = 0;
		QueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		QueueDesc.NodeMask = 0;

		ID3D12CommandQueue* CommandQueue;
		if (Device->CreateCommandQueue(&QueueDesc, __uuidof(ID3D12CommandQueue), (void**)&CommandQueue) < 0) {
			this->DestroyMyWindow();
			throw OSException("Failed to create Command Queue.");
		}

		ID3D12CommandAllocator* CommandAllocator;
		if (Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&CommandAllocator) < 0) {
			this->DestroyMyWindow();
			throw OSException("Failed to create Command Allocator.");
		}

		ID3D12GraphicsCommandList* CommandList;
		if (Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, CommandAllocator, NULL, __uuidof(ID3D12GraphicsCommandList), (void**)&CommandList) < 0) {
			this->DestroyMyWindow();
			throw OSException("Failed to create Command List.");
		}

		DXGI_RATIONAL RefreshRate{ 60, 1 };

		DXGI_MODE_DESC BufferDesc{};
		BufferDesc.Width = 100;
		BufferDesc.Height = 100;
		BufferDesc.RefreshRate = RefreshRate;
		BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		DXGI_SAMPLE_DESC SampleDesc{ 1, 0 };

		DXGI_SWAP_CHAIN_DESC SwapChainDesc{};
		SwapChainDesc.BufferDesc = BufferDesc;
		SwapChainDesc.SampleDesc = SampleDesc;
		SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		SwapChainDesc.BufferCount = 2;
		SwapChainDesc.OutputWindow = this->Window.hWnd;
		SwapChainDesc.Windowed = TRUE;
		SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		IDXGISwapChain* SwapChain;
		if (Factory->CreateSwapChain(CommandQueue, &SwapChainDesc, &SwapChain) < 0) {
			this->DestroyMyWindow();
			throw OSException("Failed to create DXGI Swap Chain.");
		}

		MethodsTable = (uintx_t*)::calloc(150, sizeof(uintx_t));
		memcpy(MethodsTable, *(uintx_t**)Device, 44 * sizeof(uintx_t));
		memcpy(MethodsTable + 44, *(uintx_t**)CommandQueue, 19 * sizeof(uintx_t));
		memcpy(MethodsTable + 44 + 19, *(uintx_t**)CommandAllocator, 9 * sizeof(uintx_t));
		memcpy(MethodsTable + 44 + 19 + 9, *(uintx_t**)CommandList, 60 * sizeof(uintx_t));
		memcpy(MethodsTable + 44 + 19 + 9 + 60, *(uintx_t**)SwapChain, 18 * sizeof(uintx_t));

		MH_Initialize();

		Device->Release();
		CommandQueue->Release();
		CommandAllocator->Release();
		CommandList->Release();
		SwapChain->Release();
		this->DestroyMyWindow();
		CreateHook(54, (void**)&oExecuteCommandLists, hkExecuteCommandLists, MethodsTable);
		CreateHook(140, (void**)&oPresent12, hkPresent12, MethodsTable);
		CreateHook(84, (void**)&oDrawInstanced, hkDrawInstanced, MethodsTable);
		CreateHook(85, (void**)&oDrawIndexedInstanced, hkDrawIndexedInstanced, MethodsTable);

		return true;
	}


}

#endif