#include <fractal/fractal.hpp>

namespace frac {
	DebugLogger::DebugLogger(const std::string &filename, Priority priority) {
		m_log.open(filename, std::fstream::out);
		m_startTime = lrc::now();

		m_log << "============[ FRACTAL RENDERER DEBUG LOG ]============\n" << std::endl;
	}

	DebugLogger::~DebugLogger() {
		m_log << "\n============[ FRACTAL RENDERER DEBUG LOG ]============";
		m_log.flush();
		m_log.close();
	}

	void DebugLogger::write(const std::string &message, Priority priority,
							const std::string &filename, int64_t line) {
		if (static_cast<size_t>(priority) < static_cast<size_t>(m_priority)) return;

		double time						   = lrc::now();
		constexpr size_t maxFilenameLength = 40;
		std::string truncatedFilename;
		std::string priorityString;
		std::string cleanedMessage;

		if (filename.size() > maxFilenameLength)
			truncatedFilename =
			  "..." + filename.substr(filename.size() - maxFilenameLength + 3, maxFilenameLength);
		else
			truncatedFilename = filename;

		switch (priority) {
			case Priority::Info: priorityString = "INFO"; break;
			case Priority::Warning: priorityString = "WARNING"; break;
			case Priority::Error: priorityString = "ERROR"; break;
		}

		// Pad new lines with 26 spaces to align with the beginning of the message
		// in the log file.
		int64_t preambleLength = 26 + maxFilenameLength;
		for (char c : message) {
			cleanedMessage += c;
			if (c == '\n') cleanedMessage += std::string(preambleLength, ' ');
		}

		// Use std::endl here to force-flush the buffer -- this is the only way
		// to ensure that the log is written to disk in the event of a crash
		m_log << fmt::format("[ {:.5f} ] {:>7} {:>{}}:{:0>4} {}",
							 time - m_startTime,
							 priorityString,
							 truncatedFilename,
							 maxFilenameLength,
							 line,
							 cleanedMessage)
			  << std::endl;
	}

#if defined(LIBRAPID_DEBUG) // More reliable than NDEBUG
	DebugLogger debugLogger("./log.txt", Priority::Info);
#else
	DebugLogger debugLogger("./log.txt", Priority::Warning);
#endif
} // namespace frac
