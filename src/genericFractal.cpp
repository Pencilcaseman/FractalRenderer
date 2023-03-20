#include <fractal/fractal.hpp>

namespace frac {
	Fractal::Fractal(const RenderConfig &config) : m_renderConfig(config) {}

	void Fractal::updateRenderConfig(const RenderConfig &config) {
		m_renderConfig = config;
	}

	size_t Fractal::supportedOptimisations() const {
		return 0; // By default, assume no optimisations are valid
	}

	ci::ColorA Fractal::getColorLow(const lrc::Complex<LowPrecision> &coord,
									int64_t iters, const ColorPalette &palette,
									const coloring::ColorFuncLow &colorFunc) const {
		return colorFunc(coord, iters, palette);
	}

	ci::ColorA Fractal::getColorHigh(const lrc::Complex<HighPrecision> &coord,
									 int64_t iters, const ColorPalette &palette,
									 const coloring::ColorFuncHigh &colorFunc) const {
		return colorFunc(coord, iters, palette);
	}
} // namespace frac
