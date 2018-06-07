#include "Engine/Input/Sdl/SdlInput.h"
#include "Engine/Engine/Logging.h"

#include <memory>

#include <SDL.h>
#include <SDL_video.h>

SdlInput::SdlInput(std::shared_ptr<Logger> logger)
	: m_logger(logger)
{
}

SdlInput::~SdlInput()
{
	Dispose();
}

void SdlInput::Dispose()
{
}

bool SdlInput::Setup()
{
	return true;
}

std::shared_ptr<IInput> SdlInput::Create(
	std::shared_ptr<Logger> logger)
{
	std::shared_ptr<SdlInput> handle = std::make_shared<SdlInput>(logger);

	if (!handle->Setup())
	{
		handle->Dispose();
		return nullptr;
	}

	return handle;
}