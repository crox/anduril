// Wurkkos TS10 (RGB aux version) config options for Anduril
// Copyright (C) 2023 Selene ToyKeeper
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "wurkkos/ts10/anduril.h"

#undef RAMP_SMOOTH_CEIL
#undef RAMP_SMOOTH_FLOOR
#undef RAMP_DISCRETE_FLOOR RAMP_SMOOTH_FLOOR
#undef RAMP_DISCRETE_CEIL RAMP_SMOOTH_CEIL
#define RAMP_SMOOTH_CEIL 110
#define RAMP_SMOOTH_FLOOR 15
#define DEFAULT_MANUAL_MEMORY 20
#define RAMP_DISCRETE_CEIL RAMP_SMOOTH_CEIL
#define RAMP_DISCRETE_FLOOR RAMP_SMOOTH_FLOOR

#undef DEFAULT_MANUAL_MEMORY
