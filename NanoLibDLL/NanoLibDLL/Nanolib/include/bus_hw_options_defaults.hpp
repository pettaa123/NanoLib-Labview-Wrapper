#pragma once
#include <string>

namespace nlc {

/**
	* @brief CAN bus baudrates
	* 
	* - std::string
*/
struct CanBaudRate {
	const std::string BAUD_RATE_1000K = "1000k";
	const std::string BAUD_RATE_800K = "800k";
	const std::string BAUD_RATE_500K = "500k";
	const std::string BAUD_RATE_250K = "250k";
	const std::string BAUD_RATE_125K = "125k";
	const std::string BAUD_RATE_100K = "100k";
	const std::string BAUD_RATE_50K = "50k";
	const std::string BAUD_RATE_20K = "20k";
	const std::string BAUD_RATE_10K = "10k";
	const std::string BAUD_RATE_5K = "5k";
};

/**
	* @brief IXXAT adapter bus number
	*
	* - std::string
*/
struct IxxatAdapterBusNumber {
	const std::string BUS_NUMBER_0_DEFAULT = "0";
	const std::string BUS_NUMBER_1 = "1";
	const std::string BUS_NUMBER_2 = "2";
	const std::string BUS_NUMBER_3 = "3";
};

/**
	* @brief IXXAT adapter configuration options
*/
struct Ixxat {
	const std::string ADAPTER_BUS_NUMBER_OPTIONS_NAME = "ixxat adapter bus number";
	const IxxatAdapterBusNumber adapterBusNumber = IxxatAdapterBusNumber();
};

/**
	* @brief CAN bus configuration options
*/
struct CanBus {
	const std::string BAUD_RATE_OPTIONS_NAME = "can adapter baud rate";
	const CanBaudRate baudRate = CanBaudRate();
	const Ixxat ixxat = Ixxat();
};

/**
	* @brief Serial communication parity options
	*
	* - std::string
*/
struct SerialParity {
	const std::string NONE = "none";
	const std::string ODD = "odd";
	const std::string EVEN = "even";
	const std::string MARK = "mark";
	const std::string SPACE = "space";
};

/**
	* @brief Serial communication baudrate
	*
	* - std::string
*/
struct SerialBaudRate {
	const std::string BAUD_RATE_7200 = "7200";
	const std::string BAUD_RATE_9600 = "9600";
	const std::string BAUD_RATE_14400 = "14400";
	const std::string BAUD_RATE_19200 = "19200";
	const std::string BAUD_RATE_38400 = "38400";
	const std::string BAUD_RATE_56000 = "56000";
	const std::string BAUD_RATE_57600 = "57600";
	const std::string BAUD_RATE_115200 = "115200";
	const std::string BAUD_RATE_128000 = "128000";
	const std::string BAUD_RATE_256000 = "256000";
};

/**
	* @brief Serial communication configuration options
*/
struct Serial {
	const std::string BAUD_RATE_OPTIONS_NAME = "serial baud rate";
	const SerialBaudRate baudRate = SerialBaudRate();
	const std::string PARITY_OPTIONS_NAME = "serial parity";
	const SerialParity parity = SerialParity();
};

/**
 * @brief RESTful communication configuration options
 */
struct RESTfulBus {
	const std::string CONNECT_TIMEOUT_OPTION_NAME = "RESTful Connect Timeout";
	const unsigned long DEFAULT_CONNECT_TIMEOUT = 200; /* msec */
	const std::string REQUEST_TIMEOUT_OPTION_NAME = "RESTful Request Timeout";
	const unsigned long DEFAULT_REQUEST_TIMEOUT = 200; /* msec */
	const std::string RESPONSE_TIMEOUT_OPTION_NAME = "RESTful Response Timeout";
	const unsigned long DEFAULT_RESPONSE_TIMEOUT = 750; /* msec */
};

/**
 * @brief EtherCAT communication configuration options
 */
struct EtherCATBus {

	/**
	 * @brief Network state treated as Firmware mode.
	 * Acceptable values:
	 * EtherCATState::PRE_OPERATIONAL
	 * EtherCATState::SAFE_OPERATIONAL
	 * EtherCATState::OPERATIONAL
	 * The default value is EtherCATState::PRE_OPERATIONAL.
	 */
	const std::string NETWORK_FIRMWARE_STATE_OPTION_NAME = "Network Firmware State";
	const std::string DEFAULT_NETWORK_FIRMWARE_STATE = "PRE-OPERATIONAL";

	/**
	 * @brief Timeout in msec to acquire exclusive lock on the network.
	 * The default value is 500 msec.
	 */
	const std::string EXCLUSIVE_LOCK_TIMEOUT_OPTION_NAME = "Exclusive Lock Timeout";
	const unsigned int DEFAULT_EXCLUSIVE_LOCK_TIMEOUT = 500; /* msec */

	/**
	 * @brief Timeout in msec to acquire shared lock on the network.
	 * The default value is 250 msec.
	 */
	const std::string SHARED_LOCK_TIMEOUT_OPTION_NAME = "Shared Lock Timeout";
	const unsigned int DEFAULT_SHARED_LOCK_TIMEOUT = 250; /* msec */

	/**
	 * @brief Timeout in msec for a read operation.
	 * The default value is 700 msec.
	 */
	const std::string READ_TIMEOUT_OPTION_NAME = "Read Timeout";
	const unsigned int DEFAULT_READ_TIMEOUT = 700; /* msec */

	/**
	 * @brief Timeout in msec for a write operation.
	 * The default value is 200 msec.
	 */
	const std::string WRITE_TIMEOUT_OPTION_NAME = "Write Timeout";
	const unsigned int DEFAULT_WRITE_TIMEOUT = 200; /* msec */

	/**
	 * @brief Maximum read or write attempts.
	 * Non-zero values are acceptable.
	 * The default value is 5.
	 */
	const std::string READ_WRITE_ATTEMPTS_OPTION_NAME = "Read/Write Attempts";
	const unsigned int DEFAULT_READ_WRITE_ATTEMPTS = 5;

	/**
	 * @brief Maximum number of attempts to change network state.
	 * Non-zero values are acceptable.
	 * The default value is 10.
	 */
	const std::string CHANGE_NETWORK_STATE_ATTEMPTS_OPTION_NAME = "Change Network State Attempts";
	const unsigned int DEFAULT_CHANGE_NETWORK_STATE_ATTEMPTS = 10;

	/**
	 * @brief Enables or disables PDO processing for digital inputs and outputs
	 * Acceptable values are "True" or "False".
	 * The default value is "True".
	 */
	const std::string PDO_IO_ENABLED_OPTION_NAME = "PDO IO Enabled";
	const std::string DEFAULT_PDO_IO_ENABLED = "True";
};


/**
	* @brief Default configuration options class
*/
struct BusHwOptionsDefault {
	const CanBus canBus = CanBus();
	const Serial serial = Serial();
	const RESTfulBus restfulBus = RESTfulBus();
	const EtherCATBus ethercatBus = EtherCATBus();
};

const BusHwOptionsDefault busHwOptionsDefaults;
} // namespace nlc
