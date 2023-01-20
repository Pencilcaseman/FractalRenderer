#pragma once

#ifndef FRACTAL_SETTINGS_PATH
#	define FRACTAL_SETTINGS_PATH FRACTAL_RENDERER_ROOT_DIR "/settings/settings.json"
#endif

namespace frac {
	class MainWindow : public ci::app::App {
	public:
		/// Nothing passed to the constructor
		MainWindow() = default;

		/// Set up the window, configure ImGui and initialize the fractal rendering surfaces
		void setup() override;

		// Run on shutdown
		// void cleanup() override {  }

		/// Called every frame to render the frame
		void draw() override;

		/// Draw the UI
		void drawImGui();

		/// Callback for mouse movement (this does not include mouse clicks or drags)
		/// \param event
		void mouseMove(ci::app::MouseEvent event) override;

		ci::Surface m_fractalSurface;			 // The surface that the fractal is rendered to
		ci::gl::Texture2dRef m_fractalTexture;	 // The image to be rendered to the screen
		ci::Font m_font = ci::Font("Arial", 24); // The font to use for rendering text
		lrc::Vec2i m_mousePos;					 // The current position of the mouse in the window
		json m_settings;						 // The settings for the fractal
	};
} // namespace frac
