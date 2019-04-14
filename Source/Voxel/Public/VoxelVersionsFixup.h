// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Launch/Resources/Version.h"

#if ENGINE_MINOR_VERSION < 22
#define UE_DEPRECATED(VERSION, MESSAGE) DEPRECATED(VERSION, MESSAGE)
#endif