#include <iostream>
#include <d3d11.h>
#include <DirectXMath.h>

#include <SimShader.h>

using namespace _SimShaderAux;

//=====================================基本渲染环境=====================================

constexpr UINT CLIENT_WIDTH = 640;
constexpr UINT CLIENT_HEIGHT = 480;

HINSTANCE hInstance_ = NULL;
HWND hWnd_ = NULL;

IDXGISwapChain *swapChain_ = nullptr;

ID3D11Device *D3D_ = nullptr;
ID3D11DeviceContext *DC_ = nullptr;

ID3D11Texture2D *depthStencilBuffer_ = nullptr;
ID3D11DepthStencilState *depthStencilState_ = nullptr;
ID3D11DepthStencilView *depthStencilView_ = nullptr;
ID3D11RenderTargetView *renderTargetView_ = nullptr;

bool mainLoopDone_ = false;

bool InitD3DContext(void);
void DestroyD3DContext(void);

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_KEYDOWN:
        if(wParam == VK_ESCAPE)
            mainLoopDone_ = true;
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

bool InitD3DContext(void)
{
    //=================创建渲染窗口=================

    hInstance_ = GetModuleHandle(nullptr);

    WNDCLASSEX wc;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance_;
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = L"TestWindowClass";
    wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
    if(!RegisterClassEx(&wc))
        throw std::runtime_error("Failed to register window class");

    RECT rect = { 0, 0, CLIENT_WIDTH, CLIENT_HEIGHT };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    hWnd_ = CreateWindow(L"TestWindowClass", L"Test Window",
        WS_OVERLAPPED | WS_CAPTION,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        nullptr, nullptr, hInstance_, nullptr);
    if(!hWnd_)
    {
        UnregisterClass(L"TestWindowClass", hInstance_);
        throw std::runtime_error("Failed to create rendering window");
    }

    SetForegroundWindow(hWnd_);
    ShowWindow(hWnd_, SW_SHOW);

    //=================创建D3D渲染环境=================

    HRESULT hr;

    RECT clientRect;
    GetClientRect(hWnd_, &clientRect);
    LONG width = clientRect.right - clientRect.left;
    LONG height = clientRect.bottom - clientRect.top;

    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    swapChainDesc.BufferCount = 2;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.Width = width;
    swapChainDesc.BufferDesc.Height = height;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.Flags = 0;
    swapChainDesc.OutputWindow = hWnd_;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Windowed = TRUE;

    hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        &featureLevel, 1, D3D11_SDK_VERSION, &swapChainDesc,
        &swapChain_, &D3D_, nullptr, &DC_);
    if(FAILED(hr))
        return false;

    //=================取得渲染对象视图=================

    ID3D11Texture2D *backbuffer;
    hr = swapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backbuffer));
    if(FAILED(hr))
        return false;

    hr = D3D_->CreateRenderTargetView(backbuffer, nullptr, &renderTargetView_);
    backbuffer->Release();
    if(FAILED(hr))
        return false;

    //=================准备深度模板缓存=================

    D3D11_TEXTURE2D_DESC depthStencilBufDesc;
    depthStencilBufDesc.ArraySize = 1;
    depthStencilBufDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilBufDesc.CPUAccessFlags = 0;
    depthStencilBufDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilBufDesc.Height = height;
    depthStencilBufDesc.Width = width;
    depthStencilBufDesc.MipLevels = 0;
    depthStencilBufDesc.MiscFlags = 0;
    depthStencilBufDesc.SampleDesc.Count = 1;
    depthStencilBufDesc.SampleDesc.Quality = 0;
    depthStencilBufDesc.Usage = D3D11_USAGE_DEFAULT;

    hr = D3D_->CreateTexture2D(&depthStencilBufDesc, nullptr, &depthStencilBuffer_);
    if(FAILED(hr))
        return false;

    D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc;
    D3D11_DEPTH_STENCILOP_DESC depthStencilOp;
    depthStencilOp.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilOp.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilOp.StencilFunc = D3D11_COMPARISON_ALWAYS;
    depthStencilOp.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
    depthStencilStateDesc.DepthEnable = TRUE;
    depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS;
    depthStencilStateDesc.StencilEnable = FALSE;
    depthStencilStateDesc.StencilReadMask = 0xFF;
    depthStencilStateDesc.StencilWriteMask = 0xFF;
    depthStencilStateDesc.FrontFace = depthStencilOp;
    depthStencilStateDesc.BackFace = depthStencilOp;

    hr = D3D_->CreateDepthStencilState(&depthStencilStateDesc, &depthStencilState_);
    if(FAILED(hr))
        return false;

    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Flags = 0;
    depthStencilViewDesc.Texture2D.MipSlice = 0;
    hr = D3D_->CreateDepthStencilView(depthStencilBuffer_, &depthStencilViewDesc, &depthStencilView_);
    if(FAILED(hr))
        return false;

    DC_->OMSetRenderTargets(1, &renderTargetView_, depthStencilView_);
    DC_->OMSetDepthStencilState(depthStencilState_, 0);

    //=================视口设置=================

    D3D11_VIEWPORT vp;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    vp.Width = static_cast<FLOAT>(width);
    vp.Height = static_cast<FLOAT>(height);
    vp.MaxDepth = 1.0f;
    vp.MinDepth = 0.0f;
    DC_->RSSetViewports(1, &vp);

    return true;
}

