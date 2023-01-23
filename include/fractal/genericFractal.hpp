#pragma once

namespace frac {
	class Fractal {
	public:
		Fractal()							= default;
		Fractal(const Fractal &)			= delete;
		Fractal(Fractal &&)					= delete;
		Fractal &operator=(const Fractal &) = delete;
		Fractal &operator=(Fractal &&)		= delete;

		virtual lrc::Complex<HighPrecision>
		iterPixelRaw(const lrc::Complex<HighPrecision> &c) const = 0;

		virtual int64_t
		iterPixel(const lrc::Complex<HighPrecision> &c) const = 0;

	private:
	};
} // namespace frac
