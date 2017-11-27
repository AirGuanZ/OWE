/*================================================================
Filename: SimShaderStage.h
Date: 2017.11.26
Created by AirGuanZ
================================================================*/
#ifndef __SIMSHADER_STAGE_H__
#define __SIMSHADER_STAGE_H__

#include <cassert>
#include <d3d11.h>

#include "SimShaderObjectBinding.h"
#include "SimShaderReflection.h"
#include "SimShaderUncopiable.h"

#include "SimShaderConstantBuffer.h"
#include "SimShaderSampler.h"
#include "SimShaderResource.h"

namespace _SimShaderAux
{
    template<ShaderStageSelector>
    class _ShaderStageSpec;

    template<>
    class _ShaderStageSpec<SS_VS>
    {
    public:
        using D3DShaderType = ID3D11VertexShader;

        static D3DShaderType *InitShader(ID3D11Device *dev, void *shaderCompiled, SIZE_T length)
        {
            assert(dev != nullptr);
            assert(shaderCompiled != nullptr && length > 0);
            D3DShaderType *rt = nullptr;
            if(FAILED(dev->CreateVertexShader(shaderCompiled, length, nullptr, &rt)))
                return nullptr;
            return rt;
        }

        static void BindShader(ID3D11DeviceContext *DC, D3DShaderType *shader)
        {
            assert(DC != nullptr);
            DC->VSSetShader(shader, nullptr, 0);
        }
    };

    template<>
    class _ShaderStageSpec<SS_PS>
    {
    public:
        using D3DShaderType = ID3D11PixelShader;

        static D3DShaderType *InitShader(ID3D11Device *dev, void *shaderCompiled, SIZE_T length)
        {
            assert(dev != nullptr);
            assert(shaderCompiled != nullptr && length > 0);
            D3DShaderType *rt = nullptr;
            if(FAILED(dev->CreatePixelShader(shaderCompiled, length, nullptr, &rt)))
                return nullptr;
            return rt;
        }

        static void BindShader(ID3D11DeviceContext *DC, D3DShaderType *shader)
        {
            assert(DC != nullptr);
            DC->PSSetShader(shader, nullptr, 0);
        }
    }; 

    template<ShaderStageSelector StageSelector>
    class _ShaderStage : public _Uncopiable
    {
    public:
        using StageSpec = _ShaderStageSpec<StageSelector>;
        using D3DShaderType = typename StageSpec::D3DShaderType;

        _ShaderStage(ID3D11Device *dev, void shaderByteCode, SIZE_T length);
        ~_ShaderStage(void);

        void BindShader(ID3D11DeviceContext *DC);
        void UnbindShader(ID3D11DeviceContext *DC);

        _ConstantBufferManager<StageSelector> *CreateConstantBufferManager(void);
        _ShaderResourceManager<StageSelector> *CreateShaderResourceManager(void);
        _ShaderSamplerManager<StageSelector> *CreateShaderSamplerManager(void);

    private:
        D3DShaderType *shader_;
    };
}

#endif //__SIMSHADER_STAGE_H__
