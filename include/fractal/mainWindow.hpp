#pragma once

namespace frac {
	class MainWindow : public ci::app::App {
	public:
		/// Nothing passed to the constructor
		MainWindow() = default;

		void configureSettings();

		/// Set up the main window, making sure it's the right size and that the frame rates
		/// are set correctly
		void configureWindow();

		/// Configure ImGui, setting up the style and enabling docking
		void configureImGui();

		/// Set up the window, configure ImGui and initialize the fractal rendering surfaces
		void setup() override;

		void stopRender();

		// Run on shutdown
		void cleanup() override;

		void drawFractal();

		void outlineRenderBoxes();

		/// Called every frame to render the frame
		void draw() override;

		/// Draw the UI
		void drawImGui();

		void moveFractalCorner(const lrc::Vec<HighPrecision, 2> &topLeft,
							   const lrc::Vec<HighPrecision, 2> &size);

		void moveFractalCenter(const lrc::Vec<HighPrecision, 2> &center,
							   const lrc::Vec<HighPrecision, 2> &size);

		void zoomFractal(const lrc::Vec2i &pixTopLeft,
						 const lrc::Vec2i &pixBottomRight);

		/// Render the fractal into the fractal surface, and copy that to the
		/// fractal surface to be drawn. This will be executed on a separate
		/// thread in order to keep the UI updating
		void renderFractal();

		/// Callback for mouse movement (this does not include mouse clicks or
		/// drags) \param event The mouse event
		void mouseMove(ci::app::MouseEvent event) override;

		/// Callback for mouse clicks
		/// \param event The mouse event
		void mouseDown(ci::app::MouseEvent event) override;

		/// Callback for mouse drags
		/// \param event The mouse event
		void mouseDrag(ci::app::MouseEvent event) override;

		/// Callback for mouse releases
		/// \param event The mouse event
		void mouseUp(ci::app::MouseEvent event) override;

		void keyDown(ci::app::KeyEvent event) override;

	private:
		template<typename T>
		static lrc::Vec<T, 2> aspectCorrectedBox(const lrc::Vec<T, 2> &p1,
												 const lrc::Vec<T, 2> &p2,
												 float aspectRatio) {
			lrc::Vec<T, 2> correctedBox;
			lrc::Vec<T, 2> delta = p2 - p1;
			if (delta.y() < delta.x() / aspectRatio)
				correctedBox = {delta.x(), delta.x() / aspectRatio};
			else
				correctedBox = {delta.y() * aspectRatio, delta.y()};
			return correctedBox;
		}

		void drawZoomBox(const lrc::Vec2f &start, const lrc::Vec2f &end) const;

		FractalRenderer m_renderer;				 // The fractal renderer
		ci::gl::Texture2dRef m_fractalTexture;	 // The fractal texture
		ci::Font m_font = ci::Font("Arial", 24); // The font to use for rendering text
		lrc::Vec2i m_mousePos;	   // The current position of the mouse in the window
		lrc::Vec2i m_mouseDownPos; // The position of the mouse when it was clicked
		bool m_mouseDown = false;  // Whether the mouse is currently down

		bool m_drawingZoomBox = false;
		bool m_showZoomBox	  = false;
		bool m_moveZoomBox	  = false;
		lrc::Vec2i m_zoomBoxStart;
		lrc::Vec2i m_zoomBoxEnd;

		// Values used for ImGui input fields
		std::string m_fineMovementRe;
		std::string m_fineMovementIm;
		std::string m_fineMovementZoom;
	};
} // namespace frac
