#pragma once

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"

#include <memory>

class IGraphicsImageView
{
protected:
	IGraphicsImageView() { };

public:
	virtual ~IGraphicsImageView() { };

	virtual int GetWidth() = 0;
	virtual int GetHeight() = 0;
	virtual std::shared_ptr<IGraphicsImage> GetImage() = 0;

};
