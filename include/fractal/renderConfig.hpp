#pragma once

namespace frac {
	/// Stores the pixel-space coordinates of a region to render
	struct RenderBox {
		lrc::Vec2i topLeft;
		lrc::Vec2i dimensions;
	};

	struct RenderConfig {
		int64_t numThreads; // Number of threads to render on (max)
		int64_t maxIters;	// Largest number of iterations to allow
		int antiAlias;		// Anti-aliasing factor -- 1 = no anti-aliasing
		LowPrecision bail;	// Bailout value

		lrc::Vec2i imageSize; // Size of the image to render
		lrc::Vec2i boxSize;	  // Size of sub-regions to render (see RenderBox)

		lrc::Vec<HighPrecision, 2> fracTopLeft; // The fractal-space center of the image
		lrc::Vec<LowPrecision, 2> fracSize;	   // The width and height of the fractal space
	};
} // namespace frac
