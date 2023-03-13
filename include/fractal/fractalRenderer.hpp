#pragma once

namespace frac {
	class FractalRenderer {
	public:
		FractalRenderer() = default;
		explicit FractalRenderer(const json &config);
		~FractalRenderer();

		void setConfig(const json &config);

		void stopRender();

		void moveFractalCorner(const lrc::Vec<HighPrecision, 2> &topLeft,
							   const lrc::Vec<HighPrecision, 2> &size);

		void moveFractalCenter(const lrc::Vec<HighPrecision, 2> &center,
							   const lrc::Vec<HighPrecision, 2> &size);

		/// Render the fractal into the fractal surface, and copy that to the
		/// fractal surface to be drawn. This will be executed on a separate thread
		/// in order to keep the UI updating
		void renderFractal();

		/// Render a sub-section of the fractal, defined by the \p box variable. This is
		/// intended to be used within the call queue to render multiple sections in
		/// parallel \param box
		void renderBox(const RenderBox &box, int64_t boxIndex = -1);

		ci::ColorA pixelColorLow(const LowVec2 &pixPos, int64_t aliasFactor,
								 const LowVec2 &step, const LowVec2 &aliasStepCorrect);

		ci::ColorA pixelColorHigh(const HighVec2 &pixPos, int64_t aliasFactor,
								  const HighVec2 &step, const HighVec2 &aliasStepCorrect);

		void updateRenderConfig();
		void updateConfigPrecision();
		void regenerateSurface();

		LIBRAPID_NODISCARD RenderBoxTimeStats boxTimeStats() const;

		LIBRAPID_NODISCARD const RenderConfig &config() const;
		LIBRAPID_NODISCARD RenderConfig &config();

		LIBRAPID_NODISCARD const std::vector<RenderBox> &renderBoxes() const;
		LIBRAPID_NODISCARD std::vector<RenderBox> &renderBoxes();

		LIBRAPID_NODISCARD const json &settings() const;
		LIBRAPID_NODISCARD json &settings();

		LIBRAPID_NODISCARD const ci::Surface &surface() const;
		LIBRAPID_NODISCARD ci::Surface &surface();

	private:
		RenderConfig m_renderConfig;		// The settings for the fractal renderer
		ci::Surface m_fractalSurface;		// The surface that the fractal is rendered to
		json m_settings;					// The settings for the fractal
		std::unique_ptr<Fractal> m_fractal; // The fractal to render
		ThreadPool m_threadPool;			// Pool for render threads

		std::vector<RenderBox> m_renderBoxes; // The state of each render box
		bool m_haltRender = false; // Used to gracefully stop the render threads
	};
} // namespace frac
