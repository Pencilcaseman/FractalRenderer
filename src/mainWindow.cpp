#include <fractal/fractal.hpp>

namespace frac {
	void MainWindow::configureSettings() {
		// Load the settings file
		FRAC_LOG(fmt::format("Loading settings from {}", FRACTAL_UI_SETTINGS_PATH));

		std::fstream settingsFile(FRACTAL_UI_SETTINGS_PATH, std::ios::in);
		if (settingsFile.is_open()) {
			m_renderer.setConfig(json::parse(settingsFile));
			m_history.append(m_renderer.config(), m_renderer.surface());
		} else {
			FRAC_ERROR("Failed to open settings file");
			quit();
		}

		FRAC_LOG("Settings Configured");
	}

	void MainWindow::configureWindow() {
		setFrameRate(-1);				  // Unlimited framerate
		ci::gl::enableVerticalSync(true); // Enable vertical sync to avoid tearing
		setWindowSize(1200, 700);		  // Set the initial window size

		// Set up rendering settings
		ci::gl::enableDepthWrite();
		ci::gl::enableDepthRead();
		ci::gl::enableDepth();
		glDepthFunc(GL_ALWAYS);

		FRAC_LOG("Window Configured");
	}

	void MainWindow::configureImGui() {
		ImGui::Initialize();
		ImGui::StyleColorsDark();
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		ImGui::GetIO().FontGlobalScale = 1.0f;

		FRAC_LOG("ImGui Configured");
	}

	void MainWindow::setup() {
		FRAC_LOG("Setup Called");

		configureSettings();
		configureWindow();
		configureImGui();

		m_renderer.regenerateSurface();
		renderFractal();

		FRAC_LOG("Setup Complete");
	}

	void MainWindow::stopRender() { m_renderer.stopRender(); }

	void MainWindow::cleanup() {
		stopRender();
		FRAC_LOG("Cleaned Up");
	}

	void MainWindow::drawFractal() {
		m_fractalTexture = ci::gl::Texture2d::create(m_renderer.surface());

		const RenderConfig &config = m_renderer.config();
		double aspect = (double)config.imageSize.x() / (double)config.imageSize.y();
		double height = getWindowHeight();
		lrc::Vec2f renderSize(height * aspect, height);

		ci::gl::color(ci::ColorA(1, 1, 1, 1));
		ci::gl::draw(m_fractalTexture, ci::Rectf({0, 0}, renderSize));
	}

	void MainWindow::outlineRenderBoxes() {
		const RenderConfig &config				  = m_renderer.config();
		const std::vector<RenderBox> &renderBoxes = m_renderer.renderBoxes();
		for (const auto &box : renderBoxes) {
			switch (box.state) {
				case RenderBoxState::None:
				case RenderBoxState::Rendered: continue;
				case RenderBoxState::Queued:
					ci::gl::color(ci::ColorA(0, 1, 0, 0.2));
					break;
				case RenderBoxState::Rendering:
					ci::gl::color(ci::ColorA(1, 1, 0, 0.2));
					break;
			}

			ci::ivec2 boxPos  = box.topLeft;
			ci::ivec2 boxSize = box.dimensions;
			boxPos.y += getWindowHeight() - config.imageSize.y();
			ci::gl::drawStrokedRect(ci::Rectf(boxPos, boxPos + boxSize), 1);
		}
	}

	void MainWindow::draw() {
		ci::gl::clear(ci::Color(0.2f, 0.2f, 0.2f));

		drawImGui();
		drawFractal();
		outlineRenderBoxes();
		drawHistory();

		if (m_drawingZoomBox) {
			// Draw an aspect-ratio corrected box
			RenderConfig config = m_renderer.config();
			float aspectRatio = (float)config.imageSize.x() / (float)config.imageSize.y();
			lrc::Vec2i correctedBox =
			  aspectCorrectedBox(m_mouseDownPos, m_mousePos, aspectRatio);
			auto correctedEnd = m_mouseDownPos + correctedBox;
			drawZoomBox(m_mouseDownPos, correctedEnd);
		}

		if (m_showZoomBox) { drawZoomBox(m_zoomBoxStart, m_zoomBoxEnd); }
	}

