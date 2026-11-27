‘bootstrap-O1’
Removes any -O-started option from BOOT_CFLAGS, and adds -O1 to it. ‘BUILD_CONFIG=bootstrap-O1’ is equivalent to ‘BOOT_CFLAGS='-g -O1'’.

‘bootstrap-O3’
‘bootstrap-Og’
Analogous to bootstrap-O1.

‘bootstrap-native’
‘bootstrap-native’
Optimize the compiler code for the build host, if supported by the architecture. Note this only affects the compiler, not the targeted code. If you want the later, choose options suitable to the target you are looking for. For example ‘--with-cpu’ would be a good starting point.

‘bootstrap-lto’
Enables Link-Time Optimization for host tools during bootstrapping. ‘BUILD_CONFIG=bootstrap-lto’ is equivalent to adding -flto to ‘BOOT_CFLAGS’. This option assumes that the host supports the linker plugin (e.g. GNU ld version 2.21 or later or GNU gold version 2.21 or later).

‘bootstrap-lto-noplugin’
This option is similar to bootstrap-lto, but is intended for hosts that do not support the linker plugin. Without the linker plugin static libraries are not compiled with link-time optimizations. Since the GCC middle end and back end are in libbackend.a this means that only the front end is actually LTO optimized.

‘bootstrap-lto-lean’
This option is similar to bootstrap-lto, but is intended for faster build by only using LTO in the final bootstrap stage. With ‘make profiledbootstrap’ the LTO frontend is trained only on generator files.

‘bootstrap-debug’
Verifies that the compiler generates the same executable code, whether or not it is asked to emit debug information. To this end, this option builds stage2 host programs without debug information, and uses contrib/compare-debug to compare them with the stripped stage3 object files. If BOOT_CFLAGS is overridden so as to not enable debug information, stage2 will have it, and stage3 won’t. This option is enabled by default when GCC bootstrapping is enabled, if strip can turn object files compiled with and without debug info into identical object files. In addition to better test coverage, this option makes default bootstraps faster and leaner.

‘bootstrap-debug-big’
Rather than comparing stripped object files, as in bootstrap-debug, this option saves internal compiler dumps during stage2 and stage3 and compares them as well, which helps catch additional potential problems, but at a great cost in terms of disk space. It can be specified in addition to ‘bootstrap-debug’.

‘bootstrap-debug-lean’
This option saves disk space compared with bootstrap-debug-big, but at the expense of some recompilation. Instead of saving the dumps of stage2 and stage3 until the final compare, it uses -fcompare-debug to generate, compare and remove the dumps during stage3, repeating the compilation that already took place in stage2, whose dumps were not saved.

‘bootstrap-debug-lib’
This option tests executable code invariance over debug information generation on target libraries, just like bootstrap-debug-lean tests it on host programs. It builds stage3 libraries with -fcompare-debug, and it can be used along with any of the bootstrap-debug options above.

There aren’t -lean or -big counterparts to this option because most libraries are only build in stage3, so bootstrap compares would not get significant coverage. Moreover, the few libraries built in stage2 are used in stage3 host programs, so we wouldn’t want to compile stage2 libraries with different options for comparison purposes.

‘bootstrap-debug-ckovw’
Arranges for error messages to be issued if the compiler built on any stage is run without the option -fcompare-debug. This is useful to verify the full -fcompare-debug testing coverage. It must be used along with bootstrap-debug-lean and bootstrap-debug-lib.

‘bootstrap-cet’
This option enables Intel CET for host tools during bootstrapping. ‘BUILD_CONFIG=bootstrap-cet’ is equivalent to adding -fcf-protection to ‘BOOT_CFLAGS’. This option assumes that the host supports Intel CET (e.g. GNU assembler version 2.30 or later).

‘bootstrap-time’
Arranges for the run time of each program started by the GCC driver, built in any stage, to be logged to time.log, in the top level of the build tree.

‘bootstrap-asan’
Compiles GCC itself using Address Sanitization in order to catch invalid memory accesses within the GCC code.

‘bootstrap-hwasan’
Compiles GCC itself using HWAddress Sanitization in order to catch invalid memory accesses within the GCC code. This option is only available on AArch64 systems that are running Linux kernel version 5.4 or later.

‘bootstrap-ubsan’
Compiles GCC itself using Undefined Behavior Sanitization in order to catch undefined behavior within the GCC code. Note that it does not abort on errors by default. UBSAN_OPTIONS can be set to change this, like ‘UBSAN_OPTIONS='abort_on_error=1:halt_on_error=1:print_summary=1:print_stacktrace=1'’.

