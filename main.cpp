#include "app/FirstApp/FirstApp.h"

#include <iostream>
#include <cstdlib>

#include "Core/Liara_Settings.h"

int Run()
{
    try
    {
        Liara::Liara_Settings& settings = Liara::Singleton<Liara::Liara_Settings>::GetInstance();
        settings.LoadFromFile("settings.cfg");

        FirstApp app;
        app.Run();

        settings.SaveToFile("settings.cfg", true, true);
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

#if WIN32
    #include <Windows.h>
    int WINAPI WinMain(HINSTANCE, HINSTANCE, PSTR, INT) { return Run(); }
#else
    int main() { return Run(); }
#endif
