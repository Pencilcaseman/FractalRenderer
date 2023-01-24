#include <fractal/fractal.hpp>

namespace frac {
	Fractal::Fractal(const RenderConfig &config) : m_renderConfig(config) {}

	ci::ColorA Fractal::getColor(const lrc::Complex<HighPrecision> &coord, int64_t iters) const {
		// Return a pixel color from a given coordinate and number of iterations
		float red = lrc::abs(coord);
		float green = lrc::abs(coord);
		float blue = lrc::abs(coord);

		lrc::Vec<float, 3> col(red, green, blue);
		col = col.norm();

		return ci::ColorA(col.x(), col.y(), col.z(), 1);
	}
} // namespace frac
