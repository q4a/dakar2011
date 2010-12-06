#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([Dakar2011],[],[btuska@elte.hu])
AC_CONFIG_SRCDIR([BigTerrain.cpp])
#AC_CONFIG_HEADER([config.h])

LDFLAGS="$LDFLAGS -L/usr/X11R6/lib"
CPPFLAGS="$CPPFLAGS -I/usr/X11R6/include"

# ARGS
AC_ARG_WITH(irrlicht, [  --with-irrlicht=ARG     use Irrlicht from the path at ARG], [], [])
if test -n "$with_irrlicht"
then
    if test "$with_irrlicht" != "yes"
    then
        LDFLAGS="$LDFLAGS -L${with_irrlicht}/lib/Linux"
        CPPFLAGS="$CPPFLAGS -I${with_irrlicht}/include"
        CPPFLAGS="$CPPFLAGS -I${with_irrlicht}/source/Irrlicht"
        # -L${with_irrlicht}/source/Irrlicht
        AC_MSG_NOTICE([irrlicht is set to $with_irrlicht])
        #AC_MSG_ERROR([$LDFLAGS])
    else
        AC_MSG_NOTICE([irrlicht is set])
    fi
else
    AC_MSG_NOTICE([irrlicht is not set])
fi

AC_ARG_WITH(newton, [  --with-newton=ARG       use Newton from the path at ARG], [], [])
if test -n "$with_newton"
then
    if test "$with_newton" != "yes"
    then
        LDFLAGS="$LDFLAGS -L${with_newton}/sdk"
        CPPFLAGS="$CPPFLAGS -I${with_newton}/sdk"
        AC_MSG_NOTICE([newton is set to $with_newton])
    else
        AC_MSG_NOTICE([newton is set])
    fi
else
    AC_MSG_NOTICE([newton is not set])
fi

AC_ARG_WITH(cg, [  --with-cg=ARG           use Cg from the path at ARG], [], [])
if test -n "$with_cg"
then
    if test "$with_cg" != "yes"
    then
        LDFLAGS="$LDFLAGS -L${with_cg}/lib"
        CPPFLAGS="$CPPFLAGS -I${with_cg}/include"
        AC_MSG_NOTICE([cg is set to $with_cg])
    else
        AC_MSG_NOTICE([cg is set])
    fi
else
    AC_MSG_NOTICE([cg is not set])
fi

AC_ARG_WITH(openal, [  --with-openal=ARG       use OpenAL from the path at ARG], [], [])
if test -n "$with_openal"
then
    if test "$with_openal" != "yes"
    then
        LDFLAGS="$LDFLAGS -L${with_openal}/build"
        CPPFLAGS="$CPPFLAGS -I${with_openal}/include"
        AC_MSG_NOTICE([openal is set to $with_openal])
    else
        AC_MSG_NOTICE([openal is set])
    fi
else
    AC_MSG_NOTICE([openal is not set])
fi

AC_ARG_WITH(alut, [  --with-alut=ARG         use alut from the path at ARG], [], [])
if test -n "$with_alut"
then
    if test "$with_alut" != "yes"
    then
        LDFLAGS="$LDFLAGS -L${with_alut}/lib"
        CPPFLAGS="$CPPFLAGS -I${with_alut}/include"
        AC_MSG_NOTICE([alut is set to $with_alut])
    else
        AC_MSG_NOTICE([alut is set])
    fi
else
    AC_MSG_NOTICE([alut is not set])
fi

AC_ARG_WITH(irrklang, [  --with-irrklang[[=ARG]]   use irrKlang [[at path ARG]] instead of the pure OpenAL], [AC_SUBST([USE_MY_SOUNDENGINE],[])], [AC_SUBST([USE_MY_SOUNDENGINE],[-DUSE_MY_SOUNDENGINE])])
if test -n "$with_irrklang"
then
    if test "$with_irrklang" != "yes"
    then
        LDFLAGS="$LDFLAGS -L${with_irrklang}/bin/linux-gcc ${with_irrklang}/bin/linux-gcc/ikpMP3.so ${with_irrklang}/bin/linux-gcc/ikpFlac.so"
        CPPFLAGS="$CPPFLAGS -I${with_irrklang}/include"
        AC_SUBST([IRRKLANG_LIB],[-lIrrKlang])
        AC_MSG_NOTICE([irrklang is set to $with_irrklang])
    else
        AC_SUBST([IRRKLANG_LIB],[])
        AC_MSG_NOTICE([irrklang is set])
    fi
