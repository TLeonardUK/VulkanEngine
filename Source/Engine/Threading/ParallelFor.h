#pragma once
#include "Pch.h"

#include "Engine/Types/String.h"

typedef std::function<void(int index)> ParallelForSignature_t;

void ParallelFor(int count, ParallelForSignature_t, int granularity = 1, const String& name = "Parallel For Task");
