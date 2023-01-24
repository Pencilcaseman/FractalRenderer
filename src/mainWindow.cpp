#include <fractal/fractal.hpp>

namespace frac {
	void MainWindow::setup() {
		FRAC_LOG("Setup Called");

		// Set framerate
		setFrameRate(-1);
		ci::gl::enableVerticalSync(true);

		// Set the initial window size
		setWindowSize(800, 600);

		// Setup ImGui
		ImGui::Initialize();
		ImGui::StyleColorsDark();
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		ImGui::GetIO().FontGlobalScale = 1.0f;

		// Set up rendering settings
		ci::gl::enableDepthWrite();
		ci::gl::enableDepthRead();

		// Load the settings file
		FRAC_LOG(fmt::format("Loading settings from {}", FRACTAL_SETTINGS_PATH));
		std::fstream settingsFile(FRACTAL_SETTINGS_PATH, std::ios::in);
		if (settingsFile.is_open()) {
			settingsFile >> m_settings;
			settingsFile.close();
		} else {
			FRAC_ERROR("Failed to open settings file");
			quit();
		}

		// Validate settings
		if (m_settings["menus"].is_null()) {
			FRAC_ERROR("Settings file does not contain a \"menus\" object");
			quit();
		}

		if (m_settings["menus"]["fractalInfo"].is_null()) {
			FRAC_ERROR("Settings file does not contain a \"menus.fractalInfo\" object");
			quit();
		}

		m_renderConfig = {1,
						  100,
						  1,
						  10000,

						  lrc::Vec2i(600, 500),
						  lrc::Vec2i(600, 500),

						  lrc::Vec<HighPrecision, 2>(-2.2, -1.4),
						  lrc::Vec<HighPrecision, 2>(3.2, 2.8333)};

		m_fractal = std::make_unique<Mandelbrot>(m_renderConfig);

		regenerateSurfaces();
		m_fractalTexture = ci::gl::Texture2d::create(m_fractalSurface);
		renderFractal();
	}

	void MainWindow::draw() {
		ci::gl::clear(ci::Color(0.2f, 0.2f, 0.2f));

		drawImGui();

		m_fractalTexture = ci::gl::Texture2d::create(m_fractalSurface);
		lrc::Vec2f drawPos(0, getWindowHeight() - m_renderConfig.imageSize.y());

		ci::gl::draw(
		  m_fractalTexture,
		  ci::Rectf(drawPos, lrc::Vec2f(drawPos) + lrc::Vec2f(m_renderConfig.imageSize)));

		// ci::gl::draw(m_fractalTexture, ci::Rectf({0, 0}, {300, 300}));
	}

	void MainWindow::drawImGui() {
		// Fractal Information Window
		json fractalInfo = m_settings["menus"]["fractalInfo"];
		ImGui::SetNextWindowPos({(float)fractalInfo["posX"], (float)fractalInfo["posY"]},
								ImGuiCond_Once);
		ImGui::SetNextWindowSize({(float)fractalInfo["width"], (float)fractalInfo["height"]},
								 ImGuiCond_Once);
		ImGui::Begin("Fractal Info", nullptr);
		{
			ImGui::Text("Fractal Type: Mandelbrot");
			ImGui::Text("%s", fmt::format("Re:   {}", 0).c_str());
			ImGui::Text("%s", fmt::format("Im:   {}", 0).c_str());
			ImGui::Text("%s", fmt::format("Zoom: {}x", 1).c_str());
		}
		ImGui::End();
	}

	void MainWindow::regenerateSurfaces() {
		int64_t w		 = m_renderConfig.imageSize.x();
		int64_t h		 = m_renderConfig.imageSize.y();
		m_fractalSurface = ci::Surface((int32_t)w, (int32_t)h, true);
	}

	void MainWindow::renderFractal() {
		FRAC_LOG("Rendering Fractal...");
		renderBox({lrc::Vec2i {0, 0}, m_renderConfig.imageSize});
		FRAC_LOG("Fractal Complete...");
	}

	void MainWindow::renderBox(const RenderBox &box) {
		using HighVec2 = lrc::Vec<HighPrecision, 2>;

		HighVec2 fractalOrigin = lrc::map(static_cast<HighVec2>(box.topLeft),
										  lrc::Vec<HighPrecision, 2>({0, 0}),
										  static_cast<HighVec2>(m_renderConfig.imageSize),
										  m_renderConfig.fracTopLeft,
										  static_cast<HighVec2>(m_renderConfig.fracSize));
		HighVec2 step		   = m_renderConfig.fracSize / static_cast<HighVec2>(box.dimensions);

		// Make the primary axis of iteration the x-axis to improve cache efficiency and
		// increase
		for (int64_t py = box.topLeft.y(); py < box.dimensions.y(); ++py) {
			for (int64_t px = box.topLeft.x(); px < box.dimensions.x(); ++px) {
				auto pos = fractalOrigin + step * lrc::Vec<HighPrecision, 2>(px, py);

				// m_fractalSurface.setPixel(lrc::Vec2i(px, py), ci::ColorA(1, 0, 0, 1));

				auto iters = m_fractal->iterCoord(lrc::Complex<HighPrecision>(pos.x(), pos.y()));
				if (iters == m_renderConfig.maxIters) {
					m_fractalSurface.setPixel(lrc::Vec2i(px, py), ci::ColorA(0, 0, 0, 1));
				} else {
					m_fractalSurface.setPixel(lrc::Vec2i(px, py),
											  m_fractal->getColor({pos.x(), pos.y()}, iters));
				}
			}
		}
	}

	void MainWindow::mouseMove(ci::app::MouseEvent event) { m_mousePos = event.getPos(); }
} // namespace frac