	void MainWindow::drawImGui() {
		RenderConfig &config = m_renderer.config();
		const json &settings = m_renderer.settings();

		// Fractal Information Window
		json fractalInfo = settings["menus"]["fractalInfo"];
		ImGui::SetNextWindowPos({(float)fractalInfo["posX"], (float)fractalInfo["posY"]},
								ImGuiCond_Once);
		ImGui::SetNextWindowSize(
		  {(float)fractalInfo["width"], (float)fractalInfo["height"]}, ImGuiCond_Once);
		ImGui::Begin("Fractal Info", nullptr);
		{
			ImGui::Text("Fractal Type: Mandelbrot");

			HighPrecision re   = config.fracTopLeft.x() + config.fracSize.x() / 2;
			HighPrecision im   = config.fracTopLeft.y() + config.fracSize.y() / 2;
			HighPrecision zoom = config.originalFracSize.x() / config.fracSize.x();

			ImGui::TextWrapped("%s", fmt::format("Re:   {}", re).c_str());
			ImGui::TextWrapped("%s", fmt::format("Im:   {}", im).c_str());

			std::ostringstream os;
			os << std::fixed << std::setprecision(6) << std::scientific << zoom;
			ImGui::TextWrapped("%s", fmt::format("Zoom: {}x", os.str()).c_str());

			double maxZoomExponent = config.precision / lrc::log2(10);
			ImGui::TextWrapped(
			  "%s", fmt::format("Max Zoom: e+{:.3f}", maxZoomExponent).c_str());
		}
		ImGui::End();

		// Fine movement window
		json fineMovement = settings["menus"]["fineMovement"];
		ImGui::SetNextWindowPos(
		  {(float)fineMovement["posX"], (float)fineMovement["posY"]}, ImGuiCond_Once);
		ImGui::SetNextWindowSize(
		  {(float)fineMovement["width"], (float)fineMovement["height"]}, ImGuiCond_Once);
		ImGui::Begin("Fine Movement", nullptr);
		{
			ImGui::InputText("Re", &m_fineMovementRe);
			ImGui::InputText("Im", &m_fineMovementIm);
			ImGui::InputText("Zoom", &m_fineMovementZoom);

			if (ImGui::Button("Apply")) {
				HighPrecision re, im, zoom, sizeRe, sizeIm;
				scn::scan(m_fineMovementRe, "{}", re);
				scn::scan(m_fineMovementIm, "{}", im);
				scn::scan(m_fineMovementZoom, "{}", zoom);

				FRAC_LOG(fmt::format("Received Real Part: {}", re));

				sizeRe = config.originalFracSize.x() / zoom;
				sizeIm = config.originalFracSize.y() / zoom;
				moveFractalCenter(lrc::Vec<HighPrecision, 2>(re, im),
								  lrc::Vec<HighPrecision, 2>(sizeRe, sizeIm));
				renderFractal();
			}
		}
		ImGui::End();

		// Render configuration
		json renderConfigMenu = settings["menus"]["renderConfig"];
		ImGui::SetNextWindowPos(
		  {(float)renderConfigMenu["posX"], (float)renderConfigMenu["posY"]},
		  ImGuiCond_Once);
		ImGui::SetNextWindowSize(
		  {(float)renderConfigMenu["width"], (float)renderConfigMenu["height"]},
		  ImGuiCond_Once);
		ImGui::Begin("Render Configuration", nullptr);
		{
			static int64_t minThreads	= 1;
			static int64_t maxThreads	= std::thread::hardware_concurrency();
			static int64_t minIters		= 1;
			static int64_t maxIters		= 100000;
			static int64_t minPrecision = 64;
			static int64_t maxPrecision = 1024;
			static int64_t minAntiAlias = 1;
			static int64_t maxAntiAlias = 16;

			static int64_t newThreads	= config.numThreads;
			static int64_t newIters		= config.maxIters;
			static int64_t newPrecision = config.precision;
			static int64_t newAntiAlias = config.antiAlias;

			ImGui::SliderScalar(
			  "Threads", ImGuiDataType_S64, &newThreads, &minThreads, &maxThreads);

			ImGui::SliderScalar("Anti Aliasing",
								ImGuiDataType_S64,
								&newAntiAlias,
								&minAntiAlias,
								&maxAntiAlias);

			ImGui::DragScalarN(
			  "Iterations", ImGuiDataType_S64, &newIters, 1, 5, &minIters, &maxIters);

			ImGui::DragScalarN("Precision",
							   ImGuiDataType_S64,
							   &newPrecision,
							   1,
							   0.1,
							   &minPrecision,
							   &maxPrecision);

			if (ImGui::Button("Apply")) {
				stopRender();
				config.numThreads = newThreads;
				config.maxIters	  = newIters;
				config.precision  = newPrecision;
				config.antiAlias  = newAntiAlias;
				lrc::prec2(newPrecision);
				m_renderer.updateRenderConfig();
				m_renderer.updateConfigPrecision();
				renderFractal();
			}
		}
		ImGui::End();

		// Render Statistics
		json renderStatistics = settings["menus"]["renderStatistics"];
		ImGui::SetNextWindowPos(
		  {(float)renderStatistics["posX"], (float)renderStatistics["posY"]},
		  ImGuiCond_Once);
		ImGui::SetNextWindowSize(
		  {(float)renderStatistics["width"], (float)renderStatistics["height"]},
		  ImGuiCond_Once);

		RenderBoxTimeStats stats = m_renderer.boxTimeStats();
		ImGui::Begin("Render Statistics", nullptr);
		{
			ImGui::Text("Pixels/s (min): %s", fmt::format("{:.3f}", stats.min).c_str());
			ImGui::Text("Pixels/s (max): %s", fmt::format("{:.3f}", stats.max).c_str());
			ImGui::Text("Pixels/s (avg): %s",
						fmt::format("{:.3f}", stats.average).c_str());
			ImGui::Text("Estimated Time Remaining: %s",
						lrc::formatTime(stats.remainingTime).c_str());
		}
		ImGui::End();
	}

