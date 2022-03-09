#include <emscripten/bind.h>

using namespace emscripten;

std::string getText()
{
    return "Hello there from C++!";
}

EMSCRIPTEN_BINDINGS(my_module) {
        function("getText", &getText);
}