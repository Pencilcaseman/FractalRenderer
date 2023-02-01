#include <fractal/fractal.hpp>

namespace frac {
	void MainWindow::configureSettings() {
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

		// Set the default precision
		lrc::prec2(m_settings["renderConfig"]["precision"].get<int64_t>());

		// Load settings from settings JSON object
		m_renderConfig = {
		  m_settings["renderConfig"]["numThreads"].get<int64_t>(),
		  m_settings["renderConfig"]["maxIters"].get<int64_t>(),
		  m_settings["renderConfig"]["precision"].get<int64_t>(),
		  m_settings["renderConfig"]["bail"].get<LowPrecision>(),
		  m_settings["renderConfig"]["antiAlias"].get<int>(),

		  lrc::Vec2i(m_settings["renderConfig"]["imageSize"]["width"].get<int64_t>(),
					 m_settings["renderConfig"]["imageSize"]["height"].get<int64_t>()),
		  lrc::Vec2i(m_settings["renderConfig"]["boxSize"]["width"].get<int64_t>(),
					 m_settings["renderConfig"]["boxSize"]["height"].get<int64_t>()),

		  lrc::Vec<HighPrecision, 2>(m_settings["renderConfig"]["fracTopLeft"]["Re"].get<float>(),
									 m_settings["renderConfig"]["fracTopLeft"]["Im"].get<float>()),
		  lrc::Vec<HighPrecision, 2>(m_settings["renderConfig"]["fracSize"]["Re"].get<float>(),
									 m_settings["renderConfig"]["fracSize"]["Im"].get<float>()),
		  lrc::Vec<HighPrecision, 2>(0, 0)};

		m_renderConfig.originalFracSize = m_renderConfig.fracSize;

		for (const auto &color : m_settings["renderConfig"]["colorPalette"]) {
			m_renderConfig.palette.addColor(ColorPalette::ColorType(color["red"].get<float>(),
																	color["green"].get<float>(),
																	color["blue"].get<float>(),
																	color["alpha"].get<float>()));
		}

		m_fractal = std::make_unique<Mandelbrot>(m_renderConfig);
	}

	void MainWindow::configureWindow() {
		setFrameRate(-1);				  // Unlimited framerate
		ci::gl::enableVerticalSync(true); // Enable vertical sync to avoid tearing
		setWindowSize(1000, 800);		  // Set the initial window size

		// Set up rendering settings
		ci::gl::enableDepthWrite();
		ci::gl::enableDepthRead();
		glDepthFunc(GL_ALWAYS);
	}

	void MainWindow::configureImGui() {
		ImGui::Initialize();
		ImGui::StyleColorsDark();
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		ImGui::GetIO().FontGlobalScale = 1.0f;

		// Enable window docking
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	}

	void MainWindow::setup() {
		FRAC_LOG("Setup Called");

		configureSettings();
		configureWindow();
		configureImGui();

		regenerateSurfaces();
		renderFractal();
	}

	void MainWindow::stopRender() {
		m_haltRender = true;
		m_threadPool.wait_for_tasks();
		m_haltRender = false;
	}

	void MainWindow::cleanup() { stopRender(); }

	void MainWindow::drawFractal() {
		m_fractalTexture = ci::gl::Texture2d::create(m_fractalSurface);
		lrc::Vec2f drawPos(0, getWindowHeight() - m_renderConfig.imageSize.y());

		ci::gl::color(ci::ColorA(1, 1, 1, 1));
		ci::gl::draw(
		  m_fractalTexture,
		  ci::Rectf(drawPos, lrc::Vec2f(drawPos) + lrc::Vec2f(m_renderConfig.imageSize)));
	}

	void MainWindow::outlineRenderBoxes() {
		for (const auto &box : m_renderBoxes) {
			switch (box.state) {
				case RenderBoxState::None:
				case RenderBoxState::Rendered: continue;
				case RenderBoxState::Queued: ci::gl::color(ci::ColorA(0, 1, 0, 0.2)); break;
				case RenderBoxState::Rendering: ci::gl::color(ci::ColorA(1, 1, 0, 0.2)); break;
			}

			ci::ivec2 boxPos  = box.topLeft;
			ci::ivec2 boxSize = box.dimensions;
			boxPos.y += getWindowHeight() - m_renderConfig.imageSize.y();
			ci::gl::drawStrokedRect(ci::Rectf(boxPos, boxPos + boxSize), 1);
		}
	}

