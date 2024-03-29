#include <fractal/fractal.hpp>

namespace frac {
	FractalRenderer::FractalRenderer(const json &config) { setConfig(config); }
	FractalRenderer::~FractalRenderer() { stopRender(); }

	void FractalRenderer::setConfig(const json &config) {
		m_settings = config;

		try {
			// Set the default precision
			lrc::prec2(m_settings["renderConfig"]["precision"].get<int64_t>());

			// Load settings from settings JSON object
			m_renderConfig = RenderConfig {
			  m_settings["renderConfig"]["numThreads"].get<int64_t>(),
			  m_settings["renderConfig"]["maxIters"].get<int64_t>(),
			  m_settings["renderConfig"]["precision"].get<int64_t>(),
			  0, // Bailout value -- Defaults set below. Can be reset from the GUI
			  m_settings["renderConfig"]["antiAlias"].get<int>(),

			  lrc::Vec2i(
				m_settings["renderConfig"]["imageSize"]["width"].get<int64_t>(),
				m_settings["renderConfig"]["imageSize"]["height"].get<int64_t>()),
			  lrc::Vec2i(m_settings["renderConfig"]["boxSize"]["width"].get<int64_t>(),
						 m_settings["renderConfig"]["boxSize"]["height"].get<int64_t>()),

			  {0, 0}, // Null coordinates and dimensions -- Defaults are set below and
			  {0, 0}, // can be reset from the GUI

			  lrc::Vec<HighPrecision, 2>(0, 0),

			  {}, // Default for now -- colors added later

			  m_settings["renderConfig"]["draftRender"].get<bool>(),
			  m_settings["renderConfig"]["draftInc"].get<int64_t>()};

			std::string fractalOriginReStr =
			  m_settings["renderConfig"]["fractalOrigin"]["Re"];
			std::string fractalOriginImStr =
			  m_settings["renderConfig"]["fractalOrigin"]["Im"];

			std::string fractalSizeReStr =
			  m_settings["renderConfig"]["fractalSize"]["Re"];
			std::string fractalSizeImStr =
			  m_settings["renderConfig"]["fractalSize"]["Im"];

			HighPrecision fractalOriginRe, fractalOriginIm, fractalSizeRe, fractalSizeIm;

			scn::scan(fractalOriginReStr, "{}", fractalOriginRe);
			scn::scan(fractalOriginImStr, "{}", fractalOriginIm);

			scn::scan(fractalSizeReStr, "{}", fractalSizeRe);
			scn::scan(fractalSizeImStr, "{}", fractalSizeIm);

			HighVec2 fracOrigin(fractalOriginRe, fractalOriginIm);
			HighVec2 fracSize(fractalSizeRe, fractalSizeIm);

			m_renderConfig.fracTopLeft = fracOrigin - fracSize / 2;
			m_renderConfig.fracSize	   = fracSize;

			m_renderConfig.originalFracSize = m_renderConfig.fracSize;

			// Load the colour palettes from the JSON object
			for (const auto &palette : m_settings["renderConfig"]["colorPalettes"]) {
				// If no palette name has been set, set it to the first palette in the
				// list
				if (m_paletteName.length() == 0) m_paletteName = palette["name"];

				ColorPalette tmp;
				for (const auto &color : palette["colors"]) {
					tmp.addColor(ColorPalette::ColorType(color["red"].get<float>(),
														 color["green"].get<float>(),
														 color["blue"].get<float>(),
														 color["alpha"].get<float>()));
				}
				m_renderConfig.palettes[palette["name"]] = tmp;
			}
		} catch (std::exception &e) {
			FRAC_LOG(fmt::format("Failed to load settings: {}", e.what()));
			stopRender();
		}
	}

	void FractalRenderer::stopRender() {
		m_haltRender = true;
		m_threadPool.wait_for_tasks();
		m_haltRender = false;
	}

