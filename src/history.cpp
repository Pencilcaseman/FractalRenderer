#include <fractal/fractal.hpp>

namespace frac {
	void HistoryNode::append(HistoryNode *list) {
		if (m_next) {
			m_next->append(list);
		} else {
			m_next		   = list;
			m_next->m_prev = this;
		}
	}

	void HistoryNode::killChildren() {
		if (m_next) m_next->killChildren();
		delete m_next;
		m_next = nullptr;
	}

	size_t HistoryNode::sizeForward(size_t prevSize) const {
		if (m_next)
			return m_next->sizeForward(prevSize + 1);
		else
			return prevSize;
	}

	size_t HistoryNode::sizeBackward(size_t prevSize) const {
		if (m_prev)
			return m_prev->sizeBackward(prevSize + 1);
		else
			return prevSize;
	}

	HistoryNode *HistoryNode::next() const { return m_next; }
	HistoryNode *HistoryNode::prev() const { return m_prev; }

	HistoryNode *HistoryNode::first() {
		if (m_prev)
			return m_prev->first();
		else
			return this;
	}

	HistoryNode *HistoryNode::last() {
		if (m_next)
			return m_next->last();
		else
			return this;
	}

	void HistoryNode::set(const RenderConfig &config, const ci::Surface &surface) {
		m_config  = config;
		m_surface = surface;
	}

	void HistoryNode::setConfig(const RenderConfig &config) { m_config = config; }

	void HistoryNode::setSurface(const ci::Surface &surface) { m_surface = surface; }

	const RenderConfig &HistoryNode::config() const { return m_config; }
	const ci::Surface &HistoryNode::surface() const { return m_surface; }
	RenderConfig &HistoryNode::config() { return m_config; }
	ci::Surface &HistoryNode::surface() { return m_surface; }

	HistoryBuffer::~HistoryBuffer() {
		m_listHead->killChildren();
		// No need to delete m_currentNode, since it will be killed recursively
		LIBRAPID_ASSERT(!m_listHead->next() && !m_listHead->prev(),
						"HistoryBuffer is not empty");
		delete m_listHead;
	}

	void HistoryBuffer::append(const RenderConfig &config, const ci::Surface &surface) {
		auto list = new HistoryNode;
		list->set(config, surface);
		if (m_listHead) {
			m_listHead->append(list);
			m_currentNode = m_listHead->last();
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

	size_t HistoryBuffer::size() const {
		if (!m_listHead) return 0;
		return m_listHead->sizeForward();
	}

	HistoryNode *HistoryBuffer::first() const { return m_listHead->first(); }
	HistoryNode *HistoryBuffer::last() const { return m_listHead->last(); }
} // namespace frac
