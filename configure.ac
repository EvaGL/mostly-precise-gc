#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_CONFIG_MACRO_DIR([m4])
AC_PREREQ([2.68])
AC_INIT([libgc], [0.1], [noreply@github.com])
AM_INIT_AUTOMAKE([libgc], [0.1])
LT_INIT

AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lm':
AC_CHECK_LIB([m], [msmalloc])

# Checks for header files.
AC_CHECK_HEADERS([limits.h stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_CHECK_FUNCS([gettimeofday munmap])

AC_CONFIG_FILES([Makefile
                 sources/Makefile])
AC_OUTPUT
