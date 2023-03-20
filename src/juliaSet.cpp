#include <fractal/fractal.hpp>

namespace frac {
	JuliaSet::JuliaSet(const RenderConfig &config) : Fractal(config) {}

	size_t JuliaSet::supportedOptimisations() const {
		// Outlining is proven for the Mandelbrot set
		return optimisations::OUTLINE_OPTIMISATION;
	}

	std::string JuliaSet::name() const { return "Julia Set"; }

	std::unordered_map<std::string, coloring::ColorFuncLow>
	JuliaSet::getLowPrecColoringAlgorithms() const {
		return {{"Logarithmic Scaling",
				 std::function([](const lrc::Complex<LowPrecision> &coord,
								  int64_t iters,
								  const ColorPalette &palette) -> ci::ColorA {
					 return coloring::logarithmicScaling(coord, iters, palette);
				 })},
				{"Paletted Logarithmic Scaling",
				 std::function([](const lrc::Complex<LowPrecision> &coord,
								  int64_t iters,
								  const ColorPalette &palette) -> ci::ColorA {
					 return coloring::palettedLogarithmicScaling(coord, iters, palette);
				 })},
				{"Stepped Gradients",
				 std::function([](const lrc::Complex<LowPrecision> &coord,
								  int64_t iters,
								  const ColorPalette &palette) -> ci::ColorA {
					 return coloring::steppedGradients(coord, iters, palette);
				 })},
				{"Fixed Iteration Palette",
				 std::function([](const lrc::Complex<LowPrecision> &coord,
								  int64_t iters,
								  const ColorPalette &palette) -> ci::ColorA {
					 return coloring::fixedIterPalette(coord, iters, palette);
				 })}};
	}

	std::unordered_map<std::string, coloring::ColorFuncHigh>
	JuliaSet::getHighPrecColoringAlgorithms() const {
		return {{"Logarithmic Scaling",
				 std::function([](const lrc::Complex<HighPrecision> &coord,
								  int64_t iters,
								  const ColorPalette &palette) -> ci::ColorA {
					 return coloring::fixedIterPalette(coord, iters, palette);
				 })},
				{"Paletted Logarithmic Scaling",
				 std::function([](const lrc::Complex<HighPrecision> &coord,
								  int64_t iters,
								  const ColorPalette &palette) -> ci::ColorA {
					 return coloring::palettedLogarithmicScaling(coord, iters, palette);
				 })},
				{"Stepped Gradients",
				 std::function([](const lrc::Complex<HighPrecision> &coord,
								  int64_t iters,
								  const ColorPalette &palette) -> ci::ColorA {
					 return coloring::steppedGradients(coord, iters, palette);
				 })},
				{"Fixed Iteration Palette",
				 std::function([](const lrc::Complex<HighPrecision> &coord,
								  int64_t iters,
								  const ColorPalette &palette) -> ci::ColorA {
					 return coloring::fixedIterPalette(coord, iters, palette);
				 })}};
	}

	std::pair<int64_t, lrc::Complex<LowPrecision>>
	JuliaSet::iterCoordLow(const lrc::Complex<LowPrecision> &coord) const {
		lrc::Complex<LowPrecision> c(-0.8, 0.156); // Julia set constant
		auto z			  = coord;
		int64_t iteration = 0;

		// Bail when larger than this
		double bailout = Fractal::m_renderConfig.bail;

		while (z.real() * z.real() + z.imag() * z.imag() <= bailout &&
			   iteration < Fractal::m_renderConfig.maxIters) {
			z = z * z + c;
			++iteration;
		}
		return {iteration, z};
	}

	std::pair<int64_t, lrc::Complex<HighPrecision>>
	JuliaSet::iterCoordHigh(const lrc::Complex<HighPrecision> &coord) const {
		lrc::Complex<HighPrecision> c(-0.8, 0.156); // Julia set constant
		auto z			  = coord;
		int64_t iteration = 0;

		// Bail when larger than this
		double bailout = Fractal::m_renderConfig.bail;

		while (z.real() * z.real() + z.imag() * z.imag() <= bailout &&
			   iteration < Fractal::m_renderConfig.maxIters) {
			z = z * z + c;
			++iteration;
		}
		return {iteration, z};
	}

	ci::ColorA JuliaSet::getColorLow(
	  const lrc::Complex<LowPrecision> &coord, int64_t iters, const ColorPalette &palette,
	  const std::function<ci::ColorA(const lrc::Complex<LowPrecision> &, int64_t,
									 const ColorPalette &)> &colorFunc) const {
		if (coord.real() * coord.real() + coord.imag() * coord.imag() < 4)
			return {0, 0, 0, 1};
		return colorFunc(coord, iters, palette);
	}

	ci::ColorA JuliaSet::getColorHigh(
	  const lrc::Complex<HighPrecision> &coord, int64_t iters,
	  const ColorPalette &palette,
	  const std::function<ci::ColorA(const lrc::Complex<HighPrecision> &, int64_t,
									 const ColorPalette &)> &colorFunc) const {
		if (coord.real() * coord.real() + coord.imag() * coord.imag() < 4)
			return {0, 0, 0, 1};
		return colorFunc(coord, iters, palette);
	}
} // namespace frac
