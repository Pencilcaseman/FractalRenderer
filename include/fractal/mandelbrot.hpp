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

		LIBRAPID_NODISCARD size_t supportedOptimisations() const override;

		LIBRAPID_NODISCARD std::string name() const override;

		LIBRAPID_NODISCARD
		std::unordered_map<std::string, coloring::ColorFuncLow>
		getLowPrecColoringAlgorithms() const override;

		LIBRAPID_NODISCARD
		std::unordered_map<std::string, coloring::ColorFuncHigh>
		getHighPrecColoringAlgorithms() const override;

		LIBRAPID_NODISCARD std::pair<int64_t, lrc::Complex<LowPrecision>>
		iterCoordLow(const lrc::Complex<LowPrecision> &coord) const override;

		LIBRAPID_NODISCARD std::pair<int64_t, lrc::Complex<HighPrecision>>
		iterCoordHigh(const lrc::Complex<HighPrecision> &coord) const override;

		LIBRAPID_NODISCARD ci::ColorA
		getColorLow(const lrc::Complex<LowPrecision> &coord, int64_t iters,
					const ColorPalette &palette,
					const coloring::ColorFuncLow &colorFunc) const override;

		LIBRAPID_NODISCARD ci::ColorA
		getColorHigh(const lrc::Complex<HighPrecision> &coord, int64_t iters,
					 const ColorPalette &palette,
					 const coloring::ColorFuncHigh &colorFunc) const override;
	};
} // namespace frac
