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
		bool draftRender;
		int64_t draftInc;
		RenderBoxState state = RenderBoxState::None;
		double renderTime	 = 0;
	};

	/// Information about the time taken to render a box
	struct RenderBoxTimeStats {
		double min	  = 0;
		double max	  = 0;
		double average = 0;
		double remainingTime = 0;
	};

	/// Contains all the information required to define an image (not including the
	/// fractal type and colouring algorithm)
	struct RenderConfig {
		int64_t numThreads; // Number of threads to render on (max)
		int64_t maxIters;	// Largest number of iterations to allow
		int64_t precision;	// Precision of floating point types used for arithmetic
		LowPrecision bail;	// Bailout value
		int64_t antiAlias;	// Anti-aliasing factor -- 1 = no anti-aliasing

		lrc::Vec2i imageSize; // Size of the image to render
		lrc::Vec2i boxSize;	  // Size of sub-regions to render (see RenderBox)

		lrc::Vec<HighPrecision, 2> fracTopLeft; // The fractal-space center of the image
		lrc::Vec<HighPrecision, 2> fracSize; // The width and height of the fractal space
		lrc::Vec<HighPrecision, 2> originalFracSize; // Original size

		ColorPalette palette; // The palette to use for rendering the fractal

		bool draftRender; // Whether to render the fractal in draft mode
		int64_t draftInc; // Increment for draft rendering
	};
} // namespace frac
