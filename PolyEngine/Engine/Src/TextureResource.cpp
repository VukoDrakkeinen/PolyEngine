#include "EnginePCH.hpp"

#include "TextureResource.hpp"
#include "ResourceManager.hpp"
#include "SOIL/SOIL.h"

#include "Logger.hpp"

using namespace Poly;

//------------------------------------------------------------------------------
TextureResource::TextureResource(const String& path)
{
	Image = SOIL_load_image(path.GetCStr(), &Width, &Height, &Channels, SOIL_LOAD_RGBA);
	if (Image == nullptr)
	{
		throw ResourceLoadFailedException();
	}

	gConsole.LogDebug("Loading texture {}: [WxH*C: {}x{}*{}]", path, Width, Height, Channels);

	// Flip Y axis
	int rowSize = Width*Channels;
	static Dynarray<unsigned char> row;
	row.Resize(rowSize);
	for (int i = 0; i < Height/2; ++i) {
		memcpy(row.GetData(), Image + ((Height - i - 1) * Width*Channels), sizeof(unsigned char) * rowSize);
		memcpy(Image + ((Height - i - 1) * Width*Channels), Image + (i * Width*Channels), sizeof(unsigned char) * rowSize);
		memcpy(Image + (i * Width*Channels), row.GetData(), sizeof(unsigned char) * rowSize);
	}

	TextureProxy = gEngine->GetRenderingDevice()->CreateTexture(Width, Height, eTextureUsageType::DIFFUSE); //HACK, remove deffise from here
	TextureProxy->SetContent(eTextureDataFormat::RGBA, Image);
	gConsole.LogDebug("Texture proxy at {}", (void*)TextureProxy.get());
}

//-----------------------------------------------------------------------------
TextureResource::~TextureResource()
{
	SOIL_free_image_data(Image);
}
