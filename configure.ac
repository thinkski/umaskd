#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.59])
AC_INIT([umaskd], [0.1], [chiszp@gmail.com])
AM_INIT_AUTOMAKE([1.9 no-define])
AC_CONFIG_SRCDIR([src/main.cc])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_INSTALL

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([limits.h stdlib.h string.h unistd.h])
AC_CHECK_HEADERS([sys/types.h sys/stat.h])
AC_CHECK_HEADERS([sys/inotify.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_CHECK_FUNCS([strchr strdup])

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
