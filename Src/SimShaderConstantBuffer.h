/*================================================================
Filename: SimShaderConstantBuffer.h
Date: 2017.11.23
Created by AirGuanZ
================================================================*/
#ifndef __SIMSHADER_CONSTANT_BUFFER_H__
#define __SIMSHADER_CONSTANT_BUFFER_H__

#include <map>
#include "SimShaderObjectBinding.h"

namespace _SimShaderAux
{
    template<ShaderStageSelector _ShaderSelector>
    class _ConstantBufferObjectBase;

    template<typename _BufferType, ShaderStageSelector _StageSelector, bool _IsDynamic>
    class _ConstantBufferAttributes;

    template<typename _BufferType, ShaderStageSelector _StageSelector, bool _IsDynamic>
    class _ConstantBufferObject;

    template<ShaderStageSelector _ShaderSelector>
    class _ConstantBufferObjectBase
    {
    public:
        _ConstantBufferObjectBase(UINT slot_, ID3D11Buffer *buf)
            : slot_(slot), buf_(buf)
        {

        }

        virtual ~_ConstantBufferObjectBase(void)
        {

        }

        void Bind(ID3D11DeviceContext *devCon) const
        {
            assert(devCon);
            _BindConstantBuffer<_ShaderSelector>(devCon, slot_, buf_);
        }

        void Unbind(ID3D11DeviceContext *devCon) const
        {
            assert(devCon);
            _BindConstantBuffer<_ShaderSelector>(devCon, slot_, nullptr);
        }
        
    protected:
        UINT slot_;
        ID3D11Buffer *buf_;
    };

    template<typename _BufferType, ShaderStageSelector _StageSelector, bool _IsDynamic>
    class _ConstantBufferAttributes
    {
    public:
        using MyType = _ConstantBufferObject<_BufferType, _StageSelector, _IsDynamic>;
        using BufferType = _BufferType;

        static constexpr ShaderStageSelector ShaderStage = _StageSelector;
        static constexpr bool IsDynamic = _IsDynamic;
    };

    //Static constant buffer object
    template<typename _BufferType, ShaderStageSelector _StageSelector>
    class _ConstantBufferObject<_BufferType, _StageSelector, false>
        : public _ConstantBufferAttributes<_BufferType, _StageSelector, false>,
          public _ConstantBufferObjectBase<_StageSelector>
    {
    private:
        _ConstantBufferObject(UINT slot, ID3D11Buffer *buf)
            : _ConstantBufferObjectBase(slot, buf)
        {

        }

        ~_ConstantBufferObject(void)
        {

        }
    };

    //Dynamic constant buffer object
    template<typename _BufferType, ShaderStageSelector _StageSelector>
    class _ConstantBufferObject<_BufferType, _StageSelector, true>
        : public _ConstantBufferAttributes<_BufferType, _StageSelector, true>,
          public _ConstantBufferObjectBase<_StageSelector>
    {
    public:
        void SetBufferData(ID3D11DeviceContext *devCon, const BufferType &data)
        {
            assert(devCon != nullptr && buf_);
            devCon->UpdateSubresource(buf_, 0, nullptr, &data, 0, 0);
        }

    private:
        _ConstantBufferObject(UINT slot, ID3D11Buffer *buf)
            : _ConstantBufferObjectBase(slot, buf)
        {

        }

        ~_ConstantBufferObject(void)
        {

        }
    };

    template<ShaderStageSelector StageSelector>
    class SimShaderConstantBufferMgr
    {
    public:

    private:

    };
}

#endif //__SIMSHADER_CONSTANT_BUFFER_H__
