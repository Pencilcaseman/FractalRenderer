#include <fractal/fractal.hpp>

namespace frac {
	void HistoryBufferLinkedList::append(HistoryBufferLinkedList *list) {
		if (m_next)
			m_next->append(list);
		else
			m_next = list;
	}

	void HistoryBufferLinkedList::killChildren() {
		if (m_next) m_next->killChildren();
		delete m_next;
	}

	size_t HistoryBufferLinkedList::sizeForward(size_t prevSize) const {
		if (m_next)
			return m_next->sizeForward(prevSize + 1);
		else
			return prevSize;
	}

	size_t HistoryBufferLinkedList::sizeBackward(size_t prevSize) const {
		if (m_prev)
			return m_prev->sizeBackward(prevSize + 1);
		else
			return prevSize;
	}

	HistoryBufferLinkedList *HistoryBufferLinkedList::next() const { return m_next; }
	HistoryBufferLinkedList *HistoryBufferLinkedList::prev() const { return m_prev; }

	HistoryBufferLinkedList *HistoryBufferLinkedList::start() {
		if (m_prev)
			return m_prev->start();
		else
			return this;
	}

	HistoryBufferLinkedList *HistoryBufferLinkedList::end() {
		if (m_next)
			return m_next->end();
		else
			return this;
	}

	void HistoryBufferLinkedList::set(const RenderConfig &config,
									  const ci::Surface &surface) {
		m_config  = config;
		m_surface = surface;
	}

	HistoryBuffer::~HistoryBuffer() {
		m_listHead->killChildren();
		// No need to delete m_currentNode, since it will be killed recursively
		LIBRAPID_ASSERT(!m_listHead->next() && !m_listHead->prev(),
						"HistoryBuffer is not empty");
		delete m_listHead;
	}

	void HistoryBuffer::append(const RenderConfig &config, const ci::Surface &surface) {
		auto list = new HistoryBufferLinkedList;
		list->set(config, surface);
		if (m_listHead) {
			m_listHead->append(list);
			m_currentNode = m_listHead->end();
		} else {
			m_listHead	  = list;
			m_currentNode = list;
		}
	}

	bool HistoryBuffer::undo() {
		if (m_currentNode->prev()) {
			m_currentNode = m_currentNode->prev();
			return true;
		}
		return false;
	}

	bool HistoryBuffer::redo() {
		if (m_currentNode->next()) {
			m_currentNode = m_currentNode->next();
			return true;
		}
		return false;
	}

	size_t HistoryBuffer::size() const { return m_listHead->sizeForward(); }
} // namespace frac
