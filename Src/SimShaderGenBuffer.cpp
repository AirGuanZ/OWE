/*================================================================
Filename: SimShaderGenBuffer.cpp
Date: 2017.11.23
Created by AirGuanZ
================================================================*/
#include <cassert>
#include "SimShaderGenBuffer.h"

namespace _SimShaderAux
{
    ID3D11Buffer *_GenConstantBuffer(ID3D11Device *dev, size_t byteSize, bool dynamic, const D3D11_SUBRESOURCE_DATA *data)
    {
        assert(dev != nullptr && byteSize > 0);
        assert(dynamic == true || data != nullptr);

        ID3D11Buffer *buf = nullptr;
        D3D11_BUFFER_DESC bufDesc =
        {
            byteSize,                                                //ByteWidth
            dynamic ? D3D11_USAGE_DEFAULT : D3D11_USAGE_IMMUTABLE,   //Usage
            D3D11_BIND_CONSTANT_BUFFER,                              //BindFlags
            dynamic ? D3D11_CPU_ACCESS_WRITE : 0U,                   //CPUAccessFlags
            0U, 0U                                                   //MiscFlags, StructureByteStride
        };
        if(FAILED(dev->CreateBuffer(&bufDesc, data, &buf)))
            return nullptr;
        return buf;
    }
}
