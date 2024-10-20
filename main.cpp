#include "Apps/GravityApp/GravityVecFieldApp.h"

#include <iostream>
#include <stdexcept>
#include <cstdlib>

int main()
{
    try
    {
        Liara::GravityVecFieldApp app;
        app.Run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}