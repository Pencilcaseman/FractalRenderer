#pragma once

namespace frac {
	namespace optimisations {
		constexpr size_t OUTLINE_OPTIMISATION = 0x000000000000001;
	}

	class FractalRenderer {
	public:
		FractalRenderer() = default;

		/// Construct a new renderer object from a JSON config object
		/// \param config The JSON config object
		explicit FractalRenderer(const json &config);

		~FractalRenderer();

		/// Set the fractal renderer config
		/// \param config JSON object
		void setConfig(const json &config);

		/// Stop the renderer gracefully and wait for all threads to rejoin main
		void stopRender();

		/// Set the complex-valued coordinate of the top-left corner of the fractal and
		/// its size
		/// \param topLeft Top-left corner
		/// \param size Size of the fractal
		/// \see moveFractalCenter
		void moveFractalCorner(const lrc::Vec<HighPrecision, 2> &topLeft,
							   const lrc::Vec<HighPrecision, 2> &size);

		/// Set the complex-valued coordinate of the center of the fractal and its size
		/// \param center Center of the fractal
		/// \param size Size of the fractal
		/// \see moveFractalCorner
		void moveFractalCenter(const lrc::Vec<HighPrecision, 2> &center,
							   const lrc::Vec<HighPrecision, 2> &size);

		/// Render the fractal into the fractal surface, and copy that to the
		/// fractal surface to be drawn. This will be executed on a separate thread
		/// in order to keep the UI updating
		void renderFractal();

		/// Render a sub-section of the fractal, defined by the \p box variable. This is
		/// intended to be used within the call queue to render multiple sections in
		/// parallel
		/// \param box The box configuration
		/// \param boxIndex Box ID (for updating states)
		void renderBox(const RenderBox &box, int64_t boxIndex = -1);

		/// Render a single edge of a RenderBox's bounding area. This is used internally
		/// inside the renderBox method.
		///
		/// Edge:
		///  - 0 -> Top
		///  - 1 -> Right
		///  - 2 -> Bottom
		///  - 3 -> Left
		///
		/// \param box
		/// \param fractalOrigin
		/// \param aliasFactor
		/// \param step
		/// \param aliasStepCorrect
		/// \param inc
		/// \param edge
		/// \return
		bool renderEdge(const RenderBox &box, const HighVec2 &fractalOrigin,
						int64_t aliasFactor, const HighVec2 &step,
						const HighVec2 &aliasStepCorrect, int64_t inc, int64_t edge);

		/// Calculate the colour of a pixel at standard-precision. This implements
		/// anti-aliasing as well
		/// \param pixPos Pixel-space coordinate
		/// \param aliasFactor Anti-aliasing factor
		/// \param step Step size
		/// \param aliasStepCorrect Anti-aliasing step correction
		/// \return Color of the pixel
		ci::ColorA pixelColorLow(const LowVec2 &pixPos, int64_t aliasFactor,
								 const LowVec2 &step, const LowVec2 &aliasStepCorrect);

		/// Calculate the colour of a pixel at high-precision. See pixelColorLow
		/// \param pixPos Pixel-space coordinate
		/// \param aliasFactor Anti-aliasing factor
		/// \param step Step size
		/// \param aliasStepCorrect Anti-aliasing step correction
		/// \return Color of the pixel
		/// \see pixelColorLow
		ci::ColorA pixelColorHigh(const HighVec2 &pixPos, int64_t aliasFactor,
								  const HighVec2 &step, const HighVec2 &aliasStepCorrect);

		/// Update the render configuration of the internal fractal pointer
		void updateRenderConfig();

		void updateFractalType(const std::shared_ptr<Fractal> &fractal);

		/// Ensure all values are using the highest precision possible
		void updateConfigPrecision();

		/// Regenerate the surfaces and resize them to fit the image size
		void regenerateSurface();

		/// Getter method for the render box time statistics
		/// \return Statistics
		LIBRAPID_NODISCARD RenderBoxTimeStats boxTimeStats() const;

		LIBRAPID_NODISCARD std::string getFractalName() const {
			return m_fractal->name();
		}

		LIBRAPID_NODISCARD std::vector<std::string> getColorFuncs() const;
		void setColorFunc(const std::string &func);

		LIBRAPID_NODISCARD std::vector<std::string> getPaletteNames() const;
		void setPaletteName(const std::string &palette);

		/// Constant getter method for the render configuration
		/// \return Render configuration
		LIBRAPID_NODISCARD const RenderConfig &config() const;

		/// Non-const getter method for the render configuration
		/// \return Render configuration
		LIBRAPID_NODISCARD RenderConfig &config();

		LIBRAPID_NODISCARD const std::string &paletteName() const;

		/// Constant getter method for the internal render box vector
		/// \return Render box vector
		LIBRAPID_NODISCARD const std::vector<RenderBox> &renderBoxes() const;

		/// Non-const getter method for the internal render box vector
		/// \return Render box vector
		LIBRAPID_NODISCARD std::vector<RenderBox> &renderBoxes();

		/// Constant getter method for the internal settings object
		/// \return Settings object
		LIBRAPID_NODISCARD const json &settings() const;

		/// Non-const getter method for the internal settings object
		/// \return Settings object
		LIBRAPID_NODISCARD json &settings();

		/// Constant getter method for the internal surface
		/// \return Surface
		LIBRAPID_NODISCARD const ci::Surface &surface() const;

		/// Non-const getter method for the internal surface
		/// \return Surface
		LIBRAPID_NODISCARD ci::Surface &surface();

		void exportImage(const std::string &path) const;
		void exportSettings(const std::string &path) const;

	private:
		RenderConfig m_renderConfig;		// The settings for the fractal renderer
		ci::Surface m_fractalSurface;		// The surface that the fractal is rendered to
		json m_settings;					// The settings for the fractal
		std::shared_ptr<Fractal> m_fractal; // The fractal to render
		ThreadPool m_threadPool;			// Pool for render threads

		// Colouring functions for the fractal
		coloring::ColorFuncLow m_colorFuncLow;
		coloring::ColorFuncHigh m_colorFuncHigh;
		std::string m_paletteName;

		std::vector<RenderBox> m_renderBoxes; // The state of each render box

		bool m_haltRender = false; // Used to gracefully stop the render threads
	};
} // namespace frac
