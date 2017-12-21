/*================================================================
Filename: Main.cpp
Date: 2017.12.10
Created by AirGuanZ
================================================================*/
#include "Test_BasicShader.h"
#include "Test_HeightMap.h"
#include "Test_MixTex.h"
#include "Test_TextureCube.h"

int main(void)
{
    try
    {
        Test_HeightMap::App app;
        app.Run();
    }
    catch(const SimShader::Error &err)
    {
        std::cout << err.what() << std::endl;
    }
}
