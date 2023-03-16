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

		/// Append a new colour to the palette
		/// \param color The colour to add
		void addColor(const ColorType &color);

		/// Return the number of colours in the palette
		/// \return The number of colours in the palette
		LIBRAPID_NODISCARD size_t size() const;

		/// Indexing operator (const)
		/// \param index The index of the colour to return
		/// \return The colour at the given index
		const ColorType &operator[](size_t index) const;

		/// Indexing operator (non-const)
		/// \param index The index of the colour to return
		/// \return The colour at the given index
		ColorType &operator[](size_t index);

		/// Linearly interpolate between two colours
		/// \param a First colour
		/// \param b Second colour
		/// \param t Interpolation factor
		/// \return The interpolated colour
		static ColorType merge(const ColorType &a, const ColorType &b, float t);

	private:
		std::vector<ColorType> m_colors;
	};
} // namespace frac
