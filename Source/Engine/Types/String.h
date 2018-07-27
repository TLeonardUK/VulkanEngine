#pragma once
#include "Pch.h"

using String = std::string;

String StringFormat(const String& Format, va_list List);
String StringFormat(String Format, ...);