#include <fractal/fractal.hpp>

namespace frac {
	Mandelbrot::Mandelbrot(const RenderConfig &config) : Fractal(config) {}

	/*
	 * Note that, since this class will be used in a polymorphic manner with other, similar classes,
	 * these two functions must be implemented separately and cannot be templated, as the compiler
	 * would error when trying to identify which function to call. Additionally, splitting the
	 * functions in this way allows for more targeted optimisations to be made in some cases.
	 */

	std::pair<int64_t, lrc::Complex<LowPrecision>>
	Mandelbrot::iterCoordLow(const lrc::Complex<LowPrecision> &coord) const {
		LowPrecision re_0 = lrc::real(coord); // Real component (initial)
		LowPrecision im_0 = lrc::imag(coord); // Imaginary component (initial)
		LowPrecision re = 0, im = 0;
		LowPrecision tmp; // Temporary variable for use in the calculation
		int64_t iteration = 0;

		// Bail when larger than this
		double bailout = 1 << 16;

		while (re * re + im * im <= bailout && iteration < Fractal::m_renderConfig.maxIters) {
			tmp = re * re - im * im + re_0;
			im	= 2 * re * im + im_0;
			re	= tmp;
			++iteration;
		}

		return {iteration, lrc::Complex<LowPrecision>(re, im)};
	}

	std::pair<int64_t, lrc::Complex<HighPrecision>>
	Mandelbrot::iterCoordHigh(const lrc::Complex<HighPrecision> &coord) const {
		HighPrecision re_0 = lrc::real(coord); // Real component (initial)
		HighPrecision im_0 = lrc::imag(coord); // Imaginary component (initial)
		HighPrecision re = 0, im = 0;
		HighPrecision tmp; // Temporary variable for use in the calculation
		int64_t iteration = 0;

		// Bail when larger than this
		double bailout = 1 << 16;

		while (re * re + im * im <= bailout && iteration < Fractal::m_renderConfig.maxIters) {
			tmp = re * re - im * im + re_0;
			im	= 2 * re * im + im_0;
			re	= tmp;
			++iteration;
		}

		return {iteration, lrc::Complex<HighPrecision>(re, im)};
	}
} // namespace frac
