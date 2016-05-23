{
  'conditions': [
    [ 'target_arch=="ia32"', {
      'cflags': [ '-m32' ],
      'ldflags': [ '-m32' ],
    }],
    [ 'target_arch=="x32"', {
      'cflags': [ '-mx32' ],
      'ldflags': [ '-mx32' ],
    }],
    [ 'target_arch=="x64"', {
      'cflags': [ '-m64' ],
      'ldflags': [ '-m64' ],
    }],
    [ 'target_arch=="ppc" and OS!="aix"', {
      'cflags': [ '-m32' ],
      'ldflags': [ '-m32' ],
    }],
    [ 'target_arch=="ppc64" and OS!="aix"', {
      'cflags': [ '-m64', '-mminimal-toc' ],
      'ldflags': [ '-m64' ],
    }],
    [ 'target_arch=="s390"', {
      'cflags': [ '-m31' ],
      'ldflags': [ '-m31' ],
    }],
    [ 'target_arch=="s390x"', {
      'cflags': [ '-m64' ],
      'ldflags': [ '-m64' ],
    }],
    [ 'OS=="solaris"', {
      'cflags': [ '-pthreads' ],
      'ldflags': [ '-pthreads' ],
      'cflags!': [ '-pthread' ],
      'ldflags!': [ '-pthread' ],
    }],
    [ 'OS=="aix"', {
      'conditions': [
        [ 'target_arch=="ppc"', {
          'ldflags': [ '-Wl,-bmaxdata:0x60000000/dsa' ],
        }],
        [ 'target_arch=="ppc64"', {
          'cflags': [ '-maix64' ],
          'ldflags': [ '-maix64' ],
        }],
      ],
      'ldflags!': [ '-rdynamic' ],
    }],
    [ 'OS=="android" and _toolset=="target"', {
      'libraries': [ '-llog' ],
    }],
  ],
}
