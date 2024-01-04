#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "od_index.hpp"
#include "result.hpp"

namespace nlc {

/**
 *  @brief Sampler state
 */
enum class SamplerState {

	// Not yet configured
	Unconfigured,

	// Configured but not started
	Configured,

	// Configured and waiting for the start trigger
	Ready,

	// Running now
	Running,

	// Completed successfully
	Completed,

	// Finished due to an error
	Failed,

	// Cancelled from the application
	Cancelled
};

/**
 * @brief Result successor, with state of the sampler
 */
class ResultSamplerState : public Result {
public:
	ResultSamplerState(const SamplerState state) : result(state) {
	}

	ResultSamplerState(const std::string &errorDesc,
					   const NlcErrorCode errorCode = NlcErrorCode::GeneralError,
					   const uint32_t extendedErrorCode = 0)
		: Result(errorCode, extendedErrorCode, errorDesc), result(SamplerState::Unconfigured) {
	}

	ResultSamplerState(const ResultSamplerState &other) : Result(other), result(other.result) {
	}

	ResultSamplerState(const Result &result) : Result(result), result(SamplerState::Unconfigured) {
	}

	~ResultSamplerState() = default;

	/**
	 * @brief Returns the SamplerState in case of a successful function call.
	 *
	 * @return SamplerState
	 */
	SamplerState getResult() const {
		return result;
	}

private:
	SamplerState result;
};


/**
 * @brief Trigger condition
 */
enum class SamplerTriggerCondition : uint8_t {

	// Never
	TC_FALSE = 0x00, 

	// Immediate
	TC_TRUE = 0x01, 

	// Bit set
	// *trigger & (1 << value) != 0
	TC_SET = 0x10,

	// Bit clear
	// *trigger & (1 << value) == 0
	TC_CLEAR = 0x11,

	// Bit rising
	// (trigger[-1] & (1 << value) == 0) && (*trigger & (1 << value) != 0)
	TC_RISING_EDGE = 0x12,												

	// Bit falling
	// (trigger[-1] & (1 << value) != 0) && (*trigger & (1 << value) == 0)
	TC_FALLING_EDGE = 0x13,											

	// Bit changing
	// (trigger[-1] & (1 << value)) != (*trigger & (1 << value))
	TC_BIT_TOGGLE = 0x14,												

	// *trigger > value
	TC_GREATER = 0x15, 

	// *trigger >= value
	TC_GREATER_OR_EQUAL = 0x16,

	// *trigger < value
	TC_LESS = 0x17,	

	// *trigger <= value
	TC_LESS_OR_EQUAL = 0x18, 

	// *trigger == value
	TC_EQUAL = 0x19, 

	// *trigger != value
	TC_NOT_EQUAL = 0x1A, 

	// (value > 0) 
	// ? (*trigger - trigger[-1] > value) 
	// : (*trigger - trigger[-1] < value)
	TC_ONE_EDGE = 0x1B,							

	// abs(trigger[-1] - *trigger) > abs(value)
	TC_MULTI_EDGE = 0x1C						
};

/**
 *  @brief Sampler mode
 */
enum class SamplerMode : uint8_t {

	// (Single) oneshot execution
	Normal,

	// Starts again after it is finished
	// SampleData::iterationNumber increases by one for each new iteration
	Repetitive,
		
	// Same as Repetitive, but without iteration incrementing
	// ONLY in software mode
	Continuous
};

/**
 *  @brief Sampler configuration
 */
struct SamplerConfiguration {

	/**
	 * @brief Up to 12 OD addresses to track
	 */
	static constexpr size_t MAX_TRACKED_ADDRESSES = 12;
	std::vector<OdIndex> trackedAddresses;

	/**
	 * @brief OD address of start trigger
	 */
	OdIndex triggerAddress;

	/**
	 * @brief Start trigger condition
	 */
	SamplerTriggerCondition triggerCondition;

	/**
	 * @brief Start trigger condition value/bit
	 */
	/*
	union
	{
		uint32_t value;
		uint8_t	 bit;
	}
	*/
	uint32_t triggerValue;

	/**
	 * @brief Sampling period in milliseconds
	 */
	uint16_t periodMilliseconds;

	/**
	 * @brief Number of samplings
	 */
	uint16_t numberOfSamples;

	/**
	 * @brief Pre-trigger number of samplings
	 */
	uint16_t preTriggerNumberOfSamples;

	/**
	 * @brief Mode of the sampler
	 */
	SamplerMode mode;

	/**
	 * @brief Using software implementation
	 */
	bool forceSoftwareImplementation;
};

/**
 * @brief Sampled value
 */
struct SampledValue {

	/**
	 * @brief Value of a tracked OD address
	 */
	int64_t value;

	/**
	 * @brief Collection time in milliseconds, to the beginning of the execution
	 */
	uint64_t collectTimeMsec;
};

/**
 * @brief Sampled data
 */
struct SampleData {

	/**
	 * @brief Iteration number. It starts at 0 and only increases in Repetitive mode.
	 */
	uint64_t iterationNumber;

	/**
	 * @brief Array of sampled values.
	 */
	std::vector<SampledValue> sampledValues;
};

/**
 * @brief Result successor, with an array of sampled values
 */
class ResultSampleDataArray : public Result {
public:

	ResultSampleDataArray() = default;

	ResultSampleDataArray(const std::vector<SampleData> &dataArray) : sampleDataArray(dataArray) {
	}

	ResultSampleDataArray(const std::string &errorDesc,
						  const NlcErrorCode errorCode = NlcErrorCode::GeneralError,
						  const uint32_t extendedErrorCode = 0)
		: Result(errorCode, extendedErrorCode, errorDesc) {
	}

	ResultSampleDataArray(const ResultSampleDataArray &other)
		: Result(other), sampleDataArray(other.getResult()) {
	}

	ResultSampleDataArray(const Result &result) : Result(result) {
	}

	~ResultSampleDataArray() = default;

	const std::vector<SampleData> &getResult() const {
		return sampleDataArray;
	}

private:
	std::vector<SampleData> sampleDataArray;
};

/**
 * @brief Sampler notification
 */
class SamplerNotify {
public:
	virtual ~SamplerNotify() {
	}

	/**
	 * @brief Notification entry
	 * @param [in] lastError		- the last error occurred during the sampling
	 * @param [in] samplerState		- sampler status at the time of notification
	 * @param [in] sampleDatas		- array of sampled data
	 * @param [in] applicationData	- application specific data
	 */
	virtual void notify(const ResultVoid &lastError, const SamplerState samplerState,
						const std::vector<SampleData> &sampleDatas, int64_t applicationData)
		= 0;
};

} // namespace nlc
