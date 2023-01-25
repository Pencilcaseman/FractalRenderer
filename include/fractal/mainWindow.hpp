#pragma once

namespace frac {
	class MainWindow : public ci::app::App {
	public:
		/// Nothing passed to the constructor
		MainWindow() = default;

		/// Set up the window, configure ImGui and initialize the fractal rendering surfaces
		void setup() override;

		void closeWindow();

		// Run on shutdown
		// void cleanup() override {  }

		/// Called every frame to render the frame
		void draw() override;

		/// Draw the UI
		void drawImGui();

		void regenerateSurfaces();

		/// Render the fractal into the fractal surface, and copy that to the
		/// fractal surface to be drawn. This will be executed on a separate thread
		/// in order to keep the UI updating
		void renderFractal();

		/// Render a sub-section of the fractal, defined by the \p box variable. This is intended
		/// to be used within the call queue to render multiple sections in parallel
		/// \param box
		void renderBox(const RenderBox &box);

		/// Callback for mouse movement (this does not include mouse clicks or drags)
		/// \param event The mouse event
		void mouseMove(ci::app::MouseEvent event) override;

		RenderConfig m_renderConfig;			 // The settings for the fractal renderer
		ci::Surface m_fractalSurface;			 // The surface that the fractal is rendered to
		ci::gl::Texture2dRef m_fractalTexture;	 // The image to be rendered to the screen
		ci::Font m_font = ci::Font("Arial", 24); // The font to use for rendering text
		lrc::Vec2i m_mousePos;					 // The current position of the mouse in the window
		json m_settings;						 // The settings for the fractal
		std::unique_ptr<Fractal> m_fractal;		 // The fractal to render
		BS::thread_pool m_threadPool;			 // Pool for render threads

		bool m_haltRender; // Used to gracefully stop the render threads
	};
} // namespace frac