else
    AC_SUBST([IRRKLANG_LIB],[])
    AC_MSG_NOTICE([irrklang is not set])
fi

# Checks for programs.
AC_PROG_CXX

# Checks for libraries.
AC_CHECK_LIB([m], [fabsf], [], [AC_MSG_ERROR([Install devel libraries])])
AC_CHECK_LIB([pthread], [pthread_self], [], [AC_MSG_ERROR([Install devel libraries])])
AC_CHECK_LIB([X11], [XInitImage], [], [AC_MSG_ERROR([Install devel libraries for X11 and set LDFLAGS and CPPFLAGS])])
AC_CHECK_LIB([Xcursor], [XcursorImageCreate], [], [AC_MSG_ERROR([Install devel libraries for X11 and set LDFLAGS and CPPFLAGS])])
AC_CHECK_LIB([Xext], [XextCreateExtension], [], [AC_MSG_ERROR([Install devel libraries for X11 and set LDFLAGS and CPPFLAGS])])
AC_CHECK_LIB([Xxf86vm], [XF86VidModeGetViewPort], [], [AC_MSG_ERROR([Install devel libraries for X11 and set LDFLAGS and CPPFLAGS])])


AC_CHECK_LIB([Irrlicht], [createDevice], [], [AC_MSG_ERROR([Download (from http://irrlicht.sourceforge.net/) and unpack Irrlicht and set --with-irrlicht])])
AC_CHECK_LIB([Irrlicht], [ _ZN3irr5scene13CSceneManager19addTerrainSceneNodeEPK10CHeightmapPNS0_10ISceneNodeEiRKNS_4core8vector3dIfEESB_SB_NS_5video6SColorEiNS0_20E_TERRAIN_PATCH_SIZEEibiii], [], [AC_MSG_ERROR([Download Irrlicht patch and patch the Irrlicht])])
AC_CHECK_LIB([Newton], [NewtonCreate], [], [AC_MSG_ERROR([Download (from http://www.newtondynamics.com/) and unpack Newton and set --with-newton])])
AC_CHECK_LIB([Cg], [cgCreateContext], [], [AC_MSG_ERROR([Download (from http://developer.nvidia.com/page/cg_main.html) and unpack Cg and set --with-cg])])
AC_CHECK_LIB([CgGL], [cgGLLoadProgram], [], [AC_MSG_ERROR([Download (from http://developer.nvidia.com/page/cg_main.html) and unpack Cg and set --with-cg])])
AC_CHECK_LIB([openal], [alBuffer3f], [], [AC_MSG_ERROR([Download (from http://connect.creativelabs.com/openal/default.aspx) and unpack OpenAL and set --with-openal])])
AC_CHECK_LIB([alut], [alutInit], [], [AC_MSG_ERROR([Download (from http://connect.creativelabs.com/openal/default.aspx) and unpack alut and set --with-alut])])

if test -n "$with_irrklang"
then
    AC_CHECK_LIB([IrrKlang], [_ZN8irrklang20createIrrKlangDeviceENS_21E_SOUND_OUTPUT_DRIVEREiPKcS2_], [], [AC_MSG_ERROR([Download (from http://www.ambiera.com/irrklang/) and unpack IrrKlang and set --with-irrklang])])
fi

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([arpa/inet.h fcntl.h float.h netdb.h netinet/in.h stdlib.h string.h sys/socket.h sys/time.h unistd.h wchar.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM

# Checks for library functions.
#AC_FUNC_VPRINTF
AC_CHECK_FUNCS([gettimeofday memset socket sqrt strstr], [], [AC_MSG_ERROR([Unsupported function found])])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
