#include <cstdio>
#include <dlfcn.h>

int main() {
    // Unbuffered stdout: the fault happens at process exit (after main
    // returns), so anything left in a buffer would be lost. Force each line
    // out immediately so the CI log shows exactly how far we got.
    setvbuf(stdout, nullptr, _IONBF, 0);

    // Load the library that owns the thread_local Foo.
    void* handle = dlopen("./libtlslib.so", RTLD_NOW);
    if (!handle) {
        printf("dlopen failed: %s\n", dlerror());
        return 1;
    }

    // Resolve and call touch() -> constructs foo -> registers its TLS
    // destructor, with the destructor code living inside libtlslib.so.
    auto touch = reinterpret_cast<void (*)()>(dlsym(handle, "touch"));
    if (!touch) {
        printf("dlsym failed: %s\n", dlerror());
        return 1;
    }
    touch();

    // Unload the library. On Haiku this UNMAPS the code, so the registered
    // destructor pointer now dangles.
    dlclose(handle);
    printf("dlclose done, about to exit\n");

    // Process exit -> the runtime runs the TLS destructor we registered.
    // glibc keeps the library mapped (keepalive) so it survives; Haiku does
    // not, so the dangling pointer is called -> SIGSEGV.
    return 0;
}
