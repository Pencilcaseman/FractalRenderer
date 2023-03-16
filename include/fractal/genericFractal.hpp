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

		/// Configure a new set of options for the fractal
		/// \param config The new RenderConfig to use
		virtual void updateRenderConfig(const RenderConfig &config);

		/// Iterate over a complex-valued coordinate and return the value at which the
		/// coordinate exceeds the given threshold or reaches the desired number of
		/// iterations. The return value also includes the number of iterations it took
		/// to reach the return coordinate.
		/// \param coord The initial complex-valued coordinate
		/// \return <iterations, resulting coordinate (low precision)>
		virtual std::pair<int64_t, lrc::Complex<LowPrecision>>
		iterCoordLow(const lrc::Complex<LowPrecision> &coord) const = 0;

		/// See iterCoordLow(const lrc::Complex<LowPrecision> &coord) const
		/// \param coord The initial complex-valued coordinate
		/// \return <iterations, resulting coordinate (high precision)>
		/// \see iterCoordLow(const lrc::Complex<LowPrecision> &coord) const
		virtual std::pair<int64_t, lrc::Complex<HighPrecision>>
		iterCoordHigh(const lrc::Complex<HighPrecision> &coord) const = 0;

		/// Apply the colouring algorithm given the result of an `iterCoord` call
		/// \param coord Resulting coordinate
		/// \param iters Number of iterations
		/// \return Colour of the point
		virtual ci::ColorA getColorLow(const lrc::Complex<LowPrecision> &coord,
									   int64_t iters) const;

		/// See getColorLow(const lrc::Complex<LowPrecision> &coord, int64_t iters) const
		/// \param coord Resulting coordinate
		/// \param iters Number of iterations
		/// \return Colour of the point
		/// \see getColorLow(const lrc::Complex<LowPrecision> &coord, int64_t iters) const
		virtual ci::ColorA getColorHigh(const lrc::Complex<HighPrecision> &coord,
										int64_t iters) const;

	protected:
		RenderConfig m_renderConfig;
	};
} // namespace frac
