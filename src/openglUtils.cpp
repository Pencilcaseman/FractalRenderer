#include <fractal/fractal.hpp>

namespace frac::glu {
	void drawLine(const lrc::Vec2f &p1, const lrc::Vec2f &p2, float thickness) {
		ci::vec3 translation({p1.x(), p1.y(), 0});
		float rotation	 = lrc::atan2(p2.y() - p1.y(), p2.x() - p1.x());
		float lineLength = static_cast<float>((p2 - p1).mag());

		ci::gl::pushMatrices();
		ci::gl::translate(translation);
		ci::gl::rotate(rotation);

		// Line segment
		ci::gl::drawSolidRect(ci::Rectf({0.f, -thickness / 2.f}, {lineLength, thickness / 2}));

		// Draw two circles to make it look nice :)
		ci::gl::drawSolidCircle({0, 0}, thickness / 2);
		ci::gl::drawSolidCircle({lineLength, 0}, thickness / 2);

		ci::gl::popMatrices();
	}

	void drawStrokedRectangle(const lrc::Vec2f &topLeft, const lrc::Vec2f &bottomRight,
							  float thickness) {
		// Draw each edge using drawLine, as it allows a thickness to be specified
		drawLine(topLeft, {bottomRight.x(), topLeft.y()}, thickness);
		drawLine({bottomRight.x(), topLeft.y()}, bottomRight, thickness);
		drawLine(bottomRight, {topLeft.x(), bottomRight.y()}, thickness);
		drawLine({topLeft.x(), bottomRight.y()}, topLeft, thickness);
	}
} // namespace frac::glu
