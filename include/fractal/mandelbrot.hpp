#pragma once

#include <fractal/genericFractal.hpp>

namespace frac {
	/*
	 * No need to document this file, since the class inherits from a generic fractal
	 * class and does not implement any novel functions.
	 */

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

		LIBRAPID_NODISCARD std::pair<int64_t, lrc::Complex<LowPrecision>>
		iterCoordLow(const lrc::Complex<LowPrecision> &coord) const override;

		LIBRAPID_NODISCARD std::pair<int64_t, lrc::Complex<HighPrecision>>
		iterCoordHigh(const lrc::Complex<HighPrecision> &coord) const override;
	};
} // namespace frac
