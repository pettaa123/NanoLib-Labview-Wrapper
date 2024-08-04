#pragma once
#include <string>
#include <vector>
#include <thread>
#include <functional>

class C5E {
public:
	C5E(std::string busType);
	~C5E();

	int Init();
	int Start();
	int Stop();
	int Jog();
	int Home();
	using Callback = std::function<void()>;
	void SetHomingCallback(Callback callback);
	bool IsHomingDone();

	int Close();

	int GetExceptions(std::vector<std::string>& exceptions);

private:
	std::string busType_;
	std::vector<std::string> exceptions_;
	int FindSubstringInVector(const std::vector<std::string>& vec,const std::string& substring) const;

	void PollingThreadFunc();

	std::atomic<bool> homingDone_;
	std::atomic<bool> stopThread_;
	std::thread pollingThread_;

	Callback homingCallback_;
};
