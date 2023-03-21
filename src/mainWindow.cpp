#include <fractal/fractal.hpp>

namespace frac {
	void MainWindow::configureSettings() {
		// Load the settings file
		FRAC_LOG(fmt::format("Loading settings from {}", FRACTAL_UI_SETTINGS_PATH));

		std::fstream settingsFile(FRACTAL_UI_SETTINGS_PATH, std::ios::in);
		if (settingsFile.is_open()) {
			m_renderer.setConfig(json::parse(settingsFile));
			m_history.append(m_renderer.config(), m_renderer.surface());
			m_historyNode = m_history.first();
		} else {
			FRAC_ERROR("Failed to open settings file");
			quit();
		}

		FRAC_LOG("Settings Configured");
	}

	void MainWindow::configureWindow() {
		setFrameRate(-1);				  // Unlimited framerate
		ci::gl::enableVerticalSync(true); // Enable vertical sync to avoid tearing
		setWindowSize(1364, 700);		  // Set the initial window size

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

	void MainWindow::appendConfigToHistory() {
		FRAC_LOG("Appending to history");

		if (m_historyNode != m_history.last()) {
			// There is a pre-existing history, so it must be overwritten
			m_historyNode->killChildren();
		}

		m_history.append(m_renderer.config(), m_renderer.surface());
		m_historyNode = m_history.last();
	}

	void MainWindow::setFractalType(const std::string &name) {
		std::shared_ptr<Fractal> newFracPtr;

		if (name == "Mandelbrot") {
			newFracPtr = std::make_shared<Mandelbrot>(m_renderer.config());
		} else if (name == "Julia Set") {
			newFracPtr = std::make_shared<JuliaSet>(m_renderer.config());
		} else if (name == "Newton's Fractal") {
			newFracPtr = std::make_shared<NewtonFractal>(m_renderer.config());
		} else {
			newFracPtr = std::make_shared<Mandelbrot>(m_renderer.config());
		}

		// If changing the fractal, clear the hisotry, since it is no longer
		// valid
		m_history.first()->killChildren();
		m_historyNode = m_history.first();

		m_renderer.updateFractalType(newFracPtr);
	}

	std::vector<std::tuple<lrc::Vec2f, lrc::Vec2f, HistoryNode *>>
	MainWindow::getHistoryFrameLocations() const {
		std::vector<std::tuple<lrc::Vec2f, lrc::Vec2f, HistoryNode *>> ret;

		const json &settings		  = m_renderer.settings();
		const float historyFrameWidth = settings["menus"]["history"]["frameWidth"];
		const float historyFrameSep	  = settings["menus"]["history"]["frameSep"];

		const auto windowWidth	   = (float)getWindowWidth();
		const RenderConfig &config = m_renderer.config();
		size_t historySize		   = m_history.size();
		HistoryNode *node		   = m_history.first();
		int64_t index			   = 0;
		int64_t totalHeight		   = 0;

		while (node) {
			float aspect = (float)config.imageSize.x() / (float)config.imageSize.y();
			lrc::Vec2f renderSize(historyFrameWidth, historyFrameWidth / aspect);
			lrc::Vec2f drawPos(windowWidth - historyFrameWidth - historyFrameSep,
							   (renderSize.y() + historyFrameSep) *
								   (float)(historySize - index - 1) +
								 historyFrameSep);

			totalHeight += (int64_t)(renderSize.y() + historyFrameSep);
			drawPos.y(drawPos.y() + m_historyScrollTarget);

			ret.emplace_back(std::tuple<lrc::Vec2f, lrc::Vec2f, HistoryNode *>(
			  drawPos, renderSize, node));

			node = node->next();
			index++;
		}

		return ret;
	}

	void MainWindow::undoLastMove() {
		FRAC_LOG("Attempting to Undo...");
		// Ensure a previous configuration actually exists
		if (m_historyNode->prev()) {
			FRAC_LOG("Undo successful");
			m_historyNode = m_historyNode->prev();

			// Update the render configuration
			m_renderer.config() = m_historyNode->config();
			renderFractal(false);
		}
	}

	void MainWindow::redoLastMove() {
		FRAC_LOG("Attempting to Redo...");
		if (m_historyNode->next()) {
			FRAC_LOG("Redo successful");
			m_historyNode = m_historyNode->next();

			// Update the render configuration
			m_renderer.config() = m_historyNode->config();
			renderFractal(false);
		}
	}

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

			ci::ivec2 boxPos  = imageToScreenSpace(box.topLeft);
			ci::ivec2 boxSize = imageToScreenSpace(box.dimensions);
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
		// Ensure there is space for the labels in the ImGui windows
		constexpr int64_t labelledItemWidth = -150;

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

			double maxZoomExponent = (double)config.precision / lrc::log2(10);
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
				bool valid = true;

				auto reScan	  = scn::scan(m_fineMovementRe, "{}", re);
				auto imScan	  = scn::scan(m_fineMovementIm, "{}", im);
				auto zoomScan = scn::scan(m_fineMovementZoom, "{}", zoom);

				if (!reScan) {
					FRAC_ERROR(fmt::format("Invalid Real Part: {}", m_fineMovementRe));
					valid = false;
				}

				if (!imScan) {
					FRAC_ERROR(
					  fmt::format("Invalid Imaginary Part: {}", m_fineMovementIm));
					valid = false;
				}

				if (!zoomScan || zoom <= 0) {
					FRAC_ERROR(fmt::format("Invalid Zoom: {}", m_fineMovementZoom));
					valid = false;
				}

				if (!valid) {
					FRAC_ERROR(fmt::format("Invalid input: {}, {}, {}",
										   m_fineMovementRe,
										   m_fineMovementIm,
										   m_fineMovementZoom));
				} else {
					FRAC_LOG(fmt::format("Received Real Part: {}", re));

					sizeRe = config.originalFracSize.x() / zoom;
					sizeIm = config.originalFracSize.y() / zoom;
					moveFractalCenter(lrc::Vec<HighPrecision, 2>(re, im),
									  lrc::Vec<HighPrecision, 2>(sizeRe, sizeIm));
					renderFractal();
				}
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
			static int64_t minDraftInc	= 1;
			static int64_t maxDraftInc	= 4;

			static int64_t newThreads	= config.numThreads;
			static int64_t newIters		= config.maxIters;
			static int64_t newPrecision = config.precision;
			static int64_t newAntiAlias = config.antiAlias;
			static bool newDraftRender	= config.draftRender;
			static int64_t newDraftInc	= config.draftInc;

			ImGui::PushItemWidth(labelledItemWidth);
			ImGui::SliderScalar(
			  "Threads", ImGuiDataType_S64, &newThreads, &minThreads, &maxThreads);

			ImGui::PushItemWidth(labelledItemWidth);
			ImGui::SliderScalar("Anti Aliasing",
								ImGuiDataType_S64,
								&newAntiAlias,
								&minAntiAlias,
								&maxAntiAlias);

			ImGui::PushItemWidth(labelledItemWidth);
			ImGui::DragScalarN(
			  "Iterations", ImGuiDataType_S64, &newIters, 1, 5, &minIters, &maxIters);

			ImGui::PushItemWidth(labelledItemWidth);
			ImGui::DragScalarN("Precision",
							   ImGuiDataType_S64,
							   &newPrecision,
							   1,
							   0.1,
							   &minPrecision,
							   &maxPrecision);

			if (ImGui::Button("Apply")) {
				stopRender();
				config.numThreads  = newThreads;
				config.maxIters	   = newIters;
				config.precision   = newPrecision;
				config.antiAlias   = newAntiAlias;
				config.draftRender = newDraftRender;
				config.draftInc	   = newDraftInc;
				lrc::prec2(newPrecision);
				m_renderer.updateRenderConfig();
				m_renderer.updateConfigPrecision();
				renderFractal();
			}

			ImGui::SameLine();
			ImGui::Checkbox("Draft Mode", &newDraftRender);

			if (newDraftRender) {
				ImGui::SameLine();
				ImGui::PushItemWidth(labelledItemWidth);
				ImGui::SliderScalar("Draft Increment",
									ImGuiDataType_S64,
									&newDraftInc,
									&minDraftInc,
									&maxDraftInc);
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

		json fractalMenu = settings["menus"]["fractalSettings"];
		ImGui::SetNextWindowPos({(float)fractalMenu["posX"], (float)fractalMenu["posY"]},
								ImGuiCond_Once);
		ImGui::SetNextWindowSize(
		  {(float)fractalMenu["width"], (float)fractalMenu["height"]}, ImGuiCond_Once);

		ImGui::Begin("Fractal Settings");
		{
			// Helper function for ImGui::Combo
			static auto getter = [](void *vec, int idx, const char **out_text) {
				auto &vector = *static_cast<std::vector<std::string> *>(vec);
				if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
				*out_text = vector.at(idx).c_str();
				return true;
			};

			// -------------------- Fractal Type --------------------
			{
				static int currentFractalType		  = 0;
				std::vector<std::string> fractalNames = {
				  "Mandelbrot",		 // 0
				  "Julia Set",		 // 1
				  "Newton's Fractal" // 2
				};

				ImGui::PushItemWidth(labelledItemWidth);
				if (ImGui::Combo("Fractal",
								 &currentFractalType,
								 +getter,
								 &fractalNames,
								 fractalNames.size())) {
					setFractalType(fractalNames[currentFractalType]);
					renderFractal();
				}
			}

			// -------------------- Coloring Algorithm --------------------
			{
				static int currentColoringFunc		   = 0;
				std::vector<std::string> coloringFuncs = m_renderer.getColorFuncs();

				ImGui::PushItemWidth(labelledItemWidth);
				if (ImGui::Combo("Colouring Algorithm",
								 &currentColoringFunc,
								 +getter,
								 &coloringFuncs,
								 coloringFuncs.size())) {
					stopRender();
					m_renderer.setColorFunc(coloringFuncs[currentColoringFunc]);
					renderFractal();
				}
			}

			// -------------------- Color Palette --------------------
			{
				static int currentPalette			  = 0;
				std::vector<std::string> paletteNames = m_renderer.getPaletteNames();

				ImGui::PushItemWidth(labelledItemWidth);
				if (ImGui::Combo("Colour Palette",
								 &currentPalette,
								 +getter,
								 &paletteNames,
								 paletteNames.size())) {
					stopRender();
					m_renderer.setPaletteName(paletteNames[currentPalette]);
					renderFractal();
				}
			}

			// -------------------- Reset Fractal --------------------
			{
				if (ImGui::Button("Reset Fractal")) {
					stopRender();
					const std::string &fractalName = m_renderer.getFractalName();
					const auto &fractalSettings =
					  m_renderer.settings()["renderConfig"]["fractals"][fractalName];
					float bail = fractalSettings["bail"];
					HighVec2 topLeft, size;

					topLeft.x(fractalSettings["fracTopLeft"]["Re"].get<double>());
					topLeft.y(fractalSettings["fracTopLeft"]["Im"].get<double>());

					size.x(fractalSettings["fracSize"]["Re"].get<double>());
					size.y(fractalSettings["fracSize"]["Im"].get<double>());

					config.bail		   = bail;
					config.fracTopLeft = topLeft;
					config.fracSize	   = size;
					renderFractal();
				}
			}

			// ------------------ Reset Image Size --------------------
			{
				ImGui::SameLine();
				if (ImGui::Button("Reset Image Size")) {
					stopRender();
					config.imageSize = imageToScreenSpace(config.imageSize);
					m_renderer.regenerateSurface();
					renderFractal();
				}
			}
		}
		ImGui::End();
	}

	void MainWindow::drawHistory() {
		const json &settings		  = m_renderer.settings();
		const float historyFrameWidth = settings["menus"]["history"]["frameWidth"];
		const float historyFrameSep	  = settings["menus"]["history"]["frameSep"];

		const auto windowWidth	   = (float)getWindowWidth();
		const auto windowHeight	   = (float)getWindowHeight();
		const RenderConfig &config = m_renderer.config();
		const std::vector<std::tuple<lrc::Vec2f, lrc::Vec2f, HistoryNode *>> frames =
		  getHistoryFrameLocations();

		int64_t totalHeight = 0;

		// Draw a bounding box for the frames to sit within
		float boxLeft = windowWidth - historyFrameWidth - historyFrameSep * 2;
		ci::gl::color(ci::ColorA(0.15, 0.15, 0.3, 1));
		ci::gl::drawSolidRect(ci::Rectf(boxLeft, 0, windowWidth, windowHeight));

		for (const auto &frame : frames) {
			const lrc::Vec2f framePos  = std::get<0>(frame);
			const lrc::Vec2f frameSize = std::get<1>(frame);
			const HistoryNode *node	   = std::get<2>(frame);

			auto texture = ci::gl::Texture2d::create(node->surface());
			totalHeight += (int64_t)(frameSize.y() + historyFrameSep);

			// Don't draw the frame if it's not visible on the screen
			if (framePos.y() > windowHeight || framePos.y() + frameSize.y() < 0) continue;

			ci::gl::color(ci::ColorA(1, 1, 1, 1));
			ci::gl::draw(texture, ci::Rectf(framePos, framePos + frameSize));
			ci::gl::color(ci::ColorA(0, 0, 0, 1));
			glu::drawStrokedRectangle(framePos, framePos + frameSize, 3);

			if (node == m_historyNode) {
				// If this frame is selected, outline it gold
				ci::gl::color(ci::ColorA(1, 1, 0, 1));
				glu::drawStrokedRectangle(framePos, framePos + frameSize, 4);
			}
		}

		if (windowHeight < (float)totalHeight) {
			m_historyScrollTarget =
			  lrc::clamp(m_historyScrollTarget, windowHeight - (float)totalHeight, 0);
		} else {
			m_historyScrollTarget = 0;
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

		// Calculate image-space coordinates (allows for resizing the window)
		// It is known that the image is drawn at (0, 0) and that the aspect ratio remains
		// constant, so it is safe to multiply by a scaling factor. It is also known that
		// the image is drawn to the full height of the window.

		updateHistoryItem();

		RenderConfig &config = m_renderer.config();
		ci::Surface &surface = m_renderer.surface();

		lrc::Vec2i imagePixTopLeft	   = screenToImageSpace(pixTopLeft);
		lrc::Vec2i imagePixBottomRight = screenToImageSpace(pixBottomRight);

		FRAC_LOG(fmt::format(
		  "Image Coordinates: {} -> {}", imagePixTopLeft, imagePixBottomRight));

		// Resize the fractal area
		HighVec2 imageSize	= config.imageSize;
		HighVec2 pixelDelta = imagePixBottomRight - imagePixTopLeft;

		HighVec2 newFracPos = lrc::map(HighVec2(imagePixTopLeft),
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
		for (int64_t y = 0; y < imgHeight; ++y) {
			for (int64_t x = 0; x < imgWidth; ++x) {
				lrc::Vec2i pixPos  = lrc::map(lrc::Vec2f(x, y),
											  lrc::Vec2f(0, 0),
											  lrc::Vec2f(imgWidth, imgHeight),
											  lrc::Vec2f(imagePixTopLeft),
											  lrc::Vec2f(imagePixBottomRight));
				newPixels[index++] = surface.getPixel(pixPos);
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

	void MainWindow::renderFractal(bool amendHistory) {
		m_renderer.renderFractal();
		if (amendHistory) appendConfigToHistory();
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

		lrc::Vec2i imageSpacePos = imageToScreenSpace(m_renderer.config().imageSize);

		// Ensure mouse is within the image
		if (m_mouseDownPos.x() >= 0 && m_mouseDownPos.x() < imageSpacePos.x() &&
			m_mouseDownPos.y() >= 0 && m_mouseDownPos.y() < imageSpacePos.y()) {
			// Check if mouse is inside the box (and it is being shown)
			if (m_showZoomBox && m_mouseDownPos.x() > m_zoomBoxStart.x() &&
				m_mouseDownPos.x() < m_zoomBoxEnd.x() &&
				m_mouseDownPos.y() > m_zoomBoxStart.y() &&
				m_mouseDownPos.y() < m_zoomBoxEnd.y()) {
				m_moveZoomBox = true;
			} else {
				m_drawingZoomBox = true;
				m_showZoomBox	 = false;
			}
		}

		// See if mouse is in the history buffer section
		const json &settings		  = m_renderer.settings();
		const float historyFrameWidth = settings["menus"]["history"]["frameWidth"];
		const float historyFrameSep	  = settings["menus"]["history"]["frameSep"];
		const std::vector<std::tuple<lrc::Vec2f, lrc::Vec2f, HistoryNode *>> frames =
		  getHistoryFrameLocations();
		const auto windowWidth	= (float)getWindowWidth();
		const auto windowHeight = (float)getWindowHeight();
		float boxLeft			= windowWidth - historyFrameWidth - historyFrameSep * 2;
		if (m_mouseDownPos.x() > boxLeft && m_mouseDownPos.x() < windowWidth) {
			// Iterate over frames to see if any contain the mouse
			for (const auto &frame : frames) {
				const lrc::Vec2i framePos  = std::get<0>(frame);
				const lrc::Vec2i frameSize = std::get<1>(frame);
				HistoryNode *node		   = std::get<2>(frame);

				if (m_mouseDownPos.x() >= framePos.x() &&
					m_mouseDownPos.x() < framePos.x() + frameSize.x() &&
					m_mouseDownPos.y() >= framePos.y() &&
					m_mouseDownPos.y() < framePos.y() + frameSize.y()) {
					// Mouse was within this frame, so set it as current
					m_historyNode		= node;
					m_renderer.config() = node->config();
					renderFractal(false); // Re-render the fractal
				}
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

	void MainWindow::mouseWheel(ci::app::MouseEvent event) {
		if (ImGui::GetIO().WantCaptureMouse) return;

		const json &settings = m_renderer.settings();
		if (m_mousePos.x() > settings["menus"]["history"]["frameWidth"]) {
			m_historyScrollTarget +=
			  event.getWheelIncrement() *
			  settings["menus"]["history"]["scrollSpeed"].get<float>();
		}
	}

	void MainWindow::keyDown(ci::app::KeyEvent event) {
		const json &settings = m_renderer.settings();

		if (m_showZoomBox) {
			const int64_t scrollSpeed = settings["menus"]["history"]["scrollSpeed"];

			switch (event.getCode()) {
				case ci::app::KeyEvent::KEY_RETURN: {
					zoomFractal(m_zoomBoxStart, m_zoomBoxEnd);
				}
				case ci::app::KeyEvent::KEY_ESCAPE: {
					m_showZoomBox = false;
					break;
				}
				case ci::app::KeyEvent::KEY_RIGHT: {
					m_zoomBoxStart.x(m_zoomBoxStart.x() + scrollSpeed);
					m_zoomBoxEnd.x(m_zoomBoxEnd.x() + scrollSpeed);
					break;
				}
				case ci::app::KeyEvent::KEY_LEFT: {
					m_zoomBoxStart.x(m_zoomBoxStart.x() - scrollSpeed);
					m_zoomBoxEnd.x(m_zoomBoxEnd.x() - scrollSpeed);
					break;
				}
				case ci::app::KeyEvent::KEY_UP: {
					m_zoomBoxStart.y(m_zoomBoxStart.y() - scrollSpeed);
					m_zoomBoxEnd.y(m_zoomBoxEnd.y() - scrollSpeed);
					break;
				}
				case ci::app::KeyEvent::KEY_DOWN: {
					m_zoomBoxStart.y(m_zoomBoxStart.y() + scrollSpeed);
					m_zoomBoxEnd.y(m_zoomBoxEnd.y() + scrollSpeed);
					break;
				}
			}
		}

		if (event.getCode() == ci::app::KeyEvent::KEY_z &&
			event.getModifiers() & ci::app::KeyEvent::CTRL_DOWN) {
			if (event.getModifiers() & ci::app::KeyEvent::SHIFT_DOWN) {
				redoLastMove();
			} else {
				undoLastMove();
			}
		}
	}

	void MainWindow::drawZoomBox(const lrc::Vec2f &start, const lrc::Vec2f &end) const {
		// Translucent inner box
		ci::gl::color(ci::ColorA(1, 0, 0, 0.333)); // Red with alpha
		ci::gl::drawSolidRect(ci::Rectf(start.x(), start.y(), end.x(), end.y()));
		// Small cross in the middle
		ci::gl::color(ci::ColorA(0, 0, 1, 0.333)); // Blue with alpha
		glu::drawCross((start + end) * 0.5f, 5.f, 2.f);
		// Bounding box
		ci::gl::color(ci::ColorA(1, 0, 0, 1)); // Red
		glu::drawStrokedRectangle(start, end, 5);
	}
} // namespace frac
