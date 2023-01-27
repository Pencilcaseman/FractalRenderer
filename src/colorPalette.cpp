#include <fractal/fractal.hpp>

namespace frac {
	void ColorPalette::addColor(const ColorType &color) {
		m_colors.push_back(color);
		FRAC_LOG(fmt::format("Adding Color: {} {} {} {}", color.x(), color.y(), color.z(), color.w()));
	}

	size_t ColorPalette::size() const { return m_colors.size(); }

	const ColorPalette::ColorType &ColorPalette::operator[](size_t index) const {
		if (index > m_colors.size())
			FRAC_ERROR(fmt::format(
			  "Index {} out of bounds for ColorPalette with size {}", index, m_colors.size()));
		return m_colors[index];
	}

	ColorPalette::ColorType &ColorPalette::operator[](size_t index) {
		if (index > m_colors.size())
			FRAC_ERROR(fmt::format(
			  "Index {} out of bounds for ColorPalette with size {}", index, m_colors.size()));
		return m_colors[index];
	}

	ColorPalette::ColorType ColorPalette::merge(const ColorType &a, const ColorType &b, float t) {
		return a * (1 - t) + b * t;
	}
} // namespace frac
