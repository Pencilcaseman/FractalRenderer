#pragma once

#include <cinderbox/cinderbox.hh>
#include <librapid>
#include <fstream>
#include <nlohmann/json.hpp>
#include <BS_thread_pool.hpp>

#ifndef FRACTAL_SETTINGS_PATH
#	define FRACTAL_UI_SETTINGS_PATH FRACTAL_RENDERER_ROOT_DIR "/settings/settings.json"
#endif

namespace lrc = librapid;

using ThreadPool = BS::thread_pool;
using json		 = nlohmann::json;

namespace frac {
	// using HighPrecision = double;
	using HighPrecision = lrc::mpf;
	using LowPrecision	= double;

	using HighVec2 = lrc::Vec<HighPrecision, 2>;
	using LowVec2  = lrc::Vec<LowPrecision, 2>;
} // namespace frac

#include <fractal/debug.hpp>
#include <fractal/colorPalette.hpp>
#include <fractal/openglUtils.hpp>
#include <fractal/renderConfig.hpp>
#include <fractal/genericFractal.hpp>
#include <fractal/mandelbrot.hpp>
#include <fractal/fractalRenderer.hpp>
#include <fractal/mainWindow.hpp>
