# Process this file with autoconf to produce a configure script.-*- autoconf -*-
AC_INIT([gtkmail], [1.2.0], [jwy@divisionbyzero.com])
AC_CONFIG_SRCDIR([config.h.in])
AM_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE([gtkmail], [1.2.0], [jwy@divisionbyzero.com])

gnomemm_version_needed=1.2.2
jlib_version_needed=1.2.0
pixbuf_version_needed=0.9.0

# Checks for programs.
AC_PROG_CXX(c++ g++)
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_AWK

# Checks for libraries.
# FIXME: Replace `main' with a function in `-le':
#AC_CHECK_LIB([e], [main])
# FIXME: Replace `main' with a function in `-links':

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE

AC_LANG(C++)

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_CHECK_FUNCS([mkdir mkfifo select strerror])

PKG_CHECK_MODULES(ALL, jlib-1.2 >= 1.2 gtkmm-2.4 >= 2.10.0 webkit-1.0 >= 1.1.12 popt >= 1.0, CXXFLAGS="$CXXFLAGS $ALL_CFLAGS";LIBS="$LIBS $ALL_LIBS", AC_MSG_ERROR(you need all of these))

AC_MSG_CHECKING([if webkit gtk+ headers can be included from C++])
AC_TRY_COMPILE(
[
    #include <webkit/webkitwebview.h>
],[
    GtkWidget* view = webkit_web_view_new();
],[
    use_html=yes
],[
    AC_MSG_ERROR([you need webkit gtk+ for HTML])
])
AC_MSG_RESULT([yes])

AC_CHECK_LIB([popt], [poptGetContext])

AC_CONFIG_FILES([Makefile src/Makefile src/xpm/Makefile])
AC_OUTPUT
