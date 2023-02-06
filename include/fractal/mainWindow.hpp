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

		void regenerateSurfaces();

		void updateConfigPrecision();

		/// Render the fractal into the fractal surface, and copy that to the
		/// fractal surface to be drawn. This will be executed on a separate thread
		/// in order to keep the UI updating
		void renderFractal();

		/// Render a sub-section of the fractal, defined by the \p box variable. This is intended
		/// to be used within the call queue to render multiple sections in parallel
		/// \param box
		void renderBox(const RenderBox &box, int64_t boxIndex = -1);

		LIBRAPID_ALWAYS_INLINE ci::ColorA pixelColorLow(const LowVec2 &pixPos, int64_t aliasFactor,
														const LowVec2 &step,
														const LowVec2 &aliasStepCorrect);

		LIBRAPID_ALWAYS_INLINE ci::ColorA pixelColorHigh(const HighVec2 &pixPos,
														 int64_t aliasFactor, const HighVec2 &step,
														 const HighVec2 &aliasStepCorrect);

		LIBRAPID_NODISCARD LIBRAPID_ALWAYS_INLINE RenderBoxTimeStats boxTimeStats() const;

		/// Callback for mouse movement (this does not include mouse clicks or drags)
		/// \param event The mouse event
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

		RenderConfig m_renderConfig;			 // The settings for the fractal renderer
		ci::Surface m_fractalSurface;			 // The surface that the fractal is rendered to
		ci::gl::Texture2dRef m_fractalTexture;	 // The image to be rendered to the screen
		ci::Font m_font = ci::Font("Arial", 24); // The font to use for rendering text
		lrc::Vec2i m_mousePos;					 // The current position of the mouse in the window
		json m_settings;						 // The settings for the fractal
		std::unique_ptr<Fractal> m_fractal;		 // The fractal to render
		ThreadPool m_threadPool;				 // Pool for render threads

		std::vector<RenderBox> m_renderBoxes; // The state of each render box

		bool m_haltRender = false; // Used to gracefully stop the render threads

		lrc::Vec2i m_mouseDownPos;	 // The position of the mouse when it was clicked
		lrc::Vec2i m_newViewBoxSize; // The size of the new view box (used for zooming)
		bool m_mouseDown = false;	 // Whether the mouse is currently down

		// Values used for ImGui input fields
		std::string m_fineMovementRe;
		std::string m_fineMovementIm;
		std::string m_fineMovementZoom;
	};
} // namespace frac
