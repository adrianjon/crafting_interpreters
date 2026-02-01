#include <emscripten.h>

#include <stdio.h>
#include <string.h>

int main(void) {
    printf("Hello, WebAssembly with Emscripten!\n");
    return 0;
}

EMSCRIPTEN_KEEPALIVE
void set_parameter(const char* param_name, float value) {
    (void)param_name;
    (void)value;
}