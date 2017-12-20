# SimShader

在编写简单的DirectX渲染程序的过程中，MS官方提供的Effect框架用起来常常有许多冗余，基于底层API管理着色器又太不方便，所以做了这么一个着色器封装。

---

## 基本使用

完整的可编程渲染流程被封装在`SimShader::Shader`中，使用时用模板参数指定要包含的可编程渲染阶段，比如`SimShader::Shader<SS_VS, SS_PS>`表示使用Vertex Shader（`SS_VS`）和Pixel Shader（`SS_PS`）。未来将会添加更多可使用的渲染阶段，如`SS_GS`（Gemotry Shader）等。

着色器的初始化通过调用`SimShader::Shader<...>::InitStage<ShaderStage>`完成，其中`ShaderStage`值为`SS_VS`或`SS_PS`，表示要初始化的是哪一个阶段。InitStage的参数可以是编译好的`ID3D10Blob*`类型，也可以是着色器源代码。

在开始渲染之前，需要绑定相应地着色器，此时调用`SimShader::Shader<...>::BindStages`即可；渲染完成后，可通过`SimShader::Shader<...>::UnbindStages`解除绑定。

要销毁着色器，调用`SimShader::Shader<...>::DestroyAllStages`即可。

一个仅使用Vertex Shader和Pixel Shader的例子如下：

```cpp
//仅需包含Inc\SimShader.hpp
#include <SimShader.hpp>

//所有类均定义在命名空间SimShader中
using namespace SimShader;

//着色器定义
//SS_VS是ShaderStage_VertexShader的缩写，SS_PS同理
Shader<SS_VS, SS_PS> shader;

//着色器初始化
bool InitShader(void)
{
    try
    {
        shader.InitStage<SS_VS>(myD3DDevice, vertexShaderSourceCode);
        shader.InitStage<SS_PS>(myD3DDevice, pixelShaderSourceCode);
    }
    catch(const Error &err)
    {
        //err.what()中包含了着色器初始化失败的错误信息
        return false;
    }
    return true;
}

//取得Vertex Shader的Input Signature
bool InitInputLayout(void)
{
    //...其他准备工作
    
    HRESULT hr = myD3DDevice->CreateInputLayout(
                    inputLayoutDesc, arraySize(inputLayoutDesc),
                    shader.GetShaderByteCodeWithInputSignature(),
                    shader.GetShaderByteCodeSizeWithInputSignature(),
                    &inputLayout);
    
    //...后续工作
}

//渲染前将Shader绑定到渲染管线
void RenderObject(void)
{
    shader.Bind(myD3DDeviceContext);
    
    //...draw calls
    
    shader.Unbind(myD3DDeviceContext);
}

//销毁着色器
void DestroyShader(void)
{
    shader.Destroy();
}
```

## 管理着色器中的资源

我们常常需要通过Constant Buffer、Shader Resource、Sampler State等途径与着色器进行交互，SimShader提供了直截了当的管理这些资源的方法。到目前为止，已经完成了对Constant Buffer、Texture2D、SamplerState进行管理的封装，对UAV等类型的资源管理将会在以后扩充完成。

Constant Buffer的管理通过`SimShader::ConstantBufferManager<ShaderStage>`完成。一个基本的使用示例如下：

```cpp
/*假定着色器中有一段这样的代码：
  cbuffer Color
  {
      float3 color;
  };
  这一结构在C++中对应下面的结构体：
  struct CBColor
  {
      Vector3 color;
      float pad;
  };
  现需要用该着色器渲染两组物体
  第一组设置为红色，第二组设置为蓝色
*/
Shader<SS_VS, SS_PS> shader;

ConstantBufferManager<SS_VS> *cbMgr1 = nullptr;
ConstantBufferManager<SS_PS> *cnMgr2 = nullptr;

void InitConstantBufferManagers(void)
{
    //假定shader已经初始化完成
    cbMgr1 = shader.CreateConstantBufferManager<SS_VS>();
    cbMgr2 = shader.CreateConstantBufferManager<SS_PS>();
    
    //C++中表示红色和蓝色的结构体实例
    CBColor cbColorRed = { { 1.0f, 0.0f, 0.0f }, 0.0f };
    CBColor cbColorBlue = { { 0.0f, 0.0f, 1.0f }, 0.0f };
    
    cbMgr1->GetConstantBuffer<CBColor>(myD3DDevice, "Color")->SetBufferData(myD3DDeviceContext, cbColorRed);
    cbMgr2->GetConstantBuffer<CBColor>(myD3DDevice, "Color")->SetBufferData(myD3DDeviceContext, cbColorBlue);
}

void Render(void)
{
    shader.Bind(myD3DDeviceContext);
  
    //渲染第一组物体
    
    cbMgr1->Bind(myD3DDeviceContext);
    //...draw calls
    cbMgr1->Unbind(myD3DDeviceContext);
    
    //渲染第二组物体
    
    cbMgr2->Bind(myD3DDeviceContext);
    //...draw calls
    cbMgr2->Unbind(myD3DDeviceContext);
  
    shader.Unbind(myD3DDeviceContext);
}

void DestroyConstantBufferManagers(void)
{
    if(cbMgr1)
    {
        delete cbMgr1;
        cbMgr1 = nullptr;
    }
    
    if(cbMgr2)
    {
        delete cbMgr2;
        cbMgr2 = nullptr;
    }
}
```

在上面的代码中，两个不同的ConstantBufferManager实例对应两组不同的物体。采用另一种方案——动态地修改某个ConstantBufferManager中的数据也是可行的：

```cpp
void Render(void)
{
    shader.Bind(myD3DDeviceContext);
  
    //渲染第一组物体
  
    CBColor cbColorRed = { { 1.0f, 0.0f, 0.0f }, 0.0f };
    cbMgr->GetConstantBuffer<CBColor>(myD3DDevice, "Color")->SetBufferData(myD3DDeviceContext, cbColorRed);
    cbMgr->Bind(myD3DDeviceContext);
    //...draw calls
    cbMgr->Unbind(myD3DDeviceContext);
    
    //渲染第二组物体
    
    CBColor cbColorBlue = { { 0.0f, 0.0f, 1.0f }, 0.0f };
    cbMgr->GetConstantBuffer<CBColor>(myD3DDevice, "Color")->SetBufferData(myD3DDeviceContext, cbColorBlue);
    cbMgr->Bind(myD3DDeviceContext);
    //...draw calls
    cbMgr->Unbind(myD3DDeviceContext);
  
    shader.Unbind(myD3DDeviceContext);
}
```

对Texture2D和Sampler State的管理与Constant Buffer相似，分别由`ShaderResourceManager<ShaderStage>`和`ShaderSamplerManager<ShaderStage>`完成。