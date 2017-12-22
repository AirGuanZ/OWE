/*================================================================
Filename: OWEShaderUniforms.h
Date: 2017.12.18
Created by AirGuanZ
================================================================*/
#ifndef __OWESHADER_UNIFORMS_H__
#define __OWESHADER_UNIFORMS_H__

#include <cassert>

#include "OWEShaderStage.h"

namespace _OWEShaderAux
{
    struct _ShaderStageUniformsBinder
    {
        template<typename T>
        void operator()(T &ref)
        {
            ref.Bind(DC);
        }
        
        ID3D11DeviceContext *DC;
    };
    
    struct _ShaderStageUniformsUnbinder
    {
        template<typename T>
        void operator()(T &ref)
        {
            ref.Unbind(DC);
        }
        
        ID3D11DeviceContext *DC;
    };
    
    template<ShaderStageSelector StageSelector>
    class _ShaderStageUniforms
    {
    public:
        _ShaderStageUniforms(const _ShaderStage<StageSelector> &stage)
        {
            CBMgr_ = stage.GetConstantBufferCount() ? stage.CreateConstantBufferManager() : nullptr;
            SRMgr_ = stage.GetShaderResourceCount() ? stage.CreateShaderResourceManager() : nullptr;
            SSMgr_ = stage.GetShaderSamplerCount() ? stage.CreateShaderSamplerManager() : nullptr;
        }

        ~_ShaderStageUniforms(void)
        {
            SafeDeleteObjects(CBMgr_, SRMgr_, SSMgr_);
        }

        _ConstantBufferManager<StageSelector> *GetConstantBufferManager(void)
        {
            return CBMgr_;
        }

        _ShaderResourceManager<StageSelector> *GetShaderResourceManager(void)
        {
            return SRMgr_;
        }

        _ShaderSamplerManager<StageSelector> *GetShaderSamplerManager(void)
        {
            return SSMgr_;
        }
        
        void Bind(ID3D11DeviceContext *DC)
        {
            assert(DC != nullptr);
            if(CBMgr_)
                CBMgr_->Bind(DC);
            if(SRMgr_)
                SRMgr_->Bind(DC);
            if(SSMgr_)
                SSMgr_->Bind(DC);
        }
        
        void Unbind(ID3D11DeviceContext *DC)
        {
            assert(DC != nullptr);
            if(CBMgr_)
                CBMgr_->Unbind(DC);
            if(SRMgr_)
                SRMgr_->Unbind(DC);
            if(SSMgr_)
                SSMgr_->Unbind(DC);
        }

        template<typename BufferType, bool Dynamic>
        _ConstantBufferObject<BufferType, StageSelector, Dynamic>*
        GetConstantBuffer(ID3D11Device *dev, const std::string &name, const BufferType *data)
        {
            assert(CBMgr_ != nullptr);
            return CBMgr_->GetConstantBuffer<BufferType, Dynamic>(dev, name, data);
        }

        _ShaderResourceObject<StageSelector> *GetShaderResource(const std::string &name)
        {
            assert(SRMgr_ != nullptr);
            return SRMgr_->GetShaderResourceObject(name);
        }

        _ShaderSamplerObject<StageSelector> *GetShaderSampler(const std::string &name)
        {
            assert(SSMgr_ != nullptr);
            return SSMgr_->GetShaderSamplerObject(name);
        }

    private:
        _ConstantBufferManager<StageSelector> *CBMgr_;
        _ShaderResourceManager<StageSelector> *SRMgr_;
        _ShaderSamplerManager<StageSelector> *SSMgr_;
    };

    template<ShaderStageSelector...StageSelectors>
    class _ShaderUniformManager
    {
    public:
        _ShaderUniformManager(const _ShaderStage<StageSelectors>&...stages)
            : stageUniforms_(stages...)
        {

        }

        ~_ShaderUniformManager(void)
        {

        }

        template<ShaderStageSelector StageSelector>
        _ShaderStageUniforms<StageSelector> *GetStageUniforms(void)
        {
            return &std::get<FindInNumList<ShaderStageSelector, StageSelector, StageSelectors...>()>(stageUniforms_);
        }

        template<ShaderStageSelector StageSelector>
        _ConstantBufferManager<StageSelector> *GetConstantBufferManager(void)
        {
            return GetStageUniforms<StageSelector>()->GetConstantBufferManager();
        }

        template<ShaderStageSelector StageSelector>
        _ShaderResourceManager<StageSelector> *GetShaderResourceManager(void)
        {
            return GetStageUniforms<StageSelector>()->GetShaderResourceManager();
        }

        template<ShaderStageSelector StageSelector>
        _ShaderSamplerManager<StageSelector> *GetShaderSamplerManager(void)
        {
            return GetStageUniforms<StageSelector>()->GetShaderSamplerManager();
        }

        template<ShaderStageSelector StageSelector, typename BufferType, bool Dynamic = true>
        _ConstantBufferObject<BufferType, StageSelector, Dynamic>*
        GetConstantBuffer(ID3D11Device *dev, const std::string &name, const BufferType *data = nullptr)
        {
            return GetStageUniforms<StageSelector>()->GetConstantBuffer<BufferType, Dynamic>(dev, name, data);
        }

        template<ShaderStageSelector StageSelector>
        _ShaderResourceObject<StageSelector> *GetShaderResource(const std::string &name)
        {
            return GetStageUniforms<StageSelector>()->GetShaderResource(name);
        }

        template<ShaderStageSelector StageSelector>
        _ShaderSamplerObject<StageSelector> *GetShaderSampler(const std::string &name)
        {
            return GetStageUniforms<StageSelector>()->GetShaderSampler(name);
        }
        
        void Bind(ID3D11DeviceContext *DC)
        {
            DoForTupleElements(_ShaderStageUniformsBinder{ DC }, stageUniforms_);
        }
        
        void Unbind(ID3D11DeviceContext *DC)
        {
            DoForTupleElements(_ShaderStageUniformsUnbinder{ DC }, stageUniforms_);
        }

    private:
        std::tuple<_ShaderStageUniforms<StageSelectors>...> stageUniforms_;
    };
}

namespace OWEShader
{
    template<_OWEShaderAux::ShaderStageSelector...StageSelectors>
    using ShaderUniforms = _OWEShaderAux::_ShaderUniformManager<StageSelectors...>;
}

#endif //__OWESHADER_UNIFORMS_H__
