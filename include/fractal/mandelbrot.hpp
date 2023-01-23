#pragma once

#include <fractal/genericFractal.hpp>

namespace frac {
	class Mandelbrot : public Fractal {
	public:
		Mandelbrot(const RenderConfig &config);
		Mandelbrot(const Mandelbrot &)			  = delete;
		Mandelbrot(Mandelbrot &&)				  = delete;
		Mandelbrot &operator=(const Mandelbrot &) = delete;
		Mandelbrot &operator=(Mandelbrot &&)	  = delete;

		int64_t iterCoord(const lrc::Complex<HighPrecision> &coord) const override;
	};
} // namespace frac
