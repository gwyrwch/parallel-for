#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include "array_processer.h"


int main() {
	auto benchmark = [](auto& processer){
		for (auto num_threads: {1, 2, 4, 8, 16, 32}) {
			const auto stats = processer.Process({
				.num_threads = num_threads,
				.additional_heavyness = 500
			});

			if (!stats.all_correct) {
				puts("Failed");
			}
			printf("Total time = %dms with num_threads = %d\n", stats.run_time, num_threads);
			fflush(stdout);
		}
	};

	puts("With mutex:"); 
	ArrayProcesserLock processer_lock(1024 * 1024);
	benchmark(processer_lock);

	puts("With atomic");
	ArrayProcesserNoLock processer_nolock(1024 * 1024);
	benchmark(processer_nolock);



	// Config config = {.a = 5, .b = "5"};

	// Config* config = new Config();
	// config->a = 5;
	// std::cout << config->a;
}