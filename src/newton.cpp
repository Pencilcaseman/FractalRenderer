#include <fractal/fractal.hpp>

namespace frac {
	NewtonFractal::NewtonFractal(const RenderConfig &config) : Fractal(config) {}

	size_t NewtonFractal::supportedOptimisations() const {
		return 0; // No optimisations supported (currently)
	}

	std::pair<int64_t, lrc::Complex<LowPrecision>>
	NewtonFractal::iterCoordLow(const lrc::Complex<LowPrecision> &coord) const {
		// Roots (solutions) of the polynomial
		static std::vector<lrc::Complex<LowPrecision>> roots = {
		  lrc::Complex<LowPrecision>(1, 0),
		  lrc::Complex<LowPrecision>(-.5, sqrt(3) / 2),
		  lrc::Complex<LowPrecision>(-.5, -sqrt(3) / 2)};

		constexpr double tolerance = 0.0001;

		lrc::Complex<LowPrecision> z = coord;

		for (int64_t iteration = 0; iteration < m_renderConfig.maxIters; iteration++) {
			z -= f(z) / df(z);

			for (int64_t i = 0; i < roots.size(); i++) {
				lrc::Complex<LowPrecision> difference = z - roots[i];

				// If the current iteration is close enough to a root, color the
				// pixel.
				if (lrc::abs(difference.real()) < tolerance &&
					lrc::abs(difference.imag()) < tolerance) {
					return std::make_pair(i, z);
				}
			}
		}

		return std::make_pair(int64_t(0), lrc::Complex<LowPrecision>(0, 0));
	}

	std::pair<int64_t, lrc::Complex<HighPrecision>>
	NewtonFractal::iterCoordHigh(const lrc::Complex<HighPrecision> &coord) const {
		HighPrecision re_0 = lrc::real(coord); // Real component (initial)
		HighPrecision im_0 = lrc::imag(coord); // Imaginary component (initial)
		HighPrecision re = 0, im = 0;
		HighPrecision tmp; // Temporary variable for use in the calculation
		int64_t iteration = 0;

		// Bail when larger than this
		double bailout = 1 << 16;

		while (re * re + im * im <= bailout &&
			   iteration < Fractal::m_renderConfig.maxIters) {
			tmp = re * re - im * im + re_0;
			im	= 2 * re * im + im_0;
			re	= tmp;
			++iteration;
		}

		return {iteration, lrc::Complex<HighPrecision>(re, im)};
	}

	ci::ColorA NewtonFractal::getColorLow(const lrc::Complex<LowPrecision> &coord,
										  int64_t iters) const {
		if (iters == 0) { return ci::ColorA(1, 0, 0, 1); }
		if (iters == 1) { return ci::ColorA(0, 1, 0, 1); }
		if (iters == 2) { return ci::ColorA(0, 0, 1, 1); }
		return ci::ColorA(1, 1, 1, 1);
	}

	ci::ColorA NewtonFractal::getColorHigh(const lrc::Complex<HighPrecision> &coord,
										   int64_t iters) const {
		if (iters == 0) { return ci::ColorA(1, 0, 0, 1); }
		if (iters == 1) { return ci::ColorA(0, 1, 0, 1); }
		if (iters == 2) { return ci::ColorA(0, 0, 1, 1); }
		return ci::ColorA(1, 1, 1, 1);
	}
} // namespace frac
