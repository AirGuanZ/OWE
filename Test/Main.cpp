/*================================================================
Filename: Main.cpp
Date: 2017.12.10
Created by AirGuanZ
================================================================*/
#include "Test_BasicShader.h"

int main(void)
{
    try
    {
        Test_BasicShader app;
        app.Run();
    }
    catch(const SimError &err)
    {
        std::cout << err.what() << std::endl;
    }
}