	void MainWindow::drawHistory() {
		constexpr float historyFrameWidth = 150;
		constexpr float historyFrameSep	  = 8; // Spacing between frames
		const auto windowWidth			  = (float)getWindowWidth();
		const auto windowHeight			  = (float)getWindowHeight();
		const RenderConfig &config		  = m_renderer.config();
		size_t historySize				  = m_history.size();
		HistoryNode *node				  = m_history.first();
		int64_t index					  = 0;

		// Draw a bounding box for the frames to sit within
		float boxLeft = windowWidth - historyFrameWidth - historyFrameSep * 2;
		ci::gl::color(ci::ColorA(0.15, 0.15, 0.3, 1));
		ci::gl::drawSolidRect(ci::Rectf(boxLeft, 0, windowWidth, windowHeight));

		while (node) {
			auto texture = ci::gl::Texture2d::create(node->surface());
			float aspect = (float)config.imageSize.x() / (float)config.imageSize.y();
			lrc::Vec2f renderSize(historyFrameWidth, historyFrameWidth / aspect);
			lrc::Vec2f drawPos(windowWidth - historyFrameWidth - historyFrameSep,
							   (renderSize.y() + historyFrameSep) *
								   (float)(historySize - index - 1) +
								 historyFrameSep);

			ci::gl::color(ci::ColorA(1, 1, 1, 1));
			ci::gl::draw(texture, ci::Rectf(drawPos, drawPos + renderSize));
			ci::gl::color(ci::ColorA(0, 0, 0, 1));
			glu::drawStrokedRectangle(drawPos, drawPos + renderSize, 3);

			node = node->next();
			index++;
		}
	}

	void MainWindow::moveFractalCorner(const lrc::Vec<HighPrecision, 2> &topLeft,
									   const lrc::Vec<HighPrecision, 2> &size) {
		m_renderer.moveFractalCorner(topLeft, size);
	}

	void MainWindow::moveFractalCenter(const lrc::Vec<HighPrecision, 2> &center,
									   const lrc::Vec<HighPrecision, 2> &size) {
		m_renderer.moveFractalCenter(center, size);
	}

	void MainWindow::zoomFractal(const lrc::Vec2i &pixTopLeft,
								 const lrc::Vec2i &pixBottomRight) {
		// The vast majority of the calculations must be done at the highest
		// precision possible. Without this, the zooming will not function
		// correctly when the zoom factor exceeds the range of 64-bit precision.

		FRAC_LOG("Moving Fractal...");
		FRAC_LOG(fmt::format("Pixel Coordinates: {} -> {}", pixTopLeft, pixBottomRight));
		updateHistoryItem();

		RenderConfig &config = m_renderer.config();
		ci::Surface &surface = m_renderer.surface();

		// Resize the fractal area
		HighVec2 imageSize	= config.imageSize;
		HighVec2 pixelDelta = pixBottomRight - pixTopLeft;

		HighVec2 newFracPos = lrc::map(HighVec2(pixTopLeft),
									   HighVec2(0, 0),
									   imageSize,
									   config.fracTopLeft,
									   config.fracTopLeft + config.fracSize);

		HighVec2 newFracSize = lrc::map(
		  pixelDelta, HighVec2(0, 0), imageSize, HighVec2(0, 0), config.fracSize);

		// Copy the pixels from the selected region to the new region
		// A temporary buffer is required here because, at some point, the loop
		// would be writing to the same pixels it is reading from, resulting in
		// strange visual glitches.
		int64_t imgWidth  = config.imageSize.x();
		int64_t imgHeight = config.imageSize.y();
		int64_t index	  = 0;
		std::vector<ci::ColorA> newPixels(imgWidth * imgHeight);
		auto mouseStartInImageLow =
		  pixTopLeft - lrc::Vec2i {0, getWindowHeight() - config.imageSize.y()};
		for (int64_t y = 0; y < imgHeight; ++y) {
			for (int64_t x = 0; x < imgWidth; ++x) {
				int64_t pixX = lrc::map(x,
										0.f,
										imgWidth,
										mouseStartInImageLow.x(),
										mouseStartInImageLow.x() + (float)pixelDelta.x());
				int64_t pixY = lrc::map(y,
										0.f,
										imgHeight,
										mouseStartInImageLow.y(),
										mouseStartInImageLow.y() + (float)pixelDelta.y());

				newPixels[index++] = surface.getPixel({pixX, pixY});
			}
		}

		// Write the new pixels to the surface
		index = 0;
		for (int64_t y = 0; y < imgHeight; ++y) {
			for (int64_t x = 0; x < imgWidth; ++x) {
				surface.setPixel({x, y}, newPixels[index++]);
			}
		}

		config.fracTopLeft = newFracPos;
		config.fracSize	   = newFracSize;
		m_renderer.updateRenderConfig();
		renderFractal();
	}

