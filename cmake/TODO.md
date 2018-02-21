- disable `-DNDEBUG` in release builds

- figure out what to do with `process.config`, it's derived from `config.gypi`

- integrate `deps/openssl/asm/Makefile` with the cmake build somehow but only
  regenerate the asm sources when explicitly requested
