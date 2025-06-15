#include <SDL.h>
#include "app/FirstApp/FirstApp.h"

#include <iostream>
#include <cstdlib>

int Run()
{
    try
    {
        FirstApp app;
        app.Run();
        return app.GetSettingsManager().SaveToFile("settings.cfg", true) ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}

int main(int argc, char* argv[]) { return Run(); }