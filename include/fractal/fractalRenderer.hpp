#pragma once

namespace frac {
	class FractalRenderer {
	public:
		FractalRenderer(const json &config);
		~FractalRenderer();

		void setConfig(const json &config) {
			m_settings = config;

			// Set the default precision
			lrc::prec2(m_settings["renderConfig"]["precision"].get<int64_t>());

		}

	private:
		RenderConfig m_renderConfig;			 // The settings for the fractal renderer
		ci::Surface m_fractalSurface;			 // The surface that the fractal is rendered to
		ci::gl::Texture2dRef m_fractalTexture;	 // The image to be rendered to the screen
		ci::Font m_font = ci::Font("Arial", 24); // The font to use for rendering text
		lrc::Vec2i m_mousePos;					 // The current position of the mouse in the window
		json m_settings;						 // The settings for the fractal
		std::unique_ptr<Fractal> m_fractal;		 // The fractal to render
		ThreadPool m_threadPool;				 // Pool for render threads

		std::vector<RenderBox> m_renderBoxes; // The state of each render box
	};
}
