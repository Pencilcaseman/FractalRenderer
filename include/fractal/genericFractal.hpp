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

		virtual int64_t iterCoord(const lrc::Complex<HighPrecision> &coord) const = 0;

	protected:
		RenderConfig m_renderConfig;
	};
} // namespace frac