	void FractalRenderer::moveFractalCorner(const lrc::Vec<HighPrecision, 2> &topLeft,
											const lrc::Vec<HighPrecision, 2> &size) {
		m_renderConfig.fracTopLeft = topLeft;
		m_renderConfig.fracSize	   = size;
		if (m_fractal) m_fractal->updateRenderConfig(m_renderConfig);
	}

	void FractalRenderer::moveFractalCenter(const lrc::Vec<HighPrecision, 2> &center,
											const lrc::Vec<HighPrecision, 2> &size) {
		moveFractalCorner(center - size / lrc::Vec<HighPrecision, 2>(2, 2), size);
	}

	void FractalRenderer::renderFractal() {
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
		auto numBoxes =
		  lrc::Vec2i(lrc::ceil(lrc::Vec2f(imageSize) / lrc::Vec2f(boxSize)));

		m_renderBoxes.reserve(numBoxes.x() * numBoxes.y());

		// Iterate over all boxes
		for (int64_t i = 0; i < numBoxes.y(); ++i) {
			for (int64_t j = 0; j < numBoxes.x(); ++j) {
				lrc::Vec2i adjustedBoxSize(
				  lrc::min(boxSize.x(), imageSize.x() - j * boxSize.x()),
				  lrc::min(boxSize.y(), imageSize.y() - i * boxSize.y()));
				RenderBox box {lrc::Vec2i(j, i) * boxSize,
							   adjustedBoxSize,
							   m_renderConfig.draftRender,
							   m_renderConfig.draftInc,
							   RenderBoxState::Queued};

				auto prevSize = (int64_t)m_renderBoxes.size();

				// Must happen before pushing to render queue
				m_renderBoxes.emplace_back(box);

				m_threadPool.push_task(
				  [this, box, prevSize]() { renderBox(box, prevSize); });
			}
		}

		FRAC_LOG("Fractal Complete...");
	}

	void FractalRenderer::renderBox(const RenderBox &box, int64_t boxIndex) {
		// Update the render box state
		m_renderBoxes[boxIndex].state = RenderBoxState::Rendering;
		const double start			  = lrc::now();

		const int64_t inc = box.draftRender ? box.draftInc : 1;

		HighVec2 fractalOrigin = lrc::map(
		  static_cast<HighVec2>(box.topLeft),
		  HighVec2({0, 0}),
		  static_cast<HighVec2>(m_renderConfig.imageSize),
		  m_renderConfig.fracTopLeft,
		  m_renderConfig.fracTopLeft + static_cast<HighVec2>(m_renderConfig.fracSize));

		HighVec2 step =
		  m_renderConfig.fracSize / static_cast<HighVec2>(m_renderConfig.imageSize);

		int64_t aliasFactor = m_renderConfig.antiAlias;
		if (box.draftRender) aliasFactor = 1; // No anti-aliasing for drafts

		HighPrecision scaleFactor =
		  HighPrecision(1) / static_cast<HighPrecision>(aliasFactor);
		HighVec2 aliasStepCorrect(scaleFactor, scaleFactor);

		const size_t supportedOptimisations = m_fractal->supportedOptimisations();
		const bool supportsOutlining =
		  supportedOptimisations & optimisations::OUTLINE_OPTIMISATION;

		bool blackEdges = true; // Assume edges are black to begin with

		if (m_haltRender) return;

		if (box.draftRender) {
			for (int64_t py = box.topLeft.y(); py < box.topLeft.y() + box.dimensions.y();
				 ++py) {
				for (int64_t px = box.topLeft.x();
					 px < box.topLeft.x() + box.dimensions.x();
					 ++px) {
					m_fractalSurface.setPixel(lrc::Vec2i(px, py),
											  ci::ColorA {0.2, 0, 0.2, 0.5});
				}
			}
		}

		if (supportsOutlining) {
			for (int64_t i = 0; i < 4; ++i) {
				blackEdges &= renderEdge(
				  box, fractalOrigin, aliasFactor, step, aliasStepCorrect, inc, i);
			}
		}

		if (supportsOutlining && blackEdges) {
			for (int64_t py = box.topLeft.y() + 1;
				 py < box.topLeft.y() + box.dimensions.y() - 1;
				 py += inc) {
				for (int64_t px = box.topLeft.x() + 1;
					 px < box.topLeft.x() + box.dimensions.x() - 1;
					 px += inc) {
					m_fractalSurface.setPixel(lrc::Vec2i(px, py),
											  ci::ColorA {0, 0, 0, 1});
				}
			}
		} else {
			int64_t offset = 0;
			if (supportsOutlining) offset = 1;

			// Make the primary axis of iteration the x-axis to improve cache
			// efficiency and increase performance.
			for (int64_t py = box.topLeft.y() + offset;
				 py < box.topLeft.y() + box.dimensions.y() - offset;
				 py += inc) {
				// Quick return if required. Without this, the
				// render threads will continue running after the
				// application is closed, leading to weird behaviour.
				if (m_haltRender) return;

				for (int64_t px = box.topLeft.x() + offset;
					 px < box.topLeft.x() + box.dimensions.x() - offset;
					 px += inc) {
					// Anti-aliasing
					auto pixPos = fractalOrigin + step * HighVec2(px - box.topLeft.x(),
																  py - box.topLeft.y());

					ci::ColorA pix;

					if (m_renderConfig.precision <= 64) {
						pix = pixelColorLow(pixPos, aliasFactor, step, aliasStepCorrect);
					} else {
						pix = pixelColorHigh(pixPos, aliasFactor, step, aliasStepCorrect);
					}

					m_fractalSurface.setPixel(lrc::Vec2i(px, py), pix);
				}
			}
		}

		// Update the render box state
		m_renderBoxes[boxIndex].state	   = RenderBoxState::Rendered;
		m_renderBoxes[boxIndex].renderTime = lrc::now() - start;
	}