void DestroyD3DContext(void)
{
    ReleaseCOMObjects(renderTargetView_, depthStencilView_, depthStencilState_, depthStencilBuffer_);
    ReleaseCOMObjects(D3D_, DC_);
    ReleaseCOMObjects(swapChain_);

    if(hWnd_)
    {
        DestroyWindow(hWnd_);
        UnregisterClass(L"TestWindowClass", hInstance_);
    }
}

//=====================================场景内容=====================================

ID3D11InputLayout *inputLayout_ = nullptr;
ID3D11Buffer *vtxBuf_ = nullptr;

_Shader<SS_VS, SS_PS> shader_;

struct VSCBSize
{
    DirectX::XMFLOAT2 size;
    float pad0;
    float pad1;
};

_ConstantBufferManager<SS_VS> *VSCBs_ = nullptr;

bool InitScene(void)
{
    HRESULT hr;

    //=============着色器编译=============

    ID3D10Blob *shaderByteCode = nullptr;
    ID3D10Blob *shaderErr = nullptr;
    hr = D3DCompileFromFile(
        L"Data\\Test_ConstantBuffer\\test.vs",
        nullptr, nullptr, "main",
        "vs_5_0", 0, 0, &shaderByteCode, &shaderErr);
    if(FAILED(hr))
    {
        if(shaderErr)
        {
            std::cout << shaderErr->GetBufferPointer() << std::endl;
            shaderErr->Release();
        }
        return false;
    }
    else if(shaderErr)
        shaderErr->Release();
    shader_.InitStage<SS_VS>(D3D_, shaderByteCode);
    shaderByteCode->Release();

    shaderErr = nullptr;
    hr = D3DCompileFromFile(
        L"Data\\Test_ConstantBuffer\\test.ps",
        nullptr, nullptr, "main",
        "ps_5_0", 0, 0, &shaderByteCode, &shaderErr);
    if(FAILED(hr))
    {
        if(shaderErr)
        {
            std::cout << shaderErr->GetBufferPointer() << std::endl;
            shaderErr->Release();
        }
        return false;
    }
    else if(shaderErr)
        shaderErr->Release();
    shader_.InitStage<SS_PS>(D3D_, shaderByteCode);
    shaderByteCode->Release();

    VSCBs_ = shader_.GetStage<SS_VS>()->CreateConstantBufferManager();

    //=============顶点缓存初始化=============

    DirectX::XMFLOAT2 vtxBufData[] =
    {
        { -1.0f, -1.0f },
        { 0.0f, 1.0f },
        { 1.0f, -1.0f }
    };
    
    D3D11_BUFFER_DESC vtxBufDesc;
    vtxBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vtxBufDesc.ByteWidth = sizeof(vtxBufData);
    vtxBufDesc.CPUAccessFlags = 0;
    vtxBufDesc.MiscFlags = 0;
    vtxBufDesc.StructureByteStride = sizeof(DirectX::XMFLOAT2);
    vtxBufDesc.Usage = D3D11_USAGE_IMMUTABLE;

    D3D11_SUBRESOURCE_DATA vtxDataDesc = { vtxBufData, 0, 0 };

    hr = D3D_->CreateBuffer(&vtxBufDesc, &vtxDataDesc, &vtxBuf_);
    if(FAILED(hr))
        return false;

    //=============InputLayout=============

    D3D11_INPUT_ELEMENT_DESC inputDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    hr = D3D_->CreateInputLayout(
        inputDesc, 1,
        shader_.GetStage<SS_VS>()->GetShaderByteCode(),
        shader_.GetStage<SS_VS>()->GetShaderByteCodeSize(),
        &inputLayout_);
    if(FAILED(hr))
        return false;

    return true;
}

void DestroyScene(void)
{
    ReleaseCOMObjects(inputLayout_, vtxBuf_);
    shader_.DestroyAllStages();
    SafeDeleteObjects(VSCBs_);
}

void Test_BasicShader(void)
{
    if(!InitD3DContext() || !InitScene())
    {
        DestroyScene();
        DestroyD3DContext();
        std::cout << "Failed to initialize render context" << std::endl;
        return;
    }

    MSG msg;
    mainLoopDone_ = false;
    float t = 0.0f;
    while(!mainLoopDone_)
    {
        float backgroundColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        DC_->ClearRenderTargetView(renderTargetView_, backgroundColor);
        DC_->ClearDepthStencilView(depthStencilView_, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        float sizeComponent = std::sin(t += 0.01f);
        VSCBSize CBSize = { { sizeComponent, sizeComponent }, 0.0f, 0.0f };
        VSCBs_->GetConstantBuffer<VSCBSize, true>(D3D_, "Trans")->SetBufferData(DC_, CBSize);

        DC_->IASetInputLayout(inputLayout_);
        UINT vtxStride = sizeof(DirectX::XMFLOAT2), vtxOffset = 0;
        DC_->IASetVertexBuffers(0, 1, &vtxBuf_, &vtxStride, &vtxOffset);

        shader_.BindStages(DC_);
        VSCBs_->Bind(DC_);

        DC_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        DC_->Draw(3, 0);

        VSCBs_->Unbind(DC_);
        shader_.UnbindStages(DC_);

        swapChain_->Present(1, 0);

        while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    DestroyScene();
    DestroyD3DContext();
}

int main(void)
{
    try
    {
        Test_BasicShader();
    }
    catch(const SimShaderError &err)
    {
        std::cout << err.what() << std::endl;
    }
}
