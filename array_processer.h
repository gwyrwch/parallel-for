#pragma once

#include "timer.h"
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>


struct Stats {
	bool all_correct = true;
	int run_time = 0;
};

struct Config {
	int num_threads;
	int additional_heavyness;
};

class IArrayProcesser {
protected:	
	std::vector<char> bytes;

	IArrayProcesser(int n) : bytes(n, 0) {}

public:
	virtual Stats Process(Config /*config*/) = 0;
};

class ArrayProcesserLock :  public IArrayProcesser {
private:
	int first_unprocessed;
	std::mutex m;
public:
	ArrayProcesserLock(int n) 
		: IArrayProcesser(n)
		, first_unprocessed(0) 
	{}

	Stats Process(Config config) {
		std::fill(bytes.begin(), bytes.end(), 0);
		first_unprocessed = 0;

		Timer timer;

		std::vector<std::thread> threads;
		for (size_t i = 0; i < config.num_threads; i++) {
			threads.emplace_back([this, &config](){
				while(true) {
					int i;
					{
						std::lock_guard<std::mutex> guard(m);
						i = first_unprocessed++;

						if (i >= bytes.size()) 
							break;
					}

					bytes[i] += 1; // replace with bytes[i] = 1
					std::this_thread::sleep_for(std::chrono::nanoseconds(config.additional_heavyness));
				}
			});
		}

		for (size_t i = 0; i < config.num_threads; i++) {
			threads[i].join(); // чтобы все потоки закончили работать и только потом выполнился return 
		}

		Stats result;
		result.run_time = timer.Passed();

		for (const auto e : bytes) {
			if (e != 1) {
				result.all_correct = false;
			}
		}
		return result;
	}
};


class ArrayProcesserNoLock :  public IArrayProcesser {
private:
	std::atomic<int> first_unprocessed;
public:
	ArrayProcesserNoLock(int n) 
		: IArrayProcesser(n)
		, first_unprocessed(0) 
	{}

	Stats Process(Config config) {
		std::fill(bytes.begin(), bytes.end(), 0);
		first_unprocessed = 0;

		Timer timer;

		std::vector<std::thread> threads;
		for (size_t i = 0; i < config.num_threads; i++) {
			threads.emplace_back([this, &config](){
				while(true) {
					int i = first_unprocessed.fetch_add(1);
					if (i >= bytes.size()) 
						break;
					bytes[i] += 1; // replace with bytes[i] = 1
					std::this_thread::sleep_for(std::chrono::nanoseconds(config.additional_heavyness));
				}
			});
		}

		for (size_t i = 0; i < config.num_threads; i++) {
			threads[i].join(); // чтобы все потоки закончили работать и только потом выполнился return 
		}

		Stats result;
		result.run_time = timer.Passed();

		for (const auto e : bytes) {
			if (e != 1) {
				result.all_correct = false;
			}
		}
		return result;
	}
};
