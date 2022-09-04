#pragma once
#include<d3d11.h>
#include<wrl.h>
#pragma comment(lib,"d3d11.lib")
#include<DirectXMath.h>

#include"Window.h"

class CanvasAccesser;

class Canvas3D
{
	template<typename t>
	using PtrManager = Microsoft::WRL::ComPtr<t>;
private:
	PtrManager<ID3D11Device> Device;
	PtrManager<IDXGISwapChain> SwapChain;
	PtrManager<ID3D11DeviceContext> ImmediateContext;
	PtrManager<ID3D11RenderTargetView> RenderTarget;
	PtrManager<ID3D11DepthStencilView> DepthView;
private:
	const float Halfheight;
	const float Halfwidth;
public:
	Canvas3D(const Window& wnd);
public:
	std::pair<float, float> GetNormalizedWindowPos(int x, int y) const;
	void ClearCanvas() const;
	void PresentOnScreen() const;
};