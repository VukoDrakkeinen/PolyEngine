#pragma once

#include <IRenderingDevice.hpp>
#include "GLUtils.hpp"

namespace Poly
{
	struct ScreenSize;

	enum class eInternalTextureUsageType
	{
		NONE,
		COLOR_ATTACHEMENT,
		DEPTH_ATTACHEMENT,
		_COUNT
	};

	class GLTextureDeviceProxy : public ITextureDeviceProxy
	{
	public:
		GLTextureDeviceProxy(size_t width, size_t height, eInternalTextureUsageType internalUsage, GLenum internalFormat);
		GLTextureDeviceProxy(size_t width, size_t height, eTextureUsageType usage);
		virtual ~GLTextureDeviceProxy();

		void SetContent(eTextureDataFormat inputFormat, const unsigned char* data) override;
		void SetSubContent(size_t width, size_t height, size_t offsetX, size_t offsetY, eTextureDataFormat format, const unsigned char* data) override;

		GLuint GetTextureID() const { return TextureID; }

		void Resize(const ScreenSize& size);
	private:
		void InitTextureParams();

		size_t Width = 0;
		size_t Height = 0;
		GLuint TextureID = 0;
		GLenum InternalFormat;
		eInternalTextureUsageType InternalUsage = eInternalTextureUsageType::NONE;
		eTextureUsageType Usage = eTextureUsageType::_COUNT;

		friend class GLRenderingDevice;
	};
}
