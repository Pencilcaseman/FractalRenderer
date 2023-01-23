#pragma once

#include <cinderbox/cinderbox.hh>
#include <librapid>
#include <fstream>
#include <nlohmann/json.hpp>
#include <BS_thread_pool.hpp>

namespace lrc = librapid;

using ThreadPool = BS::thread_pool;
using json		 = nlohmann::json;

namespace frac {
	using HighPrecision = double;
	using LowPrecision	= double;
} // namespace frac

#include <fractal/debug.hpp>
#include <fractal/renderConfig.hpp>
#include <fractal/genericFractal.hpp>
#include <fractal/mandelbrot.hpp>
#include <fractal/mainWindow.hpp>