	bool FractalRenderer::renderEdge(const RenderBox &box, const HighVec2 &fractalOrigin,
									 int64_t aliasFactor, const HighVec2 &step,
									 const HighVec2 &aliasStepCorrect, int64_t inc,
									 int64_t edge) {
		bool edgesInSet = true;
		if (edge & 1) { // Edge is 1 or 3 -> Right or left
			int64_t px = 0;
			if (edge == 3) px = box.dimensions.x() - 1;

			for (int64_t py = box.topLeft.y(); py < box.topLeft.y() + box.dimensions.y();
				 py += inc) {
				// Anti-aliasing
				auto pixPos = fractalOrigin + step * HighVec2(px, py - box.topLeft.y());

				ci::ColorA pix;

				if (m_renderConfig.precision <= 64) {
					pix = pixelColorLow(pixPos, aliasFactor, step, aliasStepCorrect);
				} else {
					pix = pixelColorHigh(pixPos, aliasFactor, step, aliasStepCorrect);
				}

				if (pix.r != 0 || pix.g != 0 || pix.b != 0) edgesInSet = false;

				m_fractalSurface.setPixel(lrc::Vec2i(box.topLeft.x() + px, py), pix);
			}
		} else { // Edge is 0 or 2 -> Top or bottom
			int64_t py = 0;
			if (edge == 2) py = box.dimensions.y() - 1;

			for (int64_t px = box.topLeft.x(); px < box.topLeft.x() + box.dimensions.x();
				 px += inc) {
				// Anti-aliasing
				auto pixPos = fractalOrigin + step * HighVec2(px - box.topLeft.x(), py);

				ci::ColorA pix;

				if (m_renderConfig.precision <= 64) {
					pix = pixelColorLow(pixPos, aliasFactor, step, aliasStepCorrect);
				} else {
					pix = pixelColorHigh(pixPos, aliasFactor, step, aliasStepCorrect);
				}

				if (pix.r != 0 || pix.g != 0 || pix.b != 0) edgesInSet = false;

				m_fractalSurface.setPixel(lrc::Vec2i(px, box.topLeft.y() + py), pix);
			}
		}
		return edgesInSet;
	}

