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

		/// Append a new node to the end of the linked list
		/// \param node The node to append (can contain links to other nodes)
		void append(HistoryNode *node);

		/// Free all child nodes following this one
		void killChildren();

		/// The number of nodes in the list, iterating forwards from this node
		/// \param prevSize The number of nodes in the list before this one (default to
		/// zero) \return The number of nodes in the list
		LIBRAPID_NODISCARD size_t sizeForward(size_t prevSize = 0) const;

		/// The number of nodes in the list, iterating backwards from this node
		/// \param prevSize
		/// \return The number of nodes in the list
		/// \see sizeForward
		LIBRAPID_NODISCARD size_t sizeBackward(size_t prevSize = 0) const;

		/// The next node in the linked list. This may be `nullptr`, so always check
		/// the value is valid
		/// \return The next node in the linked list
		LIBRAPID_NODISCARD HistoryNode *next() const;

		/// The previous node in the linked list. See `next()` for more information
		/// \return The previous node in the linked list
		/// \see next
		LIBRAPID_NODISCARD HistoryNode *prev() const;

		/// Iterate backwards until a node with an invalid parent is found (i.e. the
		/// first node in the linked list)
		/// \return The first node in the linked list
		LIBRAPID_NODISCARD HistoryNode *first();

		/// return the last node in the linked list
		/// \return
		/// \see first
		LIBRAPID_NODISCARD HistoryNode *last();

		/// Update the configuration and surface members of this node
		/// \param config New configuration
		/// \param surface New surface
		void set(const RenderConfig &config, const ci::Surface &surface);

		/// See `set()`
		/// \param config New configuration
		/// \see set
		void setConfig(const RenderConfig &config);

		/// See `set()`
		/// \param surface New surface
		/// \see set
		void setSurface(const ci::Surface &surface);

		/// Getter method for the configuration instance stored
		/// \return RenderConfig
		LIBRAPID_NODISCARD const RenderConfig &config() const;

		/// Getter method for the surface instance stored
		/// \return ci::Surface
		LIBRAPID_NODISCARD const ci::Surface &surface() const;

		/// Non-const getter method for the configuration instance stored
		/// \return RenderConfig
		LIBRAPID_NODISCARD RenderConfig &config();

		/// Non-const getter method for the surface instance stored
		/// \return ci::Surface
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

		/// Return the number of elements in the history buffer
		/// \return Number of elements
		LIBRAPID_NODISCARD size_t size() const;

		/// Return the first buffer item (a HistoryNode pointer)
		/// \return First item in the buffer
		LIBRAPID_NODISCARD HistoryNode *first() const;

		/// Return the last buffer item (a HistoryNode pointer)
		/// \return Last item in the buffer
		LIBRAPID_NODISCARD HistoryNode *last() const;

	private:
		HistoryNode *m_listHead	   = nullptr;
		HistoryNode *m_currentNode = nullptr;
	};
} // namespace frac