	void MainWindow::renderFractal() {
		// updateHistoryItem();
		m_renderer.renderFractal();
		m_history.append(m_renderer.config(), m_renderer.surface());
	}

	void MainWindow::updateHistoryItem() {
		// Update history buffer surface before re-rendering the fractal
		if (m_history.size() > 0) {
			m_history.last()->setSurface(m_renderer.surface());
			FRAC_LOG("Writing to surface");
		}
	}

	void MainWindow::mouseMove(ci::app::MouseEvent event) { m_mousePos = event.getPos(); }

	void MainWindow::mouseDown(ci::app::MouseEvent event) {
		// Don't capture mouse events if ImGui wants them
		if (ImGui::GetIO().WantCaptureMouse) return;

		m_mouseDown	   = true;
		m_mouseDownPos = event.getPos();

		// Ensure mouse is within the image
		if (m_mouseDownPos.x() >= 0 &&
			m_mouseDownPos.x() < m_renderer.config().imageSize.x() &&
			m_mouseDownPos.y() >= 0 &&
			m_mouseDownPos.y() < m_renderer.config().imageSize.y()) {
			constexpr size_t offset = 0; // pixel offset from the edge of the box
			if (m_showZoomBox && m_mouseDownPos.x() > m_zoomBoxStart.x() + offset &&
				m_mouseDownPos.x() < m_zoomBoxEnd.x() - offset &&
				m_mouseDownPos.y() > m_zoomBoxStart.y() + offset &&
				m_mouseDownPos.y() < m_zoomBoxEnd.y() - offset) {
				m_moveZoomBox = true;
			} else {
				m_drawingZoomBox = true;
				m_showZoomBox	 = false;
			}
		}
	}

	void MainWindow::mouseDrag(ci::app::MouseEvent event) {
		if (ImGui::GetIO().WantCaptureMouse) return;
		lrc::Vec2i delta = lrc::Vec2i(event.getPos()) - m_mousePos;
		m_mousePos		 = event.getPos();

		if (m_moveZoomBox) {
			m_zoomBoxStart += delta;
			m_zoomBoxEnd += delta;
		}
	}

	void MainWindow::mouseUp(ci::app::MouseEvent event) {
		if (ImGui::GetIO().WantCaptureMouse) return;
		RenderConfig &config = m_renderer.config();
		m_mouseDown			 = false;

		// Resize the fractal area
		lrc::Vec2i imageSize = config.imageSize;
		float aspectRatio	 = (float)imageSize.x() / (float)imageSize.y();

		lrc::Vec2f aspectCorrected =
		  aspectCorrectedBox(m_mouseDownPos, m_mousePos, aspectRatio);

		// Persistent zoom area
		if (m_drawingZoomBox) {
			m_zoomBoxStart	 = m_mouseDownPos;
			m_zoomBoxEnd	 = m_mouseDownPos + lrc::Vec2i(aspectCorrected);
			m_showZoomBox	 = true;
			m_drawingZoomBox = false;
		}
	}

	void MainWindow::keyDown(ci::app::KeyEvent event) {
		if (m_showZoomBox && event.getCode() == ci::app::KeyEvent::KEY_RETURN) {
			// Update history and apply zoom box
			// updateHistoryItem();
			zoomFractal(m_zoomBoxStart, m_zoomBoxEnd);
			m_showZoomBox = false;
		} else if (m_showZoomBox && event.getCode() == ci::app::KeyEvent::KEY_ESCAPE) {
			// Cancel zoom box
			m_showZoomBox = false;
		}
	}

	void MainWindow::drawZoomBox(const lrc::Vec2f &start, const lrc::Vec2f &end) const {
		ci::gl::color(ci::ColorA(1, 0, 0, 0.333)); // Red with alpha
		ci::gl::drawSolidRect(ci::Rectf(start.x(), start.y(), end.x(), end.y()));
		ci::gl::color(ci::ColorA(1, 0, 0, 1)); // Red
		glu::drawStrokedRectangle(start, end, 5);
	}
} // namespace frac
