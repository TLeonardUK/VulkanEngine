#include "Pch.h"

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

String FormatBytes(int bytes)
{
	const int suffixCount = 5;
	const char* suffixes[suffixCount] = { "Bytes", "KB", "MB", "GB", "TB" };

	int suffixIndex = 0;
	double floatingSize = bytes;
	while (true)
	{
		if (floatingSize < 1024 || suffixIndex >= suffixCount)
		{
			return StringFormat("%.2f %s", floatingSize, suffixes[suffixIndex]);
		}
		else
		{
			floatingSize /= 1024;
			suffixIndex++;
		}
	}
}

String SliceString(const String& value, int start, int end)
{
	int length = (int)value.size();

	if (start < 0)
	{
		start += length;
		if (start < 0)
		{
			start = 0;
		}
	}
	else if (start > length)
	{
		start = length;
	}

	if (end < 0)
	{
		end += length;
	}
	else if (end > length)
	{
		end = length;
	}

	if (start >= end)
	{
		return "";
	}
	else if (start == 0 && end == length)
	{
		return value;
	}
	else
	{
		return value.substr(start, end - start);
	}
}

Array<String> SplitString(const String& value, const String& seperator, int maxSplits, bool removeDuplicates)
{
	Array<String> result;
	String split = "";

	if (seperator == "")
	{
		result.push_back(value);
		return result;
	}

	for (size_t i = 0; i < value.size(); i++)
	{
		String res = SliceString(value, (int)i, (int)i + 1);
		if (i <= value.size() - seperator.size())
		{
			res = SliceString(value, (int)i, (int)i + (int)seperator.size());
		}

		if (res == seperator && ((int)result.size() < maxSplits || maxSplits <= 0))
		{
			if (split != "" || removeDuplicates == false)
			{
				result.push_back(split);
				split = "";
			}
			i += (seperator.size() - 1);
			continue;
		}
		else
		{
			split += SliceString(value, (int)i, (int)i + 1);
		}
	}

	if (split != "" || removeDuplicates == false)
	{
		result.push_back(split);
		split = "";
	}

	return result;
}
