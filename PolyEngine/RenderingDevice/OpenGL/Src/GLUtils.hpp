#pragma once

//#include <glbinding/gl33core/enum.h>
//#include <glbinding/gl33core/functions.h>
//#include <glbinding/gl33core/types.h>
//using namespace gl33core

#include <glbinding/gl/gl.h>
using namespace gl;

#include <Logger.hpp>

namespace Poly {
	//---------------------------------------------------------------
	inline const char* OpenGLErrorStr(GLenum err) noexcept
	{
		switch (err)
		{
			case GL_INVALID_OPERATION:				return "INVALID_OPERATION";
			case GL_INVALID_ENUM:					return "INVALID_ENUM";
			case GL_INVALID_VALUE:					return "INVALID_VALUE";
			case GL_OUT_OF_MEMORY:					return "OUT_OF_MEMORY";
			case GL_INVALID_FRAMEBUFFER_OPERATION:  return "INVALID_FRAMEBUFFER_OPERATION";
			default:								ASSERTE(false, "Unknown OpenGL error type!");
													return nullptr;
		}
	}

	//---------------------------------------------------------------
	inline void CheckGLError(const char *file, int line)
	{
		GLenum err = GL_NO_ERROR;
		while ((err = glGetError()) != GL_NO_ERROR)
			gConsole.LogError("{}:{} OpenGL error: [{}] {}", file, line, err, OpenGLErrorStr(err));
	}

	inline void CheckFBOStatus(const char *file, int line)
	{
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

		if (status != GL_FRAMEBUFFER_COMPLETE) {
			std::string text;
			switch (status) {
			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: text = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT"; break;
			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: text = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT"; break;
			case GL_FRAMEBUFFER_UNSUPPORTED: text = "GL_FRAMEBUFFER_UNSUPPORTED"; break;
			default: text = "NO DESCRIPTION"; break;
			}
			gConsole.LogError("FrameBuffer error, status: [{}] {}", status, text);
		}
	}
}

#ifdef _DEBUG
	#define CHECK_GL_ERR() ::Poly::CheckGLError(__FILE__, __LINE__ )
	#define CHECK_FBO_STATUS() ::Poly::CheckFBOStatus(__FILE__, __LINE__ )
#else
	#define CHECK_GL_ERR() (void)0
	#define CHECK_FBO_STATUS() (void)0
#endif // _DEBUG

