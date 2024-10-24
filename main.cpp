#include "app/FirstApp/FirstApp.h"

#include <iostream>
#include <stdexcept>
#include <cstdlib>

int main()
{
    try
    {
        FirstApp app;
        app.Run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}