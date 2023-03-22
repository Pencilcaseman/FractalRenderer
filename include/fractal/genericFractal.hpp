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

		/// Return an integer where the presence of a 1 or 0 at index i represents whether
		/// optimisation i is valid for this fractal
		/// \return Unsigned 64-bit integer
		LIBRAPID_NODISCARD virtual size_t supportedOptimisations() const;

		LIBRAPID_NODISCARD virtual std::string name() const;

		LIBRAPID_NODISCARD virtual std::unordered_map<std::string, coloring::ColorFuncLow>
		getLowPrecColoringAlgorithms() const = 0;

		LIBRAPID_NODISCARD virtual std::unordered_map<std::string,
													  coloring::ColorFuncHigh>
		getHighPrecColoringAlgorithms() const = 0;

		/// Iterate over a complex-valued coordinate and return the value at which the
		/// coordinate exceeds the given threshold or reaches the desired number of
		/// iterations. The return value also includes the number of iterations it took
		/// to reach the return coordinate.
		/// \param coord The initial complex-valued coordinate
		/// \return <iterations, resulting coordinate (low precision)>
		LIBRAPID_NODISCARD virtual std::pair<int64_t, lrc::Complex<LowPrecision>>
		iterCoordLow(const lrc::Complex<LowPrecision> &coord) const = 0;

		/// See iterCoordLow(const lrc::Complex<LowPrecision> &coord) const
		/// \param coord The initial complex-valued coordinate
		/// \return <iterations, resulting coordinate (high precision)>
		/// \see iterCoordLow(const lrc::Complex<LowPrecision> &coord) const
		LIBRAPID_NODISCARD virtual std::pair<int64_t, lrc::Complex<HighPrecision>>
		iterCoordHigh(const lrc::Complex<HighPrecision> &coord) const = 0;

		LIBRAPID_NODISCARD virtual ci::ColorA
		getColorLow(const lrc::Complex<LowPrecision> &coord, int64_t iters,
					const ColorPalette &palette,
					const coloring::ColorFuncLow &colorFunc) const;

		LIBRAPID_NODISCARD virtual ci::ColorA
		getColorHigh(const lrc::Complex<HighPrecision> &coord, int64_t iters,
					 const ColorPalette &palette,
					 const coloring::ColorFuncHigh &colorFunc) const;

	protected:
		RenderConfig m_renderConfig;
	};
} // namespace frac
