AC_PREREQ([2.68])
AC_INIT([libhislip], [0.1], [], [libhislip], [https://lxi.github.io])
AC_CONFIG_HEADERS([src/include/config.h])
AM_INIT_AUTOMAKE([1.11 foreign dist-xz no-dist-gzip -Wall -Werror])
AM_SILENT_RULES([yes])
AM_PROG_AR

LT_PREREQ([2.2])
LT_INIT

# Before making a release, the LIBLXI_LT_VERSION string should be
# modified.
# The string is of the form C:R:A (Current, Revision, Age).
# - If interfaces have been changed or added, but binary compatibility has
#   been preserved, change to C+1:0:A+1
# - If binary compatibility has been broken (eg. removed or changed interfaces)
#   change to C+1:0:0
# - If the interface is the same as the previous version, change to C:R+1:A
LIBHISLIP_LT_VERSION=0:0:0
AC_SUBST(LIBHISLIP_LT_VERSION)

AC_ENABLE_SHARED
AC_ENABLE_STATIC
AC_PROG_CC
AC_LANG([C])
AC_PROG_INSTALL

# Check for posix pthread headers
AC_CHECK_HEADERS(pthread.h, [], [AC_ERROR([Required pthread headers not found])])

# Check for socket headers
AC_CHECK_HEADERS(sys/socket.h, [], [AC_ERROR([Required socket headers not found])])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([man/Makefile])
AC_OUTPUT
