dnl Process this file with autoconf to produce a configure script.
AC_INIT(src/atak.c)
AM_INIT_AUTOMAKE(gnuchess, 5.04)
AM_CONFIG_HEADER(src/config.h)

dnl Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_AWK
AC_PROG_LN_S
AC_STRUCT_TM

dnl Checks for libraries.
dnl AC_CHECK_LIB(m, sqrt)
AC_CHECK_LIB(ncurses, tputs)
AC_CHECK_LIB(readline, readline)

dnl Checks for header files.
AC_HEADER_STDC

AC_CHECK_HEADERS(fcntl.h time.h sys/file.h sys/ioctl.h sys/time.h unistd.h \
 errno.h readline/readline.h readline/history.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_CHECK_SIZEOF(long)
AC_NEED_STDINT_H(src/GCint.h)
dnl AC_C_BIGENDIAN

dnl Checks for library functions.
AC_TYPE_SIGNAL
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_CHECK_FUNCS(alarm gettimeofday strchr strcspn \
  strstr strtol strtoul strerror memset)

AC_CONFIG_FILES(Makefile src/Makefile)
AC_OUTPUT
