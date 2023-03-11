#include <fractal/fractal.hpp>

namespace frac {
	Fractal::Fractal(const RenderConfig &config) : m_renderConfig(config) {}

	void Fractal::updateRenderConfig(const RenderConfig &config) { m_renderConfig = config; }

	ci::ColorA Fractal::getColorLow(const lrc::Complex<LowPrecision> &coord, int64_t iters) const {
		using Col = ColorPalette::ColorType;

		float logZN		= lrc::log(lrc::abs(lrc::Complex<float>(coord.real(), coord.imag()))) / 2;
		float nu		= lrc::log(logZN / lrc::LN2) / lrc::LN2;
		float iteration = static_cast<float>(iters) + 1 - nu;
		const auto &palette = m_renderConfig.palette;
		Col color1			= palette[static_cast<size_t>(iteration) % palette.size()];
		Col color2			= palette[(static_cast<size_t>(iteration) + 1) % palette.size()];
		Col merged			= ColorPalette::merge(color1, color2, lrc::mod(iteration, 1.0f));
		return {merged.x(), merged.y(), merged.z(), 1};

		// Nice gradient
		// double s1 = (double)iters -
		// 			lrc::log2(lrc::log2((float)coord.real() * (float)coord.real() +
		// 								(float)coord.imag() * (float)coord.imag())) +
		// 			4;
		// Col color = 0.5 + 0.5 * lrc::cos(3.0 + s1 * 0.15 + lrc::Vec3d(0.0, 0.6, 1.0));
		// return {(float)color.x(), (float)color.y(), (float)color.z(), 1};

		// Cool stepped gradients
		// return {(iters % 10) / 10.f, 0, 0, 1};
		// return {(iters % 11) / 11.f, (iters % 23) / 23.f, (iters % 31) / 31.f, 1};
		// return {(iters % 2) / 2.f, (iters % 3) / 3.f, (iters % 7) / 7.f, 1};
	}

	ci::ColorA Fractal::getColorHigh(const lrc::Complex<HighPrecision> &coord,
									 int64_t iters) const {
		using Col = ColorPalette::ColorType;

		float logZN		= lrc::log(lrc::abs(lrc::Complex<float>(coord.real(), coord.imag()))) / 2;
		float nu		= lrc::log(logZN / lrc::LN2) / lrc::LN2;
		float iteration = static_cast<float>(iters) + 1 - nu;
		const auto &palette = m_renderConfig.palette;
		Col color1			= palette[static_cast<size_t>(iteration) % palette.size()];
		Col color2			= palette[(static_cast<size_t>(iteration) + 1) % palette.size()];
		Col merged			= ColorPalette::merge(color1, color2, lrc::mod(iteration, 1.0f));
		return {merged.x(), merged.y(), merged.z(), 1};

		// Nice gradient
		// double s1 = (double)iters -
		// 			lrc::log2(lrc::log2((float)coord.real() * (float)coord.real() +
		// 								(float)coord.imag() * (float)coord.imag())) +
		// 			4;
		// Col color = 0.5 + 0.5 * lrc::cos(3.0 + s1 * 0.15 + lrc::Vec3d(0.0, 0.6, 1.0));
		// return {(float)color.x(), (float)color.y(), (float)color.z(), 1};

		// Cool stepped gradients
		// return {(iters % 10) / 10.f, 0, 0, 1};
		// return {(iters % 11) / 11.f, (iters % 23) / 23.f, (iters % 31) / 31.f, 1};
		// return {(iters % 2) / 2.f, (iters % 3) / 3.f, (iters % 7) / 7.f, 1};
	}
} // namespace frac
