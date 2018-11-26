#pragma once

#if defined(TWEEDLEDUM_FMT_INTERNAL)
#ifndef FMT_HEADER_ONLY
#define FMT_HEADER_ONLY
#endif
#ifndef FMT_USE_WINDOWS_H
#define FMT_USE_WINDOWS_H 0
#endif
#include "core.h"
#include "format.h"
#else
#include <fmt/core.h>
#include <fmt/format.h>
#endif
