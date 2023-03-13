#pragma once

namespace frac {
	class HistoryNode {
	public:
		HistoryNode()								= default;
		HistoryNode(const HistoryNode &)			= delete;
		HistoryNode(HistoryNode &&)					= delete;
		HistoryNode &operator=(const HistoryNode &) = delete;
		HistoryNode &operator=(HistoryNode &&)		= delete;
		~HistoryNode()								= default;

		void append(HistoryNode *list);
		void killChildren();
		LIBRAPID_NODISCARD size_t sizeForward(size_t prevSize = 0) const;
		LIBRAPID_NODISCARD size_t sizeBackward(size_t prevSize = 0) const;

		LIBRAPID_NODISCARD HistoryNode *next() const;
		LIBRAPID_NODISCARD HistoryNode *prev() const;

		LIBRAPID_NODISCARD HistoryNode *first();
		LIBRAPID_NODISCARD HistoryNode *last();

		void set(const RenderConfig &config, const ci::Surface &surface);
		void setConfig(const RenderConfig &config);
		void setSurface(const ci::Surface &surface);

		LIBRAPID_NODISCARD const RenderConfig &config() const;
		LIBRAPID_NODISCARD const ci::Surface &surface() const;

		LIBRAPID_NODISCARD RenderConfig &config();
		LIBRAPID_NODISCARD ci::Surface &surface();

	private:
		HistoryNode *m_next = nullptr;
		HistoryNode *m_prev = nullptr;

		RenderConfig m_config;
		ci::Surface m_surface;
	};

	class HistoryBuffer {
	public:
		HistoryBuffer()									= default;
		HistoryBuffer(const HistoryBuffer &)			= delete;
		HistoryBuffer(HistoryBuffer &&)					= delete;
		HistoryBuffer &operator=(const HistoryBuffer &) = delete;
		HistoryBuffer &operator=(HistoryBuffer &&)		= delete;

		~HistoryBuffer();

		/// Append a new point to the history buffer
		/// \param config The settings for the fractal renderer
		/// \param surface A saved copy of the fractal surface
		void append(const RenderConfig &config, const ci::Surface &surface);

		/// Undo the last operation
		bool undo();

		/// If possible, redo the last operation
		bool redo();

		LIBRAPID_NODISCARD size_t size() const;

		LIBRAPID_NODISCARD HistoryNode *first() const;
		LIBRAPID_NODISCARD HistoryNode *last() const;

	private:
		HistoryNode *m_listHead	   = nullptr;
		HistoryNode *m_currentNode = nullptr;
	};
} // namespace frac
