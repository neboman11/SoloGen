AC_INIT(SoloGen, version-0.1)

dnl Switch to a C++ compiler, and check if it works.
AC_LANG(C++)
AC_PROG_CXX

dnl Check for TCLAP and AudioFile
AC_CHECK_HEADERS(tclap/CmdLine.h, [], [AC_ERROR([TCLAP is required for this project.])])
dnl AC_CHECK_HEADERS(AudioFile/AudioFile.h, [], [AC_ERROR([AudioFile is required for this project.])])

dnl Process Makefile.in to create Makefile
AC_OUTPUT(Makefile)