mtrace-sh
=========

GNU glibc provides a built-in malloc(3) debugger, but the application needs to be built to enable it by invoking mtrace(3). Lacking the foresight to write the application correctly, using a combination of preloading a shared library that calls mtrace(3) from a static constructor will allow a pre-compiled application to be invoked with the feature enabled.