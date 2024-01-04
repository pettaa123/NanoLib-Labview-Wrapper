#pragma once

namespace nlc {

/**
 * @brief Depth (level of detail) of logging
 * Except for 'Off', the detail levels are in reverse order, 
 * i.e. the larger the level value, the smaller the level of detail.
 * At the default logging depth (Info), logs will include informational, 
 * warning, and error messages.
 */
enum class LogLevel : unsigned int {
	Off = 0,   /*!< Logging is turned off, nothing is logged at all */
	Trace = 1, /*!< The greatest possible depth of detail (expect huge logfiles). Entering and leaving functionality is included here. */
	Debug = 2, /*!< Logs include debug information - intermediate results, content sent or received, etc. */
	Info = 3,  /*!< Default level, informational messages */
	Warn = 4,  /*!< Warning messages - information about problems that have occurred, which, however, would not stop the execution of the current algorithm */
	Error = 5  /*!< Errors is a level that includes messages describing a serious problem that prevents the execution of the current algorithm from continuing. */
};

} // namespace nlc
