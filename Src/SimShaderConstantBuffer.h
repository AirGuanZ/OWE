/*================================================================
Filename: SimShaderConstantBuffer.h
Date: 2017.11.23
Created by AirGuanZ
================================================================*/
#ifndef __SIMSHADER_CONSTANT_BUFFER_H__
#define __SIMSHADER_CONSTANT_BUFFER_H__

#include "SimShaderObjectBinding.h"

namespace _SimShaderAux
{
    template<typename _ShaderSelector>
    class _ConstantBufferObjectBase
    {
    public:
        _ConstantBufferObjectBase(UINT slot_ = 0, ID3D11Buffer *buf = nullptr)
            : slot_(slot), buf_(buf)
        {

        }

        virtual ~_ConstantBufferObjectBase(void)
        {

        }

        void Bind(ID3D11DeviceContext *devCon)
        {
            assert(devCon);
            _BindConstantBuffer(devCon, slot_, buf_);
        }

        void Unbind(ID3D11DeviceContext *devCon)
        {
            assert(devCon);
            _BindConstantBuffer(devCon, slot_, nullptr);
        }
        
    protected:
        UINT slot_;
        ID3D11Buffer *buf_;
    };

    template<typename _BufferType, ShaderStageSelector _StageSelector, bool _IsDynamic>
    class _ConstantBufferObject : public _ConstantBufferObjectBase<_StageSelector>
    {
    public:
        using MyType = _ConstantBufferObject<_BufferType, _StageSelector, _IsDynamic>;
        using BufferType = _BufferType;

        static constexpr ShaderStageSelector ShaderStage = _StageSelector;
        static constexpr bool IsDynamic = _IsDynamic;

    private:
        _ConstantBufferObject(void);
        ~_ConstantBufferObject(void);
    };
}

#endif //__SIMSHADER_CONSTANT_BUFFER_H__
