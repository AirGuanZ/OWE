/*================================================================
Filename: SimShaderGenBuffer.h
Date: 2017.11.23
Created by AirGuanZ
================================================================*/
#ifndef __SIMSHADER_GEN_BUFFER_H__
#define __SIMSHADER_GEN_BUFFER_H__

#include <d3d11.h>

namespace _SimShaderAux
{
    ID3D11Buffer *_GenConstantBuffer(size_t byteSize, bool dynamic, const D3D11_SUBRESOURCE_DATA *data);
}

#endif //__SIMSHADER_GEN_BUFFER_H__
