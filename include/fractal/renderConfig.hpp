#pragma once

namespace frac {
	/// Represents the state of a render box
	enum class RenderBoxState {
		None,	   // Not yet assigned a state
		Queued,	   // Queued to be rendered
		Rendering, // Currently being rendered
		Rendered   // Rendered and ready to be written to the image
	};

	/// Stores the pixel-space coordinates of a region to render
	struct RenderBox {
		lrc::Vec2i topLeft;
		lrc::Vec2i dimensions;
		RenderBoxState state = RenderBoxState::None;
	};

	struct RenderConfig {
		int64_t numThreads; // Number of threads to render on (max)
		int64_t maxIters;	// Largest number of iterations to allow
		int64_t precision;		// Precision (in bits) of floating point tyeps used for arithmetic
		LowPrecision bail;	// Bailout value
		int antiAlias;		// Anti-aliasing factor -- 1 = no anti-aliasing

		lrc::Vec2i imageSize; // Size of the image to render
		lrc::Vec2i boxSize;	  // Size of sub-regions to render (see RenderBox)

		lrc::Vec<HighPrecision, 2> fracTopLeft;		 // The fractal-space center of the image
		lrc::Vec<HighPrecision, 2> fracSize;		 // The width and height of the fractal space
		lrc::Vec<HighPrecision, 2> originalFracSize; // Original size for zoom factor calculation

		ColorPalette palette; // The palette to use for rendering the fractal
	};
} // namespace frac
