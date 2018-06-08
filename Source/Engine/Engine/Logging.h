#pragma once

#include "Engine/Types/String.h"

#include <stdarg.h>
#include <memory>
#include <chrono>

enum class LogCategory
{
#define LOG_CATEGORY(Name) Name,
#include "Engine/Engine/ELogCategory.inc"
#undef LOG_CATEGORY

	COUNT
};

enum class LogSeverity
{
#define LOG_SEVERITY(Name, Color) Name,
#include "Engine/Engine/ELogSeverity.inc"
#undef LOG_SEVERITY

	COUNT
};

class IPlatform;

class Logger
{
private:
	std::shared_ptr<IPlatform> m_platform;

	std::chrono::milliseconds m_startTime;

	void Write(LogSeverity serverity, LogCategory category, const String& data);

public:
	Logger(std::shared_ptr<IPlatform> platform);
	~Logger();
	void Dispose();

	void WriteInfo(LogCategory category, const String data, ...);
	void WriteWarning(LogCategory category, const String data, ...);
	void WriteError(LogCategory category, const String data, ...);
	void WriteSuccess(LogCategory category, const String data, ...);

	static std::shared_ptr<Logger> Create(std::shared_ptr<IPlatform> platform);
};
