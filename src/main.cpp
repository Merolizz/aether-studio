#include "core/Application.h"
#include <iostream>
#include <exception>
#include <limits>

#ifdef _WIN32
static void pauseOnError() {
    std::cout << "\nPress Enter to exit..." << std::endl;
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}
#endif

int main() {
    try {
        aether::Application app;

        if (!app.initialize()) {
            std::cerr << "Failed to initialize application" << std::endl;
#ifdef _WIN32
            pauseOnError();
#endif
            return 1;
        }

        app.run();
        app.shutdown();

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
#ifdef _WIN32
        pauseOnError();
#endif
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception occurred" << std::endl;
#ifdef _WIN32
        pauseOnError();
#endif
        return 1;
    }
}
