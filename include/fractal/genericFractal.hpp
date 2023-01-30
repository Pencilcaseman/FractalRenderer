#pragma once

namespace frac {
	class Fractal {
	public:
		/// Constructor taking a RenderConfig object
		/// \param config
		explicit Fractal(const RenderConfig &config);
		Fractal(const Fractal &)			= delete;
		Fractal(Fractal &&)					= delete;
		Fractal &operator=(const Fractal &) = delete;
		Fractal &operator=(Fractal &&)		= delete;
		virtual ~Fractal()					= default;

		virtual void updateRenderConfig(const RenderConfig &config);

		virtual std::pair<int64_t, lrc::Complex<LowPrecision>>
		iterCoordLow(const lrc::Complex<LowPrecision> &coord) const = 0;

		virtual std::pair<int64_t, lrc::Complex<HighPrecision>>
		iterCoordHigh(const lrc::Complex<HighPrecision> &coord) const = 0;

		virtual ci::ColorA getColorLow(const lrc::Complex<LowPrecision> &coord,
									   int64_t iters) const;

		virtual ci::ColorA getColorHigh(const lrc::Complex<HighPrecision> &coord,
										int64_t iters) const;

	protected:
		RenderConfig m_renderConfig;
	};
} // namespace frac
