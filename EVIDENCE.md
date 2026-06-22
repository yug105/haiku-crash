# CI Evidence

Run: https://github.com/yug105/haiku-crash/actions/runs/27946979215

Commit: `2707b0f107dc5a9fce76901d7e25991dc7ba1466`

## Linux control

Linux builds the same `libtlslib.so`, calls `touch()`, closes the library,
then still runs the TLS destructor successfully at process exit:

```text
[ 50%] Linking CXX shared library libtlslib.so
[100%] Built target main
touched
dlclose done, about to exit
Foo destructor running
```

Result: `Linux (expected PASS)` passed.

## Haiku failure

Haiku builds the same `libtlslib.so` and reaches the same point in `main()`:

```text
[ 50%] Linking CXX shared library libtlslib.so
[100%] Built target main
===== running ./main =====
touched
dlclose done, about to exit
sh: line 15:   533 Kill Thread             ./main
===== ./main exit status: 149 =====
>>> CRASHED with signal 21 at exit (no keepalive)
```

The debug_server report then records a segment violation during process exit:

```text
thread 533: main (main)
    state: Exception (Segment violation)

    Frame        IP              Function Name
    -----------------------------------------------
    00000000    0x1ac098d07aa    ?
        Unable to retrieve disassembly for IP 0x1ac098d07aa: address not contained in any valid image.
    0x7fbde9767270    0x1ea6ce6383c    _GLOBAL__N_1::run(void*) + 0x1c
    0x7fbde97672b0    0x1bcb14264a8    __pthread_key_call_destructors + 0x48
    0x7fbde97672d0    0x1bcb14254ec    __pthread_destroy_thread + 0x2c
    0x7fbde97672f0    0x1bcb1415df9    _thread_do_exit_work + 0x49
    0x7fbde9767310    0x1bcb148eb4b    exit + 0xb
    0x7fbde9767318    0x9b5cb12925     main + 0
```

The same report's `Loaded Images` section lists `main`, `runtime_loader`,
`libgcc_s`, `libroot`, and `libstdc++`, but does not list `libtlslib.so`.
That is the important part: the registered TLS destructor is called at exit,
but its code image has already been unloaded by Haiku's `dlclose()`.

Result: `Haiku (expected CRASH at exit)` failed, as expected.

The full Haiku crash report was uploaded by the workflow as the
`haiku-crash-report` artifact.