	void MainWindow::draw() {
		ci::gl::clear(ci::Color(0.2f, 0.2f, 0.2f));

		drawImGui();
		drawFractal();
		outlineRenderBoxes();

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

			HighPrecision re   = m_renderConfig.fracTopLeft.x() + m_renderConfig.fracSize.x() / 2;
			HighPrecision im   = m_renderConfig.fracTopLeft.y() + m_renderConfig.fracSize.y() / 2;
			HighPrecision zoom = m_renderConfig.originalFracSize.x() / m_renderConfig.fracSize.x();

			ImGui::Text("%s", fmt::format("Re:   {}", re).c_str());
			ImGui::Text("%s", fmt::format("Im:   {}", im).c_str());
			ImGui::Text("%s", fmt::format("Zoom: {:e}x", (double)zoom).c_str());

			double maxZoomExponent = m_renderConfig.precision / lrc::log2(10);
			ImGui::Text("%s", fmt::format("Max Zoom: e+{:.3f}", maxZoomExponent).c_str());
		}
		ImGui::End();

		// Fine movement window
		json fineMovement = m_settings["menus"]["fineMovement"];
		ImGui::SetNextWindowPos({(float)fineMovement["posX"], (float)fineMovement["posY"]},
								ImGuiCond_Once);
		ImGui::SetNextWindowSize({(float)fineMovement["width"], (float)fineMovement["height"]},
								 ImGuiCond_Once);
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