	ci::ColorA FractalRenderer::pixelColorLow(const LowVec2 &pixPos, int64_t aliasFactor,
											  const LowVec2 &step,
											  const LowVec2 &aliasStepCorrect) {
		ci::ColorA pix(0, 0, 0, 1);

		const ColorPalette &palette = m_renderConfig.palettes[m_paletteName];

		for (int64_t aliasY = 0; aliasY < aliasFactor; ++aliasY) {
			for (int64_t aliasX = 0; aliasX < aliasFactor; ++aliasX) {
				auto pos = pixPos + step * LowVec2(aliasX, aliasY) * aliasStepCorrect;
				auto [iters, endPoint] =
				  m_fractal->iterCoordLow(lrc::Complex<LowPrecision>(pos.x(), pos.y()));
				pix += m_fractal->getColorLow(endPoint, iters, palette, m_colorFuncLow);
			}
		}

		return pix / static_cast<float>(aliasFactor * aliasFactor);
	}

	ci::ColorA FractalRenderer::pixelColorHigh(const HighVec2 &pixPos,
											   int64_t aliasFactor, const HighVec2 &step,
											   const HighVec2 &aliasStepCorrect) {
		ci::ColorA pix(0, 0, 0, 1);

		const ColorPalette &palette = m_renderConfig.palettes[m_paletteName];

		for (int64_t aliasY = 0; aliasY < aliasFactor; ++aliasY) {
			for (int64_t aliasX = 0; aliasX < aliasFactor; ++aliasX) {
				auto pos = pixPos + step * HighVec2(aliasX, aliasY) * aliasStepCorrect;
				auto [iters, endPoint] =
				  m_fractal->iterCoordHigh(lrc::Complex<HighPrecision>(pos.x(), pos.y()));
				pix += m_fractal->getColorHigh(endPoint, iters, palette, m_colorFuncHigh);
			}
		}

		return pix / static_cast<float>(aliasFactor * aliasFactor);
	}

	void FractalRenderer::updateRenderConfig() {
		m_fractal->updateRenderConfig(m_renderConfig);
	}

	void FractalRenderer::regenerateSurface() {
		FRAC_LOG("Regenerating Surface...");
		int64_t w		 = m_renderConfig.imageSize.x();
		int64_t h		 = m_renderConfig.imageSize.y();
		m_fractalSurface = ci::Surface((int32_t)w, (int32_t)h, true);
		FRAC_LOG("Surface regenerated");
	}

	void FractalRenderer::updateFractalType(const std::shared_ptr<Fractal> &fractal) {
		m_fractal = fractal;
		m_fractal->updateRenderConfig(m_renderConfig);
	}

	void FractalRenderer::updateConfigPrecision() {
		int64_t prec = m_renderConfig.precision;
		HighPrecision highPrecTopLeftX(m_renderConfig.fracTopLeft.x(), prec);
		HighPrecision highPrecTopLeftY(m_renderConfig.fracTopLeft.y(), prec);
		HighPrecision highPrecFracSizeX(m_renderConfig.fracSize.x(), prec);
		HighPrecision highPrecFracSizeY(m_renderConfig.fracSize.y(), prec);
		HighPrecision highPrecOriginalFracSizeX(m_renderConfig.originalFracSize.x(),
												prec);
		HighPrecision highPrecOriginalFracSizeY(m_renderConfig.originalFracSize.y(),
												prec);

		m_renderConfig.fracTopLeft		= {highPrecTopLeftX, highPrecTopLeftY};
		m_renderConfig.fracSize			= {highPrecFracSizeX, highPrecFracSizeY};
		m_renderConfig.originalFracSize = {highPrecOriginalFracSizeX,
										   highPrecOriginalFracSizeY};
	}

	RenderBoxTimeStats FractalRenderer::boxTimeStats() const {
		double min	 = 1e10;
		double max	 = -1e10;
		double total = 0;
		size_t count = 0;

		for (const auto &box : m_renderBoxes) {
			if (box.renderTime == 0) continue;

			if (box.renderTime < min) min = box.renderTime;
			if (box.renderTime > max) max = box.renderTime;
			total += box.renderTime;
			count += 1;
		}

		double average		  = total / (double)count;
		size_t remainingBoxes = m_renderBoxes.size() - count;
		double remainingTime =
		  ((double)remainingBoxes * average) / (double)m_renderConfig.numThreads;

		return {min, max, average, remainingTime};
	}

