#pragma once

namespace frac {
	class HistoryBufferLinkedList {
	public:
		HistoryBufferLinkedList()											= default;
		HistoryBufferLinkedList(const HistoryBufferLinkedList &)			= delete;
		HistoryBufferLinkedList(HistoryBufferLinkedList &&)					= delete;
		HistoryBufferLinkedList &operator=(const HistoryBufferLinkedList &) = delete;
		HistoryBufferLinkedList &operator=(HistoryBufferLinkedList &&)		= delete;
		~HistoryBufferLinkedList()											= default;

		void append(HistoryBufferLinkedList *list);
		void killChildren();
		LIBRAPID_NODISCARD size_t sizeForward(size_t prevSize = 0) const;
		LIBRAPID_NODISCARD size_t sizeBackward(size_t prevSize = 0) const;

		LIBRAPID_NODISCARD HistoryBufferLinkedList *next() const;
		LIBRAPID_NODISCARD HistoryBufferLinkedList *prev() const;

		LIBRAPID_NODISCARD HistoryBufferLinkedList *start();
		LIBRAPID_NODISCARD HistoryBufferLinkedList *end();

		void set(const RenderConfig &config, const ci::Surface &surface);

	private:
		HistoryBufferLinkedList *m_next = nullptr;
		HistoryBufferLinkedList *m_prev = nullptr;

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

	private:
		HistoryBufferLinkedList *m_listHead	   = nullptr;
		HistoryBufferLinkedList *m_currentNode = nullptr;
	};
} // namespace frac
