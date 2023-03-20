#pragma once

#include <fractal/genericFractal.hpp>

namespace frac {
	/*
	 * No need to document this file, since the class inherits from a generic fractal
	 * class and does not implement any novel functions.
	 */

	class NewtonFractal : public Fractal {
	public:
		/// Constructor taking a RenderConfig object
		/// \param config RenderConfig object
		explicit NewtonFractal(const RenderConfig &config);
		NewtonFractal(const NewtonFractal &)			= delete;
		NewtonFractal(NewtonFractal &&)					= delete;
		NewtonFractal &operator=(const NewtonFractal &) = delete;
		NewtonFractal &operator=(NewtonFractal &&)		= delete;

		~NewtonFractal() override = default;

		size_t supportedOptimisations() const override;

		LIBRAPID_NODISCARD
		std::unordered_map<std::string, coloring::ColorFuncLow>
		getLowPrecColoringAlgorithms() const override;

		LIBRAPID_NODISCARD
		std::unordered_map<std::string, coloring::ColorFuncHigh>
		getHighPrecColoringAlgorithms() const override;

		template<typename T>
		lrc::Complex<T> f(const lrc::Complex<T> &z) const {
			// return lrc::pow(z, 3) - 1;

			// a^3 + i 3 a^2 b - 3 a b^2 - i b^3 - 1
			auto real  = z.real();
			auto imag  = z.imag();
			auto real2 = real * real;
			auto imag2 = imag * imag;
			return lrc::Complex<T>(1, 0) * real2 * real +
				   lrc::Complex<T>(0, 3) * real2 * imag -
				   lrc::Complex<T>(3, 0) * real * imag2 -
				   lrc::Complex<T>(0, 1) * imag2 * imag - 1;
		}

		template<typename T>
		lrc::Complex<T> df(const lrc::Complex<T> &z) const {
			// return 3 * lrc::pow(z, 2);
			// a^2 + i 2 a b - b^2
			auto real = z.real();
			auto imag = z.imag();
			return lrc::Complex<T>(3, 0) * real * real +
				   lrc::Complex<T>(0, 6) * real * imag -
				   lrc::Complex<T>(3, 0) * imag * imag;
		}

		LIBRAPID_NODISCARD std::pair<int64_t, lrc::Complex<LowPrecision>>
		iterCoordLow(const lrc::Complex<LowPrecision> &coord) const override;

		LIBRAPID_NODISCARD std::pair<int64_t, lrc::Complex<HighPrecision>>
		iterCoordHigh(const lrc::Complex<HighPrecision> &coord) const override;
	};
} // namespace frac
