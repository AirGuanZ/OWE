/*================================================================
Filename: SimShaderReflection.h
Date: 2017.11.25
Created by AirGuanZ
================================================================*/
#ifndef __SIMSHADER_REFLECTION_H__
#define __SIMSHADER_REFLECTION_H__

#include <cassert>
#include <cstring>
#include <string>

#include <d3d11.h>
#include <d3dcompiler.h>

#include "SimShaderConstantBuffer.h"

namespace _SimShaderAux
{
    ID3D11ShaderReflection *_GetShaderReflection(void *shaderData, SIZE_T length)
    {
        assert(ata);
        ID3D11ShaderReflection *rt = nullptr;
        if(FAILED(D3DReflect(shaderData, length, IID_ID3D11ShaderReflection, reinterpret_cast<void**>(&rt))))
            return nullptr;
        return rt;
    }

    struct _CBInfo
    {
        UINT slot;
        UINT byteSize;
    };

    void _GetConstantBuffers(ID3D11ShaderReflection *ref, std::map<std::string, _CBInfo> *rt)
    {
        assert(ref && rt);
        rt->clear();

        D3D11_SHADER_DESC shaderDesc;
        ref->GetDesc(&shaderDesc);
        for(int cbIdx = 0; cbIdx != shaderDesc.ConstantBuffers; ++cbIdx)
        {
            int regIdx = -1; //°ó¶¨µÄ¼Ä´æÆ÷±àºÅ
            ID3D11ShaderReflectionConstantBuffer *cbuf = ref->GetConstantBufferByIndex(cbIdx);
            D3D11_SHADER_BUFFER_DESC bufDesc;
            cbuf->GetDesc(&bufDesc);
            for(int bpIdx = 0; bpIdx != shaderDesc.BoundResources; ++bpIdx)
            {
                D3D11_SHADER_INPUT_BIND_DESC bpDesc;
                ref->GetResourceBindingDesc(bpIdx, &bpDesc);
                if(std::strcmp(bpDesc.Name, bufDesc.Name) == 0)
                {
                    regIdx = bpDesc.BindPoint;
                    break;
                }
            }

            assert(regIdx != -1);
            rt->insert(std::make_pair(std::string(bufDesc.Name), _CBInfo{ regIdx, bufDesc.Size }));

        }
    }

    void _GetShaderTextures(ID3D11ShaderReflection *ref, std::map<std::string, UINT> *rt)
    {
        assert(ref && rt);
        rt->clear();

        D3D11_SHADER_DESC shaderDesc;
        ref->GetDesc(&shaderDesc);
        for(int rscIdx = 0; rscIdx != shaderDesc.BoundResources; ++rscIdx)
        {
            D3D11_SHADER_INPUT_BIND_DESC bdDesc;
            ref->GetResourceBindingDesc(rscIdx, &bdDesc);
            if(bdDesc.Type == D3D_SIT_TEXTURE)
                rt->insert(std::make_pair(std::string(bdDesc.Name), bdDesc.BindPoint));
        }
    }

    void _GetShaderSamplers(ID3D11ShaderReflection *ref, std::map<std::string, UINT> *rt)
    {
        assert(ref && rt);
        rt->clear();

        D3D11_SHADER_DESC shaderDesc;
        ref->GetDesc(&shaderDesc);
        for(int rscIdx = 0; rscIdx != shaderDesc.BoundResources; ++rscIdx)
        {
            D3D11_SHADER_INPUT_BIND_DESC bdDesc;
            ref->GetResourceBindingDesc(rscIdx, &bdDesc);
            if(bdDesc.Type == D3D_SIT_SAMPLER)
                rt->insert(std::make_pair(std::string(bdDesc.Name), bdDesc.BindPoint));
        }
    }
}

#endif //__SIMSHADER_REFLECTION_H__
