#pragma once

#include <fractal/genericFractal.hpp>

namespace frac {
	class Mandelbrot : public Fractal {
	public:
		/// Constructor taking a RenderConfig object
		/// \param config RenderConfig object
		explicit Mandelbrot(const RenderConfig &config);
		Mandelbrot(const Mandelbrot &)			  = delete;
		Mandelbrot(Mandelbrot &&)				  = delete;
		Mandelbrot &operator=(const Mandelbrot &) = delete;
		Mandelbrot &operator=(Mandelbrot &&)	  = delete;

		~Mandelbrot() override = default;

		std::pair<int64_t, lrc::Complex<HighPrecision>>
		iterCoord(const lrc::Complex<HighPrecision> &coord) const override;
	};
} // namespace frac
