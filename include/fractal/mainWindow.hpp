#pragma once

namespace frac {
	class MainWindow : public ci::app::App {
	public:
		/// Nothing passed to the constructor
		MainWindow() = default;

		void configureSettings();

		/// Set up the main window, making sure it's the right size and that the frame
		/// rates are set correctly
		void configureWindow();

		/// Configure ImGui, setting up the style and enabling docking
		void configureImGui();

		void configureFractalDefault();

		/// Set up the window, configure ImGui and initialize the fractal rendering
		/// surfaces
		void setup() override;

		/// Halt all render threads and wait for them to join main
		void stopRender();

		void appendConfigToHistory();

		void setFractalType(const std::string &name);

		std::vector<std::tuple<lrc::Vec2f, lrc::Vec2f, HistoryNode *>>
		getHistoryFrameLocations() const;

		void undoLastMove();
		void redoLastMove();

		// Run on shutdown
		void cleanup() override;

		/// Render the fractal to the screen (from the FractalRenderer surface)
		void drawFractal();

		/// Outline each render box (if active) to show their current states
		void outlineRenderBoxes();

		/// Called every frame
		void draw() override;

		/// Draw the UI
		void drawImGui();

		/// Draw the history window
		void drawHistory();

		/// Update the most recent history item with the current fractal configuration
		/// and surface
		void updateHistoryItem();

		template<typename T>
		LIBRAPID_NODISCARD lrc::Vec<T, 2>
		screenToImageSpace(const lrc::Vec<T, 2> &coord) {
			double scalingFactor =
			  (double)m_renderer.config().imageSize.y() / (double)getWindowHeight();
			return coord * scalingFactor;
		}

		template<typename T>
		LIBRAPID_NODISCARD lrc::Vec<T, 2>
		imageToScreenSpace(const lrc::Vec<T, 2> &coord) {
			double scalingFactor =
			  (double)getWindowHeight() / (double)m_renderer.config().imageSize.y();
			return coord * scalingFactor;
		}

		/// Move the top left corner of the fractal and set a new size
		/// \param topLeft Top-left corner (complex coordinate)
		/// \param size Size of the fractal (Re, Im)
		void moveFractalCorner(const lrc::Vec<HighPrecision, 2> &topLeft,
							   const lrc::Vec<HighPrecision, 2> &size);

		/// Set the center of the fractal and the dimensions -- see moveFractalCorner
		/// \param center Top-left corner
		/// \param size Size of fractal
		/// \see moveFractalCorner
		void moveFractalCenter(const lrc::Vec<HighPrecision, 2> &center,
							   const lrc::Vec<HighPrecision, 2> &size);

		/// Advanced zooming method -- given pixel coordinates for the top left and
		/// bottom right of the new area, perform the following:
		/// 1. Copy existing pixels in the specified region to a buffer
		/// 2. Regenerate the surface
		/// 3. Copy the buffer to the fractal surface
		/// 4. Reconfigure the fractal renderer
		/// 5. Trigger another fractal render
		/// \param pixTopLeft
		/// \param pixBottomRight
		void zoomFractal(const lrc::Vec2i &pixTopLeft, const lrc::Vec2i &pixBottomRight);

		/// Render the fractal into the fractal surface, and copy that to the
		/// fractal surface to be drawn. This will be executed on a separate
		/// thread in order to keep the UI updating
		void renderFractal(bool amendHistory = true);

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

		/// Callback for mouse wheel events
		/// \param event The mouse event
		void mouseWheel(ci::app::MouseEvent event) override;

		/// Callback for when a key is pressed, including the modifiers (shift, ctrl,
		/// etc) \param event The key event
		void keyDown(ci::app::KeyEvent event) override;

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

		/// Draw a zoom box at a given point. This includes a transparent box
		/// surrounded by a solid rectangle with a cross in the middle. \param start
		/// The top left corner of the box \param end The bottom right corner of the
		/// box
		void drawZoomBox(const lrc::Vec2f &start, const lrc::Vec2f &end) const;

	private:
		FractalRenderer m_renderer;				 // The fractal renderer
		ci::gl::Texture2dRef m_fractalTexture;	 // The fractal texture
		ci::Font m_font = ci::Font("Arial", 24); // The font to use for rendering text
		lrc::Vec2i m_mousePos;	   // The current position of the mouse in the window
		lrc::Vec2i m_mouseDownPos; // The position of the mouse when it was clicked
		bool m_mouseDown = false;  // Whether the mouse is currently down

		HistoryBuffer m_history;
		HistoryNode *m_historyNode	= nullptr;
		float m_historyScrollTarget = 0.0f;

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
