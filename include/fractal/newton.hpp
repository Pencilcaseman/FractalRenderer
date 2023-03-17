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

		template<typename T>
		lrc::Complex<T> f(const lrc::Complex<T> &z) const {
			return lrc::pow(z, 3) - 1;
		}

		template<typename T>
		lrc::Complex<T> df(const lrc::Complex<T> &z) const {
			return 3 * lrc::pow(z, 2);
		}

		LIBRAPID_NODISCARD std::pair<int64_t, lrc::Complex<LowPrecision>>
		iterCoordLow(const lrc::Complex<LowPrecision> &coord) const override;

		LIBRAPID_NODISCARD std::pair<int64_t, lrc::Complex<HighPrecision>>
		iterCoordHigh(const lrc::Complex<HighPrecision> &coord) const override;
	};
} // namespace frac
