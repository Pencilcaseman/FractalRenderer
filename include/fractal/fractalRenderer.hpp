#pragma once

namespace frac {
	class FractalRenderer {
	public:
		FractalRenderer(const json &config);
		~FractalRenderer();

		void setConfig(const json &config);

		void stopRender();

		void moveFractalCorner(const lrc::Vec<HighPrecision, 2> &topLeft,
							   const lrc::Vec<HighPrecision, 2> &size);

		void moveFractalCenter(const lrc::Vec<HighPrecision, 2> &center,
							   const lrc::Vec<HighPrecision, 2> &size);

		/// Render the fractal into the fractal surface, and copy that to the
		/// fractal surface to be drawn. This will be executed on a separate thread
		/// in order to keep the UI updating
		void renderFractal();

		/// Render a sub-section of the fractal, defined by the \p box variable. This is intended
		/// to be used within the call queue to render multiple sections in parallel
		/// \param box
		void renderBox(const RenderBox &box, int64_t boxIndex = -1);

	private:
		RenderConfig m_renderConfig;		   // The settings for the fractal renderer
		ci::Surface m_fractalSurface;		   // The surface that the fractal is rendered to
		ci::gl::Texture2dRef m_fractalTexture; // The image to be rendered to the screen
		lrc::Vec2i m_mousePos;				   // The current position of the mouse in the window
		json m_settings;					   // The settings for the fractal
		std::unique_ptr<Fractal> m_fractal;	   // The fractal to render
		ThreadPool m_threadPool;			   // Pool for render threads

		std::vector<RenderBox> m_renderBoxes; // The state of each render box
		bool m_haltRender = false;			  // Used to gracefully stop the render threads
	};
} // namespace frac
