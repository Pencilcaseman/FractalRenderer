#pragma once

#include <cinderbox/cinderbox.hh>
#include <librapid>
#include <fstream>
#include <nlohmann/json.hpp>
#include <BS_thread_pool.hpp>

namespace lrc = librapid;

using ThreadPool = BS::thread_pool;
using json = nlohmann::json;

#include <fractal/debug.hpp>
#include <fractal/mainWindow.hpp>

