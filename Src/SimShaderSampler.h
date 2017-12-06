/*================================================================
Filename: SimShaderSampler.h
Date: 2017.11.25
Created by AirGuanZ
================================================================*/
#ifndef __SIMSHADER_SAMPLER_H__
#define __SIMSHADER_SAMPLER_H__

#include <map>

#include "SimShaderFatalError.h"
#include "SimShaderObjectBinding.h"
#include "SimShaderReleaseCOMObjects.h"
#include "SimShaderUncopiable.h"

namespace _SimShaderAux
{
    template<ShaderStageSelector StageSelector>
    class _ShaderSamplerObject;

    template<ShaderStageSelector StageSelector>
    class _ShaderStage;

    template<ShaderStageSelector StageSelector>
    class _ShaderSamplerManager;

    template<ShaderStageSelector StageSelector>
    class _ShaderSamplerObject : public _Uncopiable
    {
    public:
        void SetSampler(ID3D11SamplerState *sampler)
        {
            ReleaseCOMObjects(sampler_);
            sampler_ = sampler;
            if(sampler_)
                sampler_->AddRef();
        }

        void Bind(ID3D11DeviceContext *DC)
        {
            assert(DC != nullptr);
            _BindShaderSampler(DC, slot_, sampler_);
        }

        void Unbind(ID3D11DeviceContext *DC)
        {
            assert(DC != nullptr);
            _BindShaderSampler(DC, slot_, nullptr);
        }

    private:
        _ShaderSamplerObject(UINT slot, ID3D11SamplerState *sampler = nullptr)
            : slot_(slot), sampler_(sampler)
        {
            if(sampler)
                sampler->AddRef();
        }

        ~_ShaderSamplerObject(void)
        {
            ReleaseCOMObjects(sampler_);
        }

    private:
        UINT slot_;
        ID3D11SamplerState *sampler_;
    };

    template<ShaderStageSelector StageSelector>
    class _ShaderSamplerManager : public _Uncopiable
    {
    public:
        using SSObj = _ShaderSamplerObject<StageSelector>;

        _ShaderSamplerManager(void) = default;

        ~_ShaderSamplerManager(void)
        {
            for(auto it : SSs_)
            {
                if(it.second.obj)
                    delete it.second.obj;
            }
        }

        void AddShaderSampler(const std::string &name, UINT slot, ID3D11SamplerState *initSampler = nullptr)
        {
            auto it = SSs_.find(name);
            if(it != SSs_.end())
                throw SimShaderError("Shader sampler name repeated: " + name);
            SSs_[name] = _SSRec{ slot, new SSobj(slot, initSampler) };
        }

        SSObj *GetShaderSamplerObject(const std::string &name)
        {
            auto it = SSs_.find(name);
            if(it == SSs_.end())
                throw SimShaderError("Shader sampler not found: " + name);

            assert(it.second.obj != nullptr);
            return it.second.obj;
        }

        void Bind(ID3D11DeviceContext *DC)
        {
            assert(DC != nullptr);
            for(auto it : SSs_)
            {
                assert(it.second.obj);
                it.second.obj->Bind(DC);
            }
        }

        void Unbind(ID3D11DeviceContext *DC)
        {
            assert(DC != nullptr);
            for(auto it : SSs_)
            {
                assert(it.second.obj);
                it.second.obj->Unbind(DC);
            }
        }

    private:
        friend class _ShaderStage<StageSelector>;
        struct _SSRec
        {
            UINT slot;
            SSObj *obj;
        };

    private:
        std::map<std::string, _SSRec> SSs_;
    };
}

#endif //__SIMSHADER_SAMPLER_H__
