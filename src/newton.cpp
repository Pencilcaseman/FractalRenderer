#include <fractal/fractal.hpp>

namespace frac {
	NewtonFractal::NewtonFractal(const RenderConfig &config) : Fractal(config) {}

	size_t NewtonFractal::supportedOptimisations() const {
		return 0; // No optimisations supported (currently)
	}

	std::string NewtonFractal::name() const { return "Newton's Fractal"; }

	std::unordered_map<std::string, coloring::ColorFuncLow>
	NewtonFractal::getLowPrecColoringAlgorithms() const {
		return {{"Fixed Iteration Palette",
				 std::function([](const lrc::Complex<LowPrecision> &coord,
								  int64_t iters,
								  const ColorPalette &palette) -> ci::ColorA {
					 return coloring::fixedIterPalette(coord, iters, palette);
				 })}};
	}

	std::unordered_map<std::string, coloring::ColorFuncHigh>
	NewtonFractal::getHighPrecColoringAlgorithms() const {
		return {{"Fixed Iteration Palette",
				 std::function([](const lrc::Complex<HighPrecision> &coord,
								  int64_t iters,
								  const ColorPalette &palette) -> ci::ColorA {
					 return coloring::fixedIterPalette(coord, iters, palette);
				 })}};
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
		// Roots (solutions) of the polynomial
		static std::vector<lrc::Complex<HighPrecision>> roots = {
		  lrc::Complex<HighPrecision>(1, 0),
		  lrc::Complex<HighPrecision>(-.5, sqrt(3) / 2),
		  lrc::Complex<HighPrecision>(-.5, -sqrt(3) / 2)};

		HighPrecision tolerance = 1e-15;

		lrc::Complex<HighPrecision> z = coord;

		for (int64_t iteration = 0; iteration < m_renderConfig.maxIters; iteration++) {
			z -= f(z) / df(z);

			for (int64_t i = 0; i < roots.size(); i++) {
				lrc::Complex<HighPrecision> difference = z - roots[i];

				// If the current iteration is close enough to a root, color the
				// pixel.
				if (lrc::abs(difference.real()) < tolerance &&
					lrc::abs(difference.imag()) < tolerance) {
					return std::make_pair(i, z);
				}
			}
		}

		return std::make_pair(int64_t(0), lrc::Complex<HighPrecision>(0, 0));
	}
} // namespace frac
