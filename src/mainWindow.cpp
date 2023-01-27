#include <fractal/fractal.hpp>

namespace frac {
	void MainWindow::setup() {
		FRAC_LOG("Setup Called");

		// Set framerate
		setFrameRate(-1);
		ci::gl::enableVerticalSync(true);

		// Set the initial window size
		setWindowSize(1000, 800);

		// Setup ImGui
		ImGui::Initialize();
		ImGui::StyleColorsDark();
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		ImGui::GetIO().FontGlobalScale = 1.0f;

		// Set up rendering settings
		ci::gl::enableDepthWrite();
		ci::gl::enableDepthRead();
		glDepthFunc(GL_ALWAYS);

		// Load the settings file
		FRAC_LOG(fmt::format("Loading settings from {}", FRACTAL_UI_SETTINGS_PATH));
		std::fstream settingsFile(FRACTAL_UI_SETTINGS_PATH, std::ios::in);
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
						  1 << 16,
						  4,

						  lrc::Vec2i(800, 700),
						  lrc::Vec2i(25, 25),

						  lrc::Vec<HighPrecision, 2>(-2.2, -1.4),
						  lrc::Vec<HighPrecision, 2>(3.2, 2.6666666)};

		m_renderConfig.palette.addColor(ColorPalette::ColorType(142, 59, 70, 1) / 255.0f);
		m_renderConfig.palette.addColor(ColorPalette::ColorType(225, 221, 143, 1) / 255.0f);
		m_renderConfig.palette.addColor(ColorPalette::ColorType(224, 119, 125, 1) / 255.0f);
		m_renderConfig.palette.addColor(ColorPalette::ColorType(76, 134, 168, 1) / 255.0f);
		m_renderConfig.palette.addColor(ColorPalette::ColorType(71, 120, 144, 1) / 255.0f);

		m_fractal = std::make_unique<Mandelbrot>(m_renderConfig);

		regenerateSurfaces();
		m_fractalTexture = ci::gl::Texture2d::create(m_fractalSurface);
		renderFractal();
	}

	void MainWindow::cleanup() {
		m_haltRender = true;
		m_threadPool.wait_for_tasks();
	}

	void MainWindow::draw() {
		ci::gl::clear(ci::Color(0.2f, 0.2f, 0.2f));

		drawImGui();

		m_fractalTexture = ci::gl::Texture2d::create(m_fractalSurface);
		lrc::Vec2f drawPos(0, getWindowHeight() - m_renderConfig.imageSize.y());

		ci::gl::color(ci::ColorA(1, 1, 1, 1));
		ci::gl::draw(
		  m_fractalTexture,
		  ci::Rectf(drawPos, lrc::Vec2f(drawPos) + lrc::Vec2f(m_renderConfig.imageSize)));

		// Draw a rectangle if dragging mouse
		if (m_mouseDown) {
			ci::gl::color(ci::ColorA(1, 0, 0, 1));
			glu::drawStrokedRectangle(m_mouseDownPos, m_mousePos, 5);
		}
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

		m_threadPool.reset(m_renderConfig.numThreads);

		// Split the render into boxes to be rendered in parallel
		auto imageSize = m_renderConfig.imageSize;
		auto boxSize   = m_renderConfig.boxSize;

		// Round number of boxes up so the full image is covered
		auto numBoxes = lrc::Vec2i(lrc::ceil(lrc::Vec2f(imageSize) / lrc::Vec2f(boxSize)));

		for (int64_t i = 0; i < numBoxes.y(); ++i) {
			for (int64_t j = 0; j < numBoxes.x(); ++j) {
				lrc::Vec2i adjustedBoxSize(lrc::min(boxSize.x(), imageSize.x() - j * boxSize.x()),
										   lrc::min(boxSize.y(), imageSize.y() - i * boxSize.y()));
				RenderBox box {lrc::Vec2i(j, i) * boxSize, adjustedBoxSize};

				// renderBox(box);
				m_threadPool.push_task([this, box]() { renderBox(box); });
			}
		}

		FRAC_LOG("Fractal Complete...");
	}

	void MainWindow::renderBox(const RenderBox &box) {
		using HighVec2 = lrc::Vec<HighPrecision, 2>;

		HighVec2 fractalOrigin =
		  lrc::map(static_cast<HighVec2>(box.topLeft),
				   lrc::Vec<HighPrecision, 2>({0, 0}),
				   static_cast<HighVec2>(m_renderConfig.imageSize),
				   m_renderConfig.fracTopLeft,
				   m_renderConfig.fracTopLeft + static_cast<HighVec2>(m_renderConfig.fracSize));

		HighVec2 step = m_renderConfig.fracSize / static_cast<HighVec2>(m_renderConfig.imageSize);

		int64_t aliasFactor		  = m_renderConfig.antiAlias;
		HighVec2 aliasStepCorrect = 1 / static_cast<HighPrecision>(aliasFactor);

		// Make the primary axis of iteration the x-axis to improve cache efficiency and
		// increase performance.
		for (int64_t py = box.topLeft.y(); py < box.topLeft.y() + box.dimensions.y(); ++py) {
			// Quick return if required. Without this, the
			// render threads will continue running after the
			// application is closed, leading to weird behaviour.
			if (m_haltRender) return;

			for (int64_t px = box.topLeft.x(); px < box.topLeft.x() + box.dimensions.x(); ++px) {
				// Anti-aliasing
				auto pixPos =
				  fractalOrigin + step * HighVec2(px - box.topLeft.x(), py - box.topLeft.y());

				ci::ColorA pix(0, 0, 0, 1);

				for (int64_t aliasY = 0; aliasY < aliasFactor; ++aliasY) {
					for (int64_t aliasX = 0; aliasX < aliasFactor; ++aliasX) {
						auto pos = pixPos + step * HighVec2(aliasX, aliasY) * aliasStepCorrect;

						auto [iters, endPoint] =
						  m_fractal->iterCoord(lrc::Complex<HighPrecision>(pos.x(), pos.y()));
						if (endPoint.real() * endPoint.real() + endPoint.imag() * endPoint.imag() <
							4) {
							pix += ci::ColorA(0, 0, 0, 1); // Probably in the set
						} else {
							pix += m_fractal->getColor(endPoint, iters); // Probably not in the set
						}
					}
				}

				m_fractalSurface.setPixel(lrc::Vec2i(px, py),
										  pix / static_cast<float>(aliasFactor * aliasFactor));
			}
		}
	}

	void MainWindow::mouseMove(ci::app::MouseEvent event) { m_mousePos = event.getPos(); }

	void MainWindow::mouseDown(ci::app::MouseEvent event) {
		m_mouseDown	   = true;
		m_mouseDownPos = event.getPos();
	}

	void MainWindow::mouseDrag(ci::app::MouseEvent event) { m_mousePos = event.getPos(); }

	void MainWindow::mouseUp(ci::app::MouseEvent event) { m_mouseDown = false; }
} // namespace frac