				sizeRe = m_renderConfig.originalFracSize.x() / zoom;
				sizeIm = m_renderConfig.originalFracSize.y() / zoom;
				moveFractalCenter(lrc::Vec<HighPrecision, 2>(re, im),
								  lrc::Vec<HighPrecision, 2>(sizeRe, sizeIm));
			}
		}
		ImGui::End();

		// Render configuration
		json renderConfig = m_settings["menus"]["renderConfig"];
		ImGui::SetNextWindowPos({(float)renderConfig["posX"], (float)renderConfig["posY"]},
								ImGuiCond_Once);
		ImGui::SetNextWindowSize({(float)renderConfig["width"], (float)renderConfig["height"]},
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

			static int64_t newThreads	= m_renderConfig.numThreads;
			static int64_t newIters		= m_renderConfig.maxIters;
			static int64_t newPrecision = m_renderConfig.precision;
			static int64_t newAntiAlias = m_renderConfig.antiAlias;

			ImGui::SliderScalar(
			  "Threads", ImGuiDataType_S64, &newThreads, &minThreads, &maxThreads);

			ImGui::SliderScalar(
			  "Anti Aliasing", ImGuiDataType_S64, &newAntiAlias, &minAntiAlias, &maxAntiAlias);

			ImGui::DragScalarN(
			  "Iterations", ImGuiDataType_S64, &newIters, 1, 100, &minIters, &maxIters);

			ImGui::DragScalarN(
			  "Precision", ImGuiDataType_S64, &newPrecision, 1, 32, &minPrecision, &maxPrecision);

			if (ImGui::Button("Apply")) {
				stopRender();
				m_renderConfig.numThreads = newThreads;
				m_renderConfig.maxIters	  = newIters;
				m_renderConfig.precision  = newPrecision;
				m_renderConfig.antiAlias  = newAntiAlias;
				lrc::prec2(newPrecision);
				m_fractal->updateRenderConfig(m_renderConfig);
				updateConfigPrecision();
				renderFractal();
			}
		}
		ImGui::End();
	}

	void MainWindow::moveFractalCorner(const lrc::Vec<HighPrecision, 2> &topLeft,
									   const lrc::Vec<HighPrecision, 2> &size) {
		m_renderConfig.fracTopLeft = topLeft;
		m_renderConfig.fracSize	   = size;
		m_fractal->updateRenderConfig(m_renderConfig);
		renderFractal();
	}

	void MainWindow::moveFractalCenter(const lrc::Vec<HighPrecision, 2> &center,
									   const lrc::Vec<HighPrecision, 2> &size) {
		moveFractalCorner(center - size / lrc::Vec<HighPrecision, 2>(2, 2), size);
	}

	void MainWindow::regenerateSurfaces() {
		FRAC_LOG("Regenerating Surfaces...");
		int64_t w		 = m_renderConfig.imageSize.x();
		int64_t h		 = m_renderConfig.imageSize.y();
		m_fractalSurface = ci::Surface((int32_t)w, (int32_t)h, true);
		m_fractalTexture = ci::gl::Texture2d::create(m_fractalSurface);
		FRAC_LOG("Surfaces regenerated");
	}

	void MainWindow::updateConfigPrecision() {
		int64_t prec = m_renderConfig.precision;
		HighPrecision highPrecTopLeftX(m_renderConfig.fracTopLeft.x(), prec);
		HighPrecision highPrecTopLeftY(m_renderConfig.fracTopLeft.y(), prec);
		HighPrecision highPrecFracSizeX(m_renderConfig.fracSize.x(), prec);
		HighPrecision highPrecFracSizeY(m_renderConfig.fracSize.y(), prec);
		HighPrecision highPrecOriginalFracSizeX(m_renderConfig.originalFracSize.x(), prec);
		HighPrecision highPrecOriginalFracSizeY(m_renderConfig.originalFracSize.y(), prec);

		m_renderConfig.fracTopLeft		= {highPrecTopLeftX, highPrecTopLeftY};
		m_renderConfig.fracSize			= {highPrecFracSizeX, highPrecFracSizeY};
		m_renderConfig.originalFracSize = {highPrecOriginalFracSizeX, highPrecOriginalFracSizeY};
	}

	void MainWindow::renderFractal() {
		if (m_threadPool.get_tasks_queued() > 0) {
			FRAC_WARN("Render already in progress. Halting...");
			m_haltRender = true;
			m_threadPool.wait_for_tasks();
			m_haltRender = false;
			FRAC_LOG("Render halted");
		}

		FRAC_LOG("Rendering Fractal...");

		m_renderBoxes.clear();
		m_threadPool.reset(m_renderConfig.numThreads);

		// Split the render into boxes to be rendered in parallel
		auto imageSize = m_renderConfig.imageSize;
		auto boxSize   = m_renderConfig.boxSize;

		// Round number of boxes up so the full image is covered
		auto numBoxes = lrc::Vec2i(lrc::ceil(lrc::Vec2f(imageSize) / lrc::Vec2f(boxSize)));

		m_renderBoxes.reserve(numBoxes.x() * numBoxes.y());

		for (int64_t i = 0; i < numBoxes.y(); ++i) {
			for (int64_t j = 0; j < numBoxes.x(); ++j) {
				lrc::Vec2i adjustedBoxSize(lrc::min(boxSize.x(), imageSize.x() - j * boxSize.x()),
										   lrc::min(boxSize.y(), imageSize.y() - i * boxSize.y()));
				RenderBox box {lrc::Vec2i(j, i) * boxSize, adjustedBoxSize, RenderBoxState::Queued};

				// renderBox(box);
				auto prevSize = (int64_t)m_renderBoxes.size();
				m_renderBoxes.emplace_back(box); // Must happen before pushing to render queue
				m_threadPool.push_task([this, box, prevSize]() { renderBox(box, prevSize); });
			}
		}

		FRAC_LOG("Fractal Complete...");
	}

	void MainWindow::renderBox(const RenderBox &box, int64_t boxIndex) {
		// Update the render box state
		m_renderBoxes[boxIndex].state = RenderBoxState::Rendering;

		HighVec2 fractalOrigin =
		  lrc::map(static_cast<HighVec2>(box.topLeft),
				   HighVec2({0, 0}),
				   static_cast<HighVec2>(m_renderConfig.imageSize),
				   m_renderConfig.fracTopLeft,
				   m_renderConfig.fracTopLeft + static_cast<HighVec2>(m_renderConfig.fracSize));

		HighVec2 step = m_renderConfig.fracSize / static_cast<HighVec2>(m_renderConfig.imageSize);

		int64_t aliasFactor		  = m_renderConfig.antiAlias;
		HighPrecision scaleFactor = HighPrecision(1) / static_cast<HighPrecision>(aliasFactor);
		HighVec2 aliasStepCorrect(scaleFactor, scaleFactor);

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

				ci::ColorA pix;

				if (m_renderConfig.precision <= 64) {
					pix = pixelColorLow(pixPos, aliasFactor, step, aliasStepCorrect);
				} else {
					pix = pixelColorHigh(pixPos, aliasFactor, step, aliasStepCorrect);
				}

				m_fractalSurface.setPixel(lrc::Vec2i(px, py), pix);
			}
		}

		// Update the render box state
		m_renderBoxes[boxIndex].state = RenderBoxState::Rendered;
	}

	ci::ColorA MainWindow::pixelColorLow(const LowVec2 &pixPos, int64_t aliasFactor,
										 const LowVec2 &step, const LowVec2 &aliasStepCorrect) {
		ci::ColorA pix(0, 0, 0, 1);

		for (int64_t aliasY = 0; aliasY < aliasFactor; ++aliasY) {
			for (int64_t aliasX = 0; aliasX < aliasFactor; ++aliasX) {
				auto pos = pixPos + step * LowVec2(aliasX, aliasY) * aliasStepCorrect;

				auto [iters, endPoint] =
				  m_fractal->iterCoordLow(lrc::Complex<LowPrecision>(pos.x(), pos.y()));
				if (endPoint.real() * endPoint.real() + endPoint.imag() * endPoint.imag() < 4) {
					pix += ci::ColorA(0, 0, 0, 1); // Probably in the set
				} else {
					pix += m_fractal->getColorLow(endPoint, iters); // Probably not in the set
				}
			}
		}

		return pix / static_cast<float>(aliasFactor * aliasFactor);
	}

	ci::ColorA MainWindow::pixelColorHigh(const HighVec2 &pixPos, int64_t aliasFactor,
										  const HighVec2 &step, const HighVec2 &aliasStepCorrect) {
		ci::ColorA pix(0, 0, 0, 1);

		for (int64_t aliasY = 0; aliasY < aliasFactor; ++aliasY) {
			for (int64_t aliasX = 0; aliasX < aliasFactor; ++aliasX) {
				auto pos = pixPos + step * HighVec2(aliasX, aliasY) * aliasStepCorrect;

				auto [iters, endPoint] =
				  m_fractal->iterCoordHigh(lrc::Complex<HighPrecision>(pos.x(), pos.y()));
				if (endPoint.real() * endPoint.real() + endPoint.imag() * endPoint.imag() < 4) {
					pix += ci::ColorA(0, 0, 0, 1); // Probably in the set
				} else {
					pix += m_fractal->getColorHigh(endPoint, iters); // Probably not in the set
				}
			}
		}

		return pix / static_cast<float>(aliasFactor * aliasFactor);
	}

	void MainWindow::mouseMove(ci::app::MouseEvent event) { m_mousePos = event.getPos(); }

	void MainWindow::mouseDown(ci::app::MouseEvent event) {
		// Don't capture mouse events if ImGui wants them
		if (ImGui::GetIO().WantCaptureMouse) return;

		m_mouseDown	   = true;
		m_mouseDownPos = event.getPos();
	}

	void MainWindow::mouseDrag(ci::app::MouseEvent event) {
		if (ImGui::GetIO().WantCaptureMouse) return;

		m_mousePos = event.getPos();
	}

	void MainWindow::mouseUp(ci::app::MouseEvent event) {
		if (ImGui::GetIO().WantCaptureMouse) return;

		m_mouseDown = false;

		// Resize the fractal area
		HighVec2 imageSize		   = m_renderConfig.imageSize;
		HighVec2 imageOrigin	   = {0, getWindowHeight() - imageSize.y()};
		HighVec2 mouseStartInImage = HighVec2(m_mouseDownPos) - imageOrigin;

		HighVec2 mouseDelta = HighVec2(m_mousePos) - HighVec2(m_mouseDownPos);
		HighVec2 newFracPos = lrc::map(mouseStartInImage,
									   HighVec2(0, 0),
									   imageSize,
									   m_renderConfig.fracTopLeft,
									   m_renderConfig.fracTopLeft + m_renderConfig.fracSize);

		HighVec2 newFracSize =
		  lrc::map(mouseDelta, HighVec2(0, 0), imageSize, HighVec2(0, 0), m_renderConfig.fracSize);

		// Copy the pixels from the selected region to the new region
		int64_t imgWidth  = m_renderConfig.imageSize.x();
		int64_t imgHeight = m_renderConfig.imageSize.y();
		int64_t index	  = 0;
		std::vector<ci::ColorA> newPixels(imgWidth * imgHeight);
		auto mouseStartInImageLow =
		  m_mouseDownPos - lrc::Vec2i {0, getWindowHeight() - m_renderConfig.imageSize.y()};
		for (int64_t y = 0; y < imgHeight; ++y) {
			for (int64_t x = 0; x < imgWidth; ++x) {
				int64_t pixX = lrc::map(x,
										0.f,
										imgWidth,
										mouseStartInImageLow.x(),
										mouseStartInImageLow.x() + (float)mouseDelta.x());
				int64_t pixY = lrc::map(y,
										0.f,
										imgHeight,
										mouseStartInImageLow.y(),
										mouseStartInImageLow.y() + (float)mouseDelta.y());

				newPixels[index++] = m_fractalSurface.getPixel({pixX, pixY});
			}
		}

		// Write the new pixels to the surface
		index = 0;
		for (int64_t y = 0; y < imgHeight; ++y) {
			for (int64_t x = 0; x < imgWidth; ++x) {
				m_fractalSurface.setPixel({x, y}, newPixels[index++]);
			}
		}

		m_renderConfig.fracTopLeft = newFracPos;
		m_renderConfig.fracSize	   = newFracSize;
		m_fractal->updateRenderConfig(m_renderConfig);

		FRAC_LOG(fmt::format("ImageOrigin: {}", imageOrigin));
		FRAC_LOG(fmt::format("MouseStartInImage: {}", mouseStartInImage));
		FRAC_LOG(fmt::format("MouseDelta: {}", mouseDelta));
		FRAC_LOG(fmt::format("NewFracPos: {}", newFracPos));
		FRAC_LOG(fmt::format("NewFracSize: {}", newFracSize));
		FRAC_LOG(fmt::format("NewFracPosPrec: {}", newFracPos.x().get_prec()));
		FRAC_LOG(fmt::format("NewFracSizePrec: {}", newFracSize.x().get_prec()));

		renderFractal();
	}
} // namespace frac
