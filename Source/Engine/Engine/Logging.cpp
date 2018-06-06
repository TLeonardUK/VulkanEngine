#include "Engine/Engine/Logging.h"
#include "Engine/Platform/Platform.h"

#include <stdarg.h>
#include <chrono>
#include <ctime>

const char* gLogSeverityLiteral[(int)LogSeverity::COUNT] =
{
#define LOG_SEVERITY(Name, Color) #Name,
#include "Engine/Engine/LogSeverity.inc"
#undef LOG_SEVERITY
};

ConsoleColor gLogSeverityConsoleColors[(int)LogSeverity::COUNT] =
{
#define LOG_SEVERITY(Name, Color) ConsoleColor::Color,
#include "Engine/Engine/LogSeverity.inc"
#undef LOG_SEVERITY
};

const char* gLogCategoryLiteral[(int)LogCategory::COUNT] =
{
#define LOG_CATEGORY(Name) #Name,
#include "Engine/Engine/LogCategory.inc"
#undef LOG_CATEGORY
};

Logger::Logger(std::shared_ptr<IPlatform> platform)
	: m_platform(platform)
{	
	m_startTime = std::chrono::duration_cast< std::chrono::milliseconds >(
		std::chrono::steady_clock::now().time_since_epoch()
	);
}

Logger::~Logger()
{
	Dispose();
}

void Logger::Dispose()
{
}

std::shared_ptr<Logger> Logger::Create(std::shared_ptr<IPlatform> platform)
{
	return std::make_shared<Logger>(platform);
}

void Logger::Write(LogSeverity serverity, LogCategory category, const String& data)
{
	const char* categoryStr = gLogCategoryLiteral[(int)category];
	const char* severityStr = gLogSeverityLiteral[(int)serverity];
	ConsoleColor severityColor = gLogSeverityConsoleColors[(int)serverity];

	//String buffer;
	//buffer.resize(data.size() + 64);
	
	std::chrono::milliseconds timestamp = std::chrono::duration_cast< std::chrono::milliseconds >(
		std::chrono::steady_clock::now().time_since_epoch()
		);
	long long timeSinceStart = (timestamp.count() - m_startTime.count());

	long long elapsedHours = ((timeSinceStart / 1000) / 60) / 60;
	long long elapsedMinutes = ((timeSinceStart / 1000) / 60) % 60;
	long long elapsedSeconds = ((timeSinceStart / 1000) % 60);
	long long elapsedMilliseconds = (timeSinceStart % 1000);

	String format = String("[%02i:%02i:%02i:%04i] [%-14s] %-8s: %s\n");
	String buffer = String::Format(
		format,
		elapsedHours,
		elapsedMinutes,
		elapsedSeconds,
		elapsedMilliseconds,
		categoryStr,
		severityStr,
		data.c_str());
/*	snprintf(
		(char*)buffer.data(), 
		buffer.size(), 
		"[%02i:%02i:%02i:%04i] [%-14s] %-8s: %s\n", 
		elapsedHours,
		elapsedMinutes,
		elapsedSeconds,
		elapsedMilliseconds,
		categoryStr,
		severityStr, 
		data.c_str());
*/

	m_platform->WriteToConsole(severityColor, buffer.c_str());
}

void Logger::WriteInfo(LogCategory category, const String data, ...)
{
	va_list list;
	va_start(list, data);
	String buffer = String::Format(data, list);
	va_end(list);

	Write(LogSeverity::Info, category, buffer);
}

void Logger::WriteWarning(LogCategory category, const String data, ...)
{
	va_list list;
	va_start(list, data);
	String buffer = String::Format(data, list);
	va_end(list);

	Write(LogSeverity::Warning, category, buffer);
}

void Logger::WriteError(LogCategory category, const String data, ...)
{
	va_list list;
	va_start(list, data);
	String buffer = String::Format(data, list);
	va_end(list);

	Write(LogSeverity::Error, category, buffer);
}

void Logger::WriteSuccess(LogCategory category, const String data, ...)
{
	va_list list;
	va_start(list, data);
	String buffer = String::Format(data, list);
	va_end(list);

	Write(LogSeverity::Success, category, buffer);
}

