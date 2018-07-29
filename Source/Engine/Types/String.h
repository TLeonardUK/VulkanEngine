#pragma once
#include "Pch.h"

#include "Engine/Types/Array.h"

using String = std::string;

String StringFormat(const String& Format, va_list List);
String StringFormat(String Format, ...);

String FormatBytes(int bytes);

String SliceString(const String& value, int start, int end);
Array<String> SplitString(const String& value, const String& seperator, int maxSplits = 0, bool removeDuplicates = false);