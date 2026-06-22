#include <cstdio>

// Non-trivial destructor -> forces the compiler to register the TLS object's
// teardown via __cxa_thread_atexit, pointing back into THIS shared library.
struct Foo {
    ~Foo() { printf("Foo destructor running\n"); }
};

// thread_local + non-trivial dtor = the registration we want to observe.
static thread_local Foo foo;

// Touching the object forces its construction, which registers the destructor.
extern "C" void touch() {
    (void)&foo;
    printf("touched\n");
}
