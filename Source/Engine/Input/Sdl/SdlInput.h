#pragma once

#include <memory>

#include "Engine/Types/String.h"
#include "Engine/Input/Input.h"

class IGraphics;
class Logger;

class SdlInput
	: public IInput
{
private:
	std::shared_ptr<Logger> m_logger;

	bool Setup();

public:
	SdlInput(std::shared_ptr<Logger> logger);
	virtual ~SdlInput();
	virtual void Dispose();

	static std::shared_ptr<IInput> Create(
		std::shared_ptr<Logger> loggere);
};
