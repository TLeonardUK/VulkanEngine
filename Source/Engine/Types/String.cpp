#include "Engine/Types/String.h"

String StringFormat(const String& Format, va_list List)
{
	String buffer;
	int bufferSize = 1024;

	while (true)
	{
		buffer.resize(bufferSize);

		va_list listCopy;
		va_copy(listCopy, List);
		int n = vsnprintf((char*)buffer.data(), buffer.size(), Format.c_str(), listCopy);
		va_end(listCopy);

		if (n > bufferSize)
		{
			bufferSize = n;
		}
		else if (n < 0)
		{
			// failed, abort.
			return "";
		}
		else
		{
			buffer.resize(n);
			break;
		}
	}

	return buffer;
}

String StringFormat(String Format, ...)
{
	va_list list;
	va_start(list, Format);
	String buffer = StringFormat(Format, list);
	va_end(list);
	return buffer;
}