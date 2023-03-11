#include <fractal/fractal.hpp>

namespace frac {
	FractalRenderer::~FractalRenderer() { stopRender(); }

	void FractalRenderer::setConfig(const json &config) {
		m_settings = config;

		try {
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

			  lrc::Vec<HighPrecision, 2>(
				m_settings["renderConfig"]["fracTopLeft"]["Re"].get<float>(),
				m_settings["renderConfig"]["fracTopLeft"]["Im"].get<float>()),
			  lrc::Vec<HighPrecision, 2>(m_settings["renderConfig"]["fracSize"]["Re"].get<float>(),
										 m_settings["renderConfig"]["fracSize"]["Im"].get<float>()),
			  lrc::Vec<HighPrecision, 2>(0, 0)};

			m_renderConfig.originalFracSize = m_renderConfig.fracSize;

			// Load the colour palette from the JSON object
			for (const auto &color : m_settings["renderConfig"]["colorPalette"]) {
				m_renderConfig.palette.addColor(
				  ColorPalette::ColorType(color["red"].get<float>(),
										  color["green"].get<float>(),
										  color["blue"].get<float>(),
										  color["alpha"].get<float>()));
			}

			m_fractal = std::make_unique<Mandelbrot>(m_renderConfig);
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
		m_fractal->updateRenderConfig(m_renderConfig);
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

	void FractalRenderer::renderBox(const RenderBox &box, int64_t boxIndex) {
		// Update the render box state
		m_renderBoxes[boxIndex].state = RenderBoxState::Rendering;
		double start				  = lrc::now();

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

		bool blackEdges = true;

		// TODO: MAKE THIS A FUNCTION!!!!!!!!!!

		if (m_haltRender) return;

		// Render the top edge
		for (int64_t px = box.topLeft.x(); px < box.topLeft.x() + box.dimensions.x(); ++px) {
			// Anti-aliasing
			auto pixPos = fractalOrigin + step * HighVec2(px - box.topLeft.x(), 0);

			ci::ColorA pix;

			if (m_renderConfig.precision <= 64) {
				pix = pixelColorLow(pixPos, aliasFactor, step, aliasStepCorrect);
			} else {
				pix = pixelColorHigh(pixPos, aliasFactor, step, aliasStepCorrect);
			}

			if (blackEdges && (pix.r != 0 && pix.g != 0 && pix.b != 0)) { blackEdges = false; }

			m_fractalSurface.setPixel(lrc::Vec2i(px, box.topLeft.y()), pix);
		}

		if (m_haltRender) return;

		// Render the right edge
		for (int64_t py = box.topLeft.y(); py < box.topLeft.y() + box.dimensions.y(); ++py) {
			// Anti-aliasing
			auto pixPos = fractalOrigin + step * HighVec2(box.dimensions.x(), py - box.topLeft.y());

			ci::ColorA pix;

			if (m_renderConfig.precision <= 64) {
				pix = pixelColorLow(pixPos, aliasFactor, step, aliasStepCorrect);
			} else {
				pix = pixelColorHigh(pixPos, aliasFactor, step, aliasStepCorrect);
			}

			if (blackEdges && (pix.r != 0 && pix.g != 0 && pix.b != 0)) { blackEdges = false; }

			m_fractalSurface.setPixel(lrc::Vec2i(box.topLeft.x() + box.dimensions.x() - 1, py),
									  pix);
		}

		if (m_haltRender) return;

		// Render the bottom edge
		for (int64_t px = box.topLeft.x(); px < box.topLeft.x() + box.dimensions.x(); ++px) {
			// Anti-aliasing
			auto pixPos =
			  fractalOrigin + step * HighVec2(px - box.topLeft.x(), box.dimensions.y() - 1);

			ci::ColorA pix;

			if (m_renderConfig.precision <= 64) {
				pix = pixelColorLow(pixPos, aliasFactor, step, aliasStepCorrect);
			} else {
				pix = pixelColorHigh(pixPos, aliasFactor, step, aliasStepCorrect);
			}

			if (blackEdges && (pix.r != 0 && pix.g != 0 && pix.b != 0)) { blackEdges = false; }

			m_fractalSurface.setPixel(lrc::Vec2i(px, box.topLeft.y() + box.dimensions.y() - 1),
									  pix);
		}

		if (m_haltRender) return;

		// Render the left edge
		for (int64_t py = box.topLeft.y(); py < box.topLeft.y() + box.dimensions.y(); ++py) {
			// Anti-aliasing
			auto pixPos = fractalOrigin +
						  step * HighVec2(box.topLeft.x() - box.topLeft.x(), py - box.topLeft.y());

			ci::ColorA pix;

			if (m_renderConfig.precision <= 64) {
				pix = pixelColorLow(pixPos, aliasFactor, step, aliasStepCorrect);
			} else {
				pix = pixelColorHigh(pixPos, aliasFactor, step, aliasStepCorrect);
			}

			if (blackEdges && (pix.r != 0 && pix.g != 0 && pix.b != 0)) { blackEdges = false; }

			m_fractalSurface.setPixel(lrc::Vec2i(box.topLeft.x(), py), pix);
		}

		if (blackEdges) {
			for (int64_t py = box.topLeft.y() + 1; py < box.topLeft.y() + box.dimensions.y() - 1;
				 ++py) {
				for (int64_t px = box.topLeft.x() + 1;
					 px < box.topLeft.x() + box.dimensions.x() - 1;
					 ++px) {
					m_fractalSurface.setPixel(lrc::Vec2i(px, py), ci::ColorA {1, 0, 0, 1});
				}
			}
		} else {
			// Make the primary axis of iteration the x-axis to improve cache efficiency and
			// increase performance.
			for (int64_t py = box.topLeft.y() + 1; py < box.topLeft.y() + box.dimensions.y() - 1;
				 ++py) {
				// Quick return if required. Without this, the
				// render threads will continue running after the
				// application is closed, leading to weird behaviour.
				if (m_haltRender) return;

				for (int64_t px = box.topLeft.x() + 1;
					 px < box.topLeft.x() + box.dimensions.x() - 1;
					 ++px) {
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
		}

		// Update the render box state
		m_renderBoxes[boxIndex].state	   = RenderBoxState::Rendered;
		m_renderBoxes[boxIndex].renderTime = lrc::now() - start;
	}

	ci::ColorA FractalRenderer::pixelColorLow(const LowVec2 &pixPos, int64_t aliasFactor,
											  const LowVec2 &step,
											  const LowVec2 &aliasStepCorrect) {
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

	ci::ColorA FractalRenderer::pixelColorHigh(const HighVec2 &pixPos, int64_t aliasFactor,
											   const HighVec2 &step,
											   const HighVec2 &aliasStepCorrect) {
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

	void FractalRenderer::updateRenderConfig() { m_fractal->updateRenderConfig(m_renderConfig); }

	void FractalRenderer::regenerateSurface() {
		FRAC_LOG("Regenerating Surface...");
		int64_t w		 = m_renderConfig.imageSize.x();
		int64_t h		 = m_renderConfig.imageSize.y();
		m_fractalSurface = ci::Surface((int32_t)w, (int32_t)h, true);
		FRAC_LOG("Surface regenerated");
	}

	void FractalRenderer::updateConfigPrecision() {
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

	const RenderConfig &FractalRenderer::config() const { return m_renderConfig; }
	RenderConfig &FractalRenderer::config() { return m_renderConfig; }

	const std::vector<RenderBox> &FractalRenderer::renderBoxes() const { return m_renderBoxes; }
	std::vector<RenderBox> &FractalRenderer::renderBoxes() { return m_renderBoxes; }

	const json &FractalRenderer::settings() const { return m_settings; }
	json &FractalRenderer::settings() { return m_settings; }

	const ci::Surface &FractalRenderer::surface() const { return m_fractalSurface; }
	ci::Surface &FractalRenderer::surface() { return m_fractalSurface; }
} // namespace frac
