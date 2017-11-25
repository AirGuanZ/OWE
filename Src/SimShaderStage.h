/*================================================================
Filename: SimShaderStage.h
Date: 2017.11.25
Created by AirGuanZ
================================================================*/
#ifndef __SIMSHADER_STAGE_H__
#define __SIMSHADER_STAGE_H__

#include <cassert>

#include "SimShaderObjectBinding.h"
#include "SimShaderReflection.h"

#include "SimShaderConstantBuffer.h"
#include "SimShaderResource.h"
#include "SimShaderSampler.h"

namespace _SimShaderAux
{
    template<ShaderStageSelector StageSelector>
    class _ShaderStage;

    template<ShaderStageSelector StageSelector>
    class _ShaderSpec;

    template<>
    class _ShaderSpec<SS_VS>
    {
    public:
        using D3DShaderType = ID3D11VertexShader;

        static D3DShaderType *InitShader(ID3D11Device *dev, void *shaderCompiled, UINT length)
        {
            assert(dev);
            assert(shaderCompiled && length);

            D3DShaderType *rt = nullptr;
            if(FAILED(dev->CreateVertexShader(shaderCompiled, length, nullptr, &rt)))
                throw SimShaderError("Failed to create vertex shader");
            return rt;
        }

        static void BindShader(ID3D11DeviceContext *DC, D3DShaderType *shader)
        {
            assert(DC);
            DC->VSSetShader(shader, nullptr, 0);
        }
    };

    template<>
    class _ShaderSpec<SS_PS>
    {
    public:
        using D3DShaderType = ID3D11PixelShader;

        static D3DShaderType *InitShader(ID3D11Device *dev, void *shaderCompiled, UINT length)
        {
            assert(dev);
            assert(shaderCompiled && length);

            D3DShaderType *rt = nullptr;
            if(FAILED(dev->CreatePixelShader(shaderCompiled, length, nullptr, &rt)))
                throw SimShaderError("Failed to create pixel shader");
            return rt;
        }

        static void BindShader(ID3D11DeviceContext *DC, D3DShaderType *shader)
        {
            assert(DC);
            DC->PSSetShader(shader, nullptr, 0);
        }
    };

    template<ShaderStageSelector StageSelector>
    class _NormalShaderStage
    {
    public:
        using ShaderSpec = _ShaderSpec<StageSelector>;

        _NormalShaderStage(ID3D11Device *dev, void *shaderCompiled, UINT length)
        {
            shader_ = ShaderSpec::InitShader(dev, shaderCompiled, length);
            ID3D11ShaderReflection *ref = _GetShaderReflection(shaderCompiled, length);

            std::map<std::string, _CBInfo> cbInfos;
            _GetConstantBuffers(ref, &cbInfos);
            for(auto it : cbInfos)
                cbMgr_.AddBuffer(it.first, it.second.slot, it.second.byteSize);

            std::map<std::string, UINT> texInfos;
            _GetShaderTextures(ref, &texInfos);
            for(auto it : texInfos)
                srMgr_.AddShaderResource(it.first, it.second);

            std::map<std::string, UINT> samInfos;
            _GetShaderSamplers(ref, &samInfos);
            for(auto it : samInfos)
                samMgr_.AddShaderSampler(it.first, it.second);

            ref->Release();
        }
        
        void BindShader(ID3D11DeviceContext *DC)
        {
            assert(DC);
            ShaderSpec::BindShader(DC, shader_);
        }

        void BindConstantBuffers(ID3D11DeviceContext *DC)
        {
            assert(DC);
            cbMgr_.Bind(DC);
        }

        void BindShaderResources(ID3D11DeviceContext *DC)
        {
            assert(DC);
            srMgr_.Bind(DC);
        }

        void BindShaderSamplers(ID3D11DeviceContext *DC)
        {
            assert(DC);
            samMgr_.Bind(DC);
        }

        void UnbindConstantBuffers(ID3D11DeviceContext *DC)
        {
            assert(DC);
            cbMgr_.Unbind(DC);
        }

        void UnbindShaderResources(ID3D11DeviceContext *DC)
        {
            assert(DC);
            srMgr_.Unbind(DC);
        }

        void UnbindShaderSamplers(ID3D11DeviceContext *DC)
        {
            assert(DC);
            samMgr_.Unbind(DC);
        }

        void BindAllResources(ID3D11DeviceContext *DC)
        {
            BindConstantBuffers(DC);
            BindShaderResources(DC);
            BindShaderSamplers(DC);
        }

        void UnbindAllResources(ID3D11DeviceContext *DC)
        {
            UnbindConstantBuffers(DC);
            UnbindShaderResources(DC);
            UnbindShaderSamplers(DC);
        }
        
        template<typename _BufferType, bool _IsDynamic = true>
        _ConstantBufferObject<_BufferType, StageSelector, _IsDynamic> *
        GetConstantBufferObject(ID3D11Device *dev, const std::string &name, const _BufferType *initData = nullptr)
        {
            return cbMgr_.GetConstantBuffer<_BufferType, _IsDynamic>(dev, name, initData);
        }

        _ShaderResourceObject<StageSelector> *GetShaderResourceObject(const std::string &name)
        {
            return srMgr_.GetShaderResourceObject(name);
        }

        _ShaderSamplerObject<StageSelector> *GetShaderSamplerObject(const std::string &name)
        {
            return samMgr_.GetShaderSamplerObject(name);
        }

    private:
        _ConstantBufferManager<StageSelector> cbMgr_;
        _ShaderResourceManager<StageSelector> srMgr_;
        _ShaderSamplerManager<StageSelector> samMgr_;

        typename ShaderSpec::D3DShaderType *shader_;
    };
}

#endif //__SIMSHADER_STAGE_H__
