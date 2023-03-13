#pragma once

namespace frac::glu {
	/// Draw a line from p1 to p2 with a given thickness. The line has a circle drawn at each end
	/// to make it look nice.
	/// \param p1 Line start
	/// \param p2 Line end \param
	/// thickness Line thickness
	void drawLine(const lrc::Vec2f &p1, const lrc::Vec2f &p2, float thickness = 1);

	/// Draw a stroked rectangle with a given thickness -- note this draws the EDGES of
	/// the rectangle, and does not fill the inside \param topLeft Top left corner of the
	/// rectangle \param bottomRight Bottom right corner of the rectangle \param thickness
	/// Line thickness
	void drawStrokedRectangle(const lrc::Vec2f &topLeft, const lrc::Vec2f &bottomRight,
							  float thickness = 1);

	void drawCross(const lrc::Vec2f &center, float radius, float thickness = 1);
} // namespace frac::glu
