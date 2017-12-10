/*================================================================
Filename: TestApp.h
Date: 2017.12.10
Created by AirGuanZ
================================================================*/
#ifndef __TEST_APP_H__
#define __TEST_APP_H__

#include <d3d11.h>
#include <DirectXMath.h>

#include <SimShader.h>

class TestApp
{
protected:
    HINSTANCE hInstance_ = NULL;
    HWND hWnd_ = NULL;

    IDXGISwapChain *swapChain_ = nullptr;

    ID3D11Device *D3D_ = nullptr;
    ID3D11DeviceContext *DC_ = nullptr;

    ID3D11Texture2D *depthStencilBuffer_ = nullptr;
    ID3D11DepthStencilState *depthStencilState_ = nullptr;
    ID3D11DepthStencilView *depthStencilView_ = nullptr;
    ID3D11RenderTargetView *renderTargetView_ = nullptr;

    bool InitD3DContext(void);

    void DestroyD3DContext(void);
};

#endif //__TEST_APP_H__
