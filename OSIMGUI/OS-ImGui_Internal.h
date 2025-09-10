#pragma once
#include "OS-ImGui_Base.h"
#include "Minhook/MinHook.h"
#include <thread>

/****************************************************
* Copyright (C)	: Liv
* @file			: OS-ImGui_Internal.h
* @author		: Liv
* @email		: 1319923129@qq.com
* @version		: 1.1
* @date			: 2024/4/5 13:00
****************************************************/

#if defined _M_X64
typedef uint64_t uintx_t;
#elif defined _M_IX86
typedef uint32_t uintx_t;
#endif

//Dx12
typedef HRESULT(APIENTRY* Present12) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef void(APIENTRY* DrawInstanced)(ID3D12GraphicsCommandList* dCommandList, UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation);
typedef void(APIENTRY* DrawIndexedInstanced)(ID3D12GraphicsCommandList* dCommandList, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation);
typedef void(APIENTRY* ExecuteCommandLists)(ID3D12CommandQueue* queue, UINT NumCommandLists, ID3D12CommandList* ppCommandLists);





namespace OSImGui
{
#ifdef OSIMGUI_INTERNAL

	class OSImGui_Internal : public OSImGui_Base
	{
	public:
		uintx_t* MethodsTable=nullptr;
		WindowType Type = INTERNAL;
		DirectXType DxType = DirectXType::AUTO;
		bool FreeDLL = false;
		HMODULE hLibModule = NULL;
		struct HookData_
		{
			std::vector<Address*> HookList;
		}HookData;
	public:
		void Start(HMODULE hLibModule ,std::function<void()> CallBack, DirectXType DxType = DirectXType::AUTO);
		void ReleaseHook();
	public:
		// DirectX 11
		bool InitDx11(IDXGISwapChain* pSwapChain);
		void CleanDx11();
		bool InitDx11Hook();
		// DirectX 9
		bool InitDx9(LPDIRECT3DDEVICE9 pDevice);
		void CleanDx9();
		bool InitDx9Hook();
		//DirectX12
		bool InitDx12Hook();
	private:
		void InitThread();
		bool CreateMyWindow();
		void DestroyMyWindow();
	};

#endif 
}