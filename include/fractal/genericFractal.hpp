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

		virtual std::pair<int64_t, lrc::Complex<HighPrecision>>
		iterCoord(const lrc::Complex<HighPrecision> &coord) const = 0;

		virtual ci::ColorA getColor(const lrc::Complex<HighPrecision> &coord, int64_t iters) const;

	protected:
		RenderConfig m_renderConfig;
	};
} // namespace frac
