#pragma once

#include <atomic>
#include <queue>

class BandwidthMonitor {
public:
	BandwidthMonitor();
	void newDataReceived(std::size_t bytes);
	double getAverageSpeed() const;

private:
	std::deque<std::pair<std::size_t, double>> data_;
	std::size_t total_bytes_;
	double last_time_;
	std::atomic<double> average_speed_;
};
