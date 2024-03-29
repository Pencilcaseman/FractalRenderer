#pragma once

namespace frac::coloring {
	using ColorFuncLow	= std::function<ci::ColorA(const lrc::Complex<LowPrecision> &,
												   int64_t, const ColorPalette &)>;
	using ColorFuncHigh = std::function<ci::ColorA(const lrc::Complex<HighPrecision> &,
												   int64_t, const ColorPalette &)>;

	template<typename Precision>
	ci::ColorA logarithmicScaling(const lrc::Complex<Precision> &coord, int64_t iters,
								  const ColorPalette &palette) {
		using Col = ColorPalette::ColorType;

		double s1 = (double)iters -
					lrc::log2(lrc::log2((float)coord.real() * (float)coord.real() +
										(float)coord.imag() * (float)coord.imag())) +
					4;
		Col color = 0.5 + 0.5 * lrc::cos(3.0 + s1 * 0.15 + lrc::Vec3d(0.0, 0.6, 1.0));
		return {(float)color.x(), (float)color.y(), (float)color.z(), 1};
	}

	template<typename Precision>
	ci::ColorA palettedLogarithmicScaling(const lrc::Complex<Precision> &coord,
										  int64_t iters, const ColorPalette &palette) {
		using Col = ColorPalette::ColorType;

		float escapeTime = (float)coord.real() * (float)coord.real() +
							(float)coord.imag() * (float)coord.imag();
		float smoothValue = iters + 1 - log2(log2(escapeTime));

		float s1 = smoothValue + 4;
		Col color1 = palette[static_cast<size_t>(s1) % palette.size()];
		Col color2 = palette[(static_cast<size_t>(s1) + 1) % palette.size()];
		Col merged = ColorPalette::merge(color1, color2, lrc::mod(s1, 1.0f));
		return {merged.x(), merged.y(), merged.z(), 1};
	}

	template<typename Precision>
	ci::ColorA steppedGradients(const lrc::Complex<Precision> &coord, int64_t iters,
								const ColorPalette &palette) {
		using Col = ColorPalette::ColorType;
		return {(iters % 2) / 2.f, (iters % 3) / 3.f, (iters % 7) / 7.f, 1};
	}

	template<typename Precision>
	ci::ColorA fixedIterPalette(const lrc::Complex<Precision> &coord, int64_t iters,
								const ColorPalette &palette) {
		auto col = palette[iters % palette.size()];
		return {col.x(), col.y(), col.z(), 1};
	}
} // namespace frac::coloring
