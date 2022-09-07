#include"Canvas3D.h"

void Canvas3D::DrawFunction(Window& wnd)
{
	auto [x, y] = wnd.mouse.GetXY();
	auto [flt_x, flt_y] = GetNormalizedWindowPos(x, y);

	VERTEX_BUFFER.emplace_back(flt_x , flt_y , 0.0f , 255 , 255 , 255);

	//vertex buffer description
	D3D11_BUFFER_DESC bd = { 0 };
	bd.ByteWidth = sizeof(VertexType) * VERTEX_BUFFER.size();					//total array size
	bd.Usage = D3D11_USAGE_DEFAULT;				// how buffer data will be used (read/write protections for GPU/CPU)
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;	// What type of buffer would it be
	bd.CPUAccessFlags = 0u;						// we don't want any cpu access for now so setting it to 0 for now
	bd.MiscFlags = 0u;							// misscellinious flag for buffer configuration (we don't want it now either)
	bd.StructureByteStride = sizeof(VertexType); // Size of every vertex in the array 

	//holds the data pointer that will be used in vertex buffer
	D3D11_SUBRESOURCE_DATA subd = { 0 };
	subd.pSysMem = VERTEX_BUFFER.data(); // pointer to array so that it can copy all the array data to the buffer

	Microsoft::WRL::ComPtr<ID3D11Buffer> VBuffer;
	Device->CreateBuffer(&bd, &subd, &VBuffer);
	UINT stride = sizeof(VertexType); // size of every vertex
	UINT offset = 0u; // displacement after which the actual data start (so 0 because no displacement is there)
	//statrting slot(from 0) , number of buffers(1 buffer) , pp , 
	ImmediateContext->IASetVertexBuffers(0u, 1u, VBuffer.GetAddressOf(), &stride, &offset);

}

Canvas3D::Canvas3D(Window& wnd) : Halfheight(wnd.height / 2), Halfwidth(wnd.width / 2)
{
	wnd.mouse.OnLeftPress = [this](Window& wnd) {
	
		DrawFunction(wnd);
	
	};

	PtrManager<ID3D11Resource> pBackBuffer;
	DXGI_SWAP_CHAIN_DESC sd = { 0 };
	sd.BufferDesc.Width = 0;  // look at the window and use it's size
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.RefreshRate.Numerator = 0; // pick the default refresh rates
	sd.BufferDesc.RefreshRate.Denominator = 0;
	sd.BufferCount = 1;  // one back buffer -> one back and one front double buffering
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // this is the color format (BGRA) 
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED; // not specifying any scaling because we want the renedred frame same as window size
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED; // how buffer scaning will be done for copying all to video memory
	sd.Flags = 0; // not setting any flags
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // use the buffer for render target
	sd.OutputWindow = wnd.window_handle;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; // discard the effects for swapping frames
	sd.Windowed = TRUE;
	// for antialiasing we don't want it right now
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;

	if (auto hrcode = D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &sd, &SwapChain, &Device, nullptr, &ImmediateContext); FAILED(hrcode))
	{
		throw hrcode;
	}

	SwapChain->GetBuffer(0, __uuidof(ID3D11Resource), &pBackBuffer);
	Device->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &RenderTarget);

	Microsoft::WRL::ComPtr<ID3D11VertexShader> vS; // shader pointer
	Microsoft::WRL::ComPtr<ID3DBlob> blb; // holds the compiled shader bytecode
	D3DReadFileToBlob(L"VertexShader.cso", &blb); // reading the bytecode and storing it to blob
	Device->CreateVertexShader(blb->GetBufferPointer(), blb->GetBufferSize(), nullptr, &vS); // creating vertex shader
	ImmediateContext->VSSetShader(vS.Get(), nullptr, 0u); // binding vertex shader

	Microsoft::WRL::ComPtr<ID3D11InputLayout> inpl;
	//semantic name , semantic index , format , inputslot , offset , input data class , data step rate

	D3D11_INPUT_ELEMENT_DESC ied[] = {

		//tells that the first 3 * 4 * 8 bits = 32 * 3 = 96 bits of the vertex struct are for positions for every vertex
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
		//and next 3 * 1 * 8 = 24 bits are color value for each of those vertex
		// unorm for automaticall convert 0-255 range 0.0-1.0 range
		{"COLOR",0,DXGI_FORMAT_R8G8B8A8_UNORM,0,12u,D3D11_INPUT_PER_VERTEX_DATA,0}
	};
	//creating and setting
	Device->CreateInputLayout(ied, (UINT)std::size(ied), blb->GetBufferPointer(), blb->GetBufferSize(), &inpl);
	ImmediateContext->IASetInputLayout(inpl.Get());

	Microsoft::WRL::ComPtr<ID3D11PixelShader> ps; // shader pointer
	D3DReadFileToBlob(L"PixelShader.cso", &blb); // reading file to blob
	Device->CreatePixelShader(blb->GetBufferPointer(), blb->GetBufferSize(), nullptr, &ps); // creating
	ImmediateContext->PSSetShader(ps.Get(), nullptr, 0u); // setting

	ImmediateContext->OMSetRenderTargets(1u, RenderTarget.GetAddressOf(), nullptr);

	D3D11_VIEWPORT vp = {};
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = 800;  //screen height
	vp.Height = 600; // screen width
	vp.MaxDepth = 1; // maximum depth for z axis
	vp.MinDepth = 0; // minimum depth for z axis
	ImmediateContext->RSSetViewports(1u, &vp);

	//draws the vertices as a list of traingles 
	ImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

}

void Canvas3D::ClearCanvas() const
{
	float color[] = { 0.0f , 0.0f , 0.0f , 0.0f };
	ImmediateContext->ClearRenderTargetView(RenderTarget.Get(), color);
	ImmediateContext->ClearDepthStencilView(DepthView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);
}

void Canvas3D::PresentOnScreen() const
{
	ImmediateContext->Draw(VERTEX_BUFFER.size(), 0);
	SwapChain->Present(1u, 0u);
}

std::pair<float, float> Canvas3D::GetNormalizedWindowPos(int x, int y) const
{
	return { (x / Halfwidth) - 1.0f,  -((y / Halfheight) - 1.0f) };
}
