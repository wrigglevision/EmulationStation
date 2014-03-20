#include "SVGResource.h"
#include "../nanosvg/nanosvg.h"
#include "../nanosvg/nanosvgrast.h"
#include "../Log.h"

#define DPI 96

SVGResource::SVGResource(const std::string& path, bool tile) : TextureResource(path, tile), mSVGImage(NULL)
{
	assert(tile == false);
}

SVGResource::~SVGResource()
{
	deinitSVG();
}

void SVGResource::unload(std::shared_ptr<ResourceManager>& rm)
{
	deinitSVG();
	TextureResource::unload(rm);
}

void SVGResource::initFromMemory(const char* file, size_t length)
{
	deinit();
	deinitSVG();

	// nsvgParse excepts a modifiable, null-terminated string
	char* copy = (char*)malloc(length + 1);
	memcpy(copy, file, length);
	copy[length] = '\0';

	mSVGImage = nsvgParse(copy, "px", DPI);
	free(copy);

	if(!mSVGImage)
	{
		LOG(LogError) << "Error parsing SVG image.";
		return;
	}

	rasterizeAt((int)mSVGImage->width, (int)mSVGImage->height);
}

void SVGResource::rasterizeAt(size_t width, size_t height)
{
	if(!mSVGImage)
		return;

	unsigned char* imagePx = (unsigned char*)malloc(width * height * 4);

	NSVGrasterizer* rast = nsvgCreateRasterizer();
	nsvgRasterize(rast, mSVGImage, 0, 0, width / mSVGImage->width, imagePx, width, height, width * 4);
	nsvgDeleteRasterizer(rast);

	// flip the pixels
	unsigned int temp;
	unsigned int* arr = (unsigned int*)imagePx;
	for(size_t y = 0; y < height / 2; y++)
	{
		for(size_t x = 0; x < width; x++)
		{
			temp = arr[x + (y * width)];
			arr[x + (y * width)] = arr[x + (height * width) - ((y + 1) * width)];
			arr[x + (height * width) - ((y + 1) * width)] = temp;
		}
	}

	initFromPixels(imagePx, width, height);
	free(imagePx);
}

Eigen::Vector2i SVGResource::getImageSize() const
{
	if(mSVGImage)
		return Eigen::Vector2i((int)mSVGImage->width, (int)mSVGImage->height);

	return Eigen::Vector2i::Zero();
}

void SVGResource::deinitSVG()
{
	if(mSVGImage)
		nsvgDelete(mSVGImage);

	mSVGImage = NULL;
}
