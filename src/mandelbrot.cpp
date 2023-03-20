#include <fractal/fractal.hpp>

namespace frac {
	Mandelbrot::Mandelbrot(const RenderConfig &config) : Fractal(config) {}

	/*
	 * Note that, since this class will be used polymorphically with other classes,
	 * these two functions must be implemented separately and cannot be templated, as the
	 * compiler would error when trying to identify which function to call. Additionally,
	 * splitting the functions in this way allows for more targeted optimisations to be
	 * made in some cases.
	 */

	size_t Mandelbrot::supportedOptimisations() const {
		// Outlining is proven for the Mandelbrot set
		return optimisations::OUTLINE_OPTIMISATION;
	}

	std::string Mandelbrot::name() const { return "Mandelbrot"; }

	std::unordered_map<std::string, coloring::ColorFuncLow>
	Mandelbrot::getLowPrecColoringAlgorithms() const {
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
	Mandelbrot::getHighPrecColoringAlgorithms() const {
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
	Mandelbrot::iterCoordLow(const lrc::Complex<LowPrecision> &coord) const {
		LowPrecision re_0 = lrc::real(coord); // Real component (initial)
		LowPrecision im_0 = lrc::imag(coord); // Imaginary component (initial)
		LowPrecision re = 0, im = 0;
		LowPrecision tmp; // Temporary variable for use in the calculation
		int64_t iteration = 0;

		// Bail when larger than this
		double bailout = Fractal::m_renderConfig.bail;

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

		while (re * re + im * im <= bailout &&
			   iteration < Fractal::m_renderConfig.maxIters) {
			tmp = re * re - im * im + re_0;
			im	= 2 * re * im + im_0;
			re	= tmp;
			++iteration;
		}

		return {iteration, lrc::Complex<HighPrecision>(re, im)};
	}

	ci::ColorA Mandelbrot::getColorLow(
	  const lrc::Complex<LowPrecision> &coord, int64_t iters, const ColorPalette &palette,
	  const std::function<ci::ColorA(const lrc::Complex<LowPrecision> &, int64_t,
									 const ColorPalette &)> &colorFunc) const {
		if (coord.real() * coord.real() + coord.imag() * coord.imag() < 4)
			return {0, 0, 0, 1};
		return colorFunc(coord, iters, palette);
	}

	ci::ColorA Mandelbrot::getColorHigh(
	  const lrc::Complex<HighPrecision> &coord, int64_t iters,
	  const ColorPalette &palette,
	  const std::function<ci::ColorA(const lrc::Complex<HighPrecision> &, int64_t,
									 const ColorPalette &)> &colorFunc) const {
		if (coord.real() * coord.real() + coord.imag() * coord.imag() < 4)
			return {0, 0, 0, 1};
		return colorFunc(coord, iters, palette);
	}
} // namespace frac
