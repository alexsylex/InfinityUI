#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include "RE/Skyrim.h"
#include "REL/Relocation.h"
#include "SKSE/SKSE.h"

#include "RE/G/GFxCharacterHandle.h"
#include "RE/G/GFxSprite.h"

#include <spdlog/sinks/basic_file_sink.h>

#ifndef NDEBUG
#include <spdlog/sinks/msvc_sink.h>
#endif

#define DLLEXPORT __declspec(dllexport)

using namespace std::literals;
using namespace REL::literals;
