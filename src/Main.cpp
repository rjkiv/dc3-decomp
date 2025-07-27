#include "App.h"

int main(int argc, char **argv) {
    App app(argc, argv);
    try {
        app.Run();
    } catch (int) {
    }
    // app.~App();
}
