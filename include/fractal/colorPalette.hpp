#pragma once

namespace frac {
	class ColorPalette {
	public:
		using ColorType = lrc::Vec<float, 4>;

		ColorPalette()								  = default;
		ColorPalette(const ColorPalette &)			  = default;
		ColorPalette(ColorPalette &&)				  = default;
		ColorPalette &operator=(const ColorPalette &) = default;
		ColorPalette &operator=(ColorPalette &&)	  = default;

		void addColor(const ColorType &color);

		LIBRAPID_NODISCARD size_t size() const;

		const ColorType &operator[](size_t index) const;
		ColorType &operator[](size_t index);

		static ColorType merge(const ColorType &a, const ColorType &b, float t);
	private:
		std::vector<ColorType> m_colors;
	};
} // namespace frac
