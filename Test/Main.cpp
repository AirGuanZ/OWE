#include <iostream>
#include "../Src/SimShaderConstantBuffer.h"

using namespace std;

int main(void)
{
    using namespace _SimShaderAux;
    _ConstantBufferManager<SS_VS> VSCBMgr;
    try
    {
        auto cb = VSCBMgr.GetConstantBuffer<int, true>((ID3D11Device*)1, "hahaha");
    }
    catch(const SimShaderError &err)
    {
        cout << err.what() << endl;
    }
}