	std::vector<std::string> FractalRenderer::getColorFuncs() const {
		const auto &colorFuncs = m_fractal->getLowPrecColoringAlgorithms();
		std::vector<std::string> ret;
		for (const auto &[name, palette] : colorFuncs) { ret.push_back(name); }
		return ret;
	}

	void FractalRenderer::setColorFunc(const std::string &name) {
		m_colorFuncName = name;
		m_colorFuncLow	= m_fractal->getLowPrecColoringAlgorithms().at(name);
		m_colorFuncHigh = m_fractal->getHighPrecColoringAlgorithms().at(name);
	}

	std::vector<std::string> FractalRenderer::getPaletteNames() const {
		std::vector<std::string> ret;
		for (const auto &[name, palette] : m_renderConfig.palettes) {
			ret.push_back(name);
		}
		return ret;
	}

	void FractalRenderer::setPaletteName(const std::string &name) {
		m_paletteName = name;
	}

	const RenderConfig &FractalRenderer::config() const { return m_renderConfig; }
	RenderConfig &FractalRenderer::config() { return m_renderConfig; }

	const std::string &FractalRenderer::paletteName() const { return m_paletteName; }

	const std::vector<RenderBox> &FractalRenderer::renderBoxes() const {
		return m_renderBoxes;
	}
	std::vector<RenderBox> &FractalRenderer::renderBoxes() { return m_renderBoxes; }

	const json &FractalRenderer::settings() const { return m_settings; }
	json &FractalRenderer::settings() { return m_settings; }

	const ci::Surface &FractalRenderer::surface() const { return m_fractalSurface; }
	ci::Surface &FractalRenderer::surface() { return m_fractalSurface; }

	void FractalRenderer::exportImage(const std::string &path) const {
		ci::writeImage(path, m_fractalSurface);
	}

	void FractalRenderer::exportSettings(const std::string &path) const {
		// All updates to the render configuration must be copied to the settings
		json settings = m_settings;

		settings["renderConfig"]["numThreads"] = m_renderConfig.numThreads;
		settings["renderConfig"]["maxIters"]   = m_renderConfig.maxIters;
		settings["renderConfig"]["precision"]  = m_renderConfig.precision;
		settings["renderConfig"]["antiAlias"]  = m_renderConfig.antiAlias;

		settings["renderConfig"]["imageSize"]["width"]	= m_renderConfig.imageSize.x();
		settings["renderConfig"]["imageSize"]["height"] = m_renderConfig.imageSize.y();

		settings["renderConfig"]["boxSize"]["width"]  = m_renderConfig.boxSize.x();
		settings["renderConfig"]["boxSize"]["height"] = m_renderConfig.boxSize.y();

		settings["renderConfig"]["draftRender"] = m_renderConfig.draftRender;
		settings["renderConfig"]["draftInc"]	= m_renderConfig.draftInc;

		HighVec2 size	= m_renderConfig.fracSize;
		HighVec2 origin = m_renderConfig.fracTopLeft + size / 2;
		settings["renderConfig"]["fractalOrigin"]["Re"] = fmt::format("{}", origin.x());
		settings["renderConfig"]["fractalOrigin"]["Im"] = fmt::format("{}", origin.y());

		settings["renderConfig"]["fractalSize"]["Re"] = fmt::format("{}", size.x());
		settings["renderConfig"]["fractalSize"]["Im"] = fmt::format("{}", size.y());

		settings["renderConfig"]["bail"]		 = m_renderConfig.bail;
		settings["renderConfig"]["fractalType"]	 = m_fractal->name();
		settings["renderConfig"]["colorFunc"]	 = m_colorFuncName;
		settings["renderConfig"]["colorPalette"] = m_paletteName;

		std::fstream file(path, std::ios::out);
		file << settings.dump(4);
		file.close();
	}
} // namespace frac
