#pragma once

#define FRAC_LOG(message)                                                                \
	::frac::debugLogger.write(message, ::frac::Priority::Info, FILENAME, __LINE__)
#define FRAC_WARN(message)                                                               \
	::frac::debugLogger.write(message, ::frac::Priority::Warning, FILENAME, __LINE__)
#define FRAC_ERROR(message)                                                              \
	::frac::debugLogger.write(message, ::frac::Priority::Error, FILENAME, __LINE__)

namespace frac {
	// A way to specify the verbosity of the debug logger
	enum class Priority { Info = 0, Warning = 5, Error = 10 };

	/// A logger type that can write debug information to a file
	class DebugLogger {
	public:
		/*
		 * Since there should only ever be a single DebugLogger instance, delete the
		 * majority of the constructors and assignment operators as they are not needed
		 */

		DebugLogger()								= delete;
		DebugLogger(const DebugLogger &)			= delete;
		DebugLogger(DebugLogger &&)					= delete;
		DebugLogger &operator=(const DebugLogger &) = delete;
		DebugLogger &operator=(DebugLogger &&)		= delete;

		/// Create a new DebugLogger instance from a filename
		/// \param filename
		explicit DebugLogger(const std::string &filename,
							 Priority priority = Priority::Info);

		/// Close the file stream on destruction
		~DebugLogger();

		/// Set the priority level of the debugger's logs. Higher priorities will be
		/// logged in release mode, while lower priorities will only be logged in debug
		/// mode
		/// \param newPriority The new priority level
		void setPriorityLevel(Priority newPriority) { m_priority = newPriority; }

		/// Write a message to the log file
		/// \param message The message to write
		/// \param priority The priority of the message (see Priority)
		/// \param filename The filename of the file that called this function
		/// \param line The line number of the file that called this function
		void write(const std::string &message, Priority priority,
				   const std::string &filename, int64_t line);

	private:
		std::fstream m_log;					  // File stream
		double m_startTime;					  // Time at which the logger was created
		Priority m_priority = Priority::Info; // Priority level of the logger
	};

	extern DebugLogger debugLogger;
} // namespace frac
