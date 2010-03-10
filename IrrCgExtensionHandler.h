// Copyright (C) 2007-2009 Patryk Nadrowski
// This file is part of the "irrCg".
// For conditions of distribution and use, see copyright notice in License.txt

#ifndef __IRR_CG_EXTENSION_HANDLER_H_INCLUDED__
#define __IRR_CG_EXTENSION_HANDLER_H_INCLUDED__

#ifdef IrrCgOGL

#include <GL/gl.h>

#ifdef IrrCgWin32
#include <windows.h>
#include <glext.h>
#endif

#ifdef IrrCgLinux
#include <GL/glx.h>
#endif

namespace IrrCg
{
    class IrrCgExtensionHandler
    {
    public:
        IrrCgExtensionHandler() : pGlActiveTextureARB(0)
        {
            #ifdef IrrCgWin32
            pGlActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC) wglGetProcAddress("glActiveTextureARB");
            #endif

            #ifdef IrrCgLinux
            pGlActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC) glXGetProcAddress(reinterpret_cast<const GLubyte*>("glActiveTextureARB"));
            #endif
        }

        ~IrrCgExtensionHandler(){};

        inline void extGlActiveTexture(GLenum texture)
        {
            if (pGlActiveTextureARB)
            pGlActiveTextureARB(texture);
        }

    private:
		PFNGLACTIVETEXTUREARBPROC pGlActiveTextureARB;
    };
}

#endif

#endif
