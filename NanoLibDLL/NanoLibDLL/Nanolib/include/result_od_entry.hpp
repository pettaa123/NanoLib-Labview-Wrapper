#pragma once


#include "od_entry.hpp"
#include "od_sub_entry.hpp"

namespace nlc {

/**
 * @brief Instance of this class is returned in case the function returns a ObjectEntry.
 */
class ResultObjectEntry : public Result {
public:
	ResultObjectEntry(nlc::ObjectEntry const &result_) : Result(), result(result_) {
	}

	explicit ResultObjectEntry(std::string const &errorString_)
		: Result(errorString_), result(invalidObject) {
	}

	explicit ResultObjectEntry(NlcErrorCode const &errCode, std::string const &errorString_)
		: Result(errCode, errorString_), result(invalidObject) {
	}

	explicit ResultObjectEntry(NlcErrorCode const &errCode, const uint32_t exErrCode,
							   std::string const &errorString_)
		: Result(errCode, exErrCode, errorString_), result(invalidObject) {
	}

	explicit ResultObjectEntry(Result const &result) : Result(result), result(invalidObject) {
	}

	virtual ~ResultObjectEntry() {
	}

	/**
	 * @brief Returns the vector of DeviceId in case of a successful function call.
	 *
	 * @return const std::vector<DeviceId>
	 */
	nlc::ObjectEntry const &getResult() const {
		return result;
	}

protected:
	static inline const nlc::ObjectEntry invalidObject;
	nlc::ObjectEntry const &result;
};

/**
 * @brief Instance of this class is returned in case the function returns a ObjectSubEntryCore.
 */
class ResultObjectSubEntry : public Result {
public:
	ResultObjectSubEntry(nlc::ObjectSubEntry const &result_) : Result(), result(result_) {
	}

	explicit ResultObjectSubEntry(std::string const &errorString_)
		: Result(errorString_), result(invalidObject) {
	}

	explicit ResultObjectSubEntry(NlcErrorCode const &errCode, std::string const &errorString_)
		: Result(errCode, errorString_) , result(invalidObject) {
	}

	explicit ResultObjectSubEntry(NlcErrorCode const &errCode, const uint32_t exErrCode,
								  std::string const &errorString_)
		: Result(errCode, exErrCode, errorString_), result(invalidObject) {
	}

	explicit ResultObjectSubEntry(Result const &result) : Result(result), result(invalidObject) {
	}

	virtual ~ResultObjectSubEntry() {
	}

	/**
	 * @brief Returns the vector of DeviceId in case of a successful function call.
	 *
	 * @return const std::vector<DeviceId>
	 */
	const nlc::ObjectSubEntry &getResult() const {
		return result;
	}

protected:
	static inline const nlc::ObjectSubEntry invalidObject;
	nlc::ObjectSubEntry const &result;
};

} // namespace nlc
