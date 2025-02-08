#include "app/FirstApp/FirstApp.h"

#include <iostream>
#include <cstdlib>

int Run()
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

#if WIN32
    #include "external/SDL2-2.30.8/src/haptic/windows/SDL_dinputhaptic_c.h"
    int WINAPI WinMain(HINSTANCE, HINSTANCE, PSTR, INT) { return Run(); }
#else
    int main() { return Run(); }
#endif
