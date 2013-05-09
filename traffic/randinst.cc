#include <stdio.h>
#include <stdlib.h>
#include "traffic.hpp"
#include "../search/greedy.hpp"

int main(int argc, const char *argv[]) {
	
	if(argc < 4) {
		fprintf(stderr, "usage: %s width height num_obs seed\n", argv[0]);
		return 0;
	}

	int width = strtol(argv[1], NULL, 10);
	int height = strtol(argv[2], NULL, 10);
	int num_obs = strtol(argv[3], NULL, 10);
	int seed = strtol(argv[4], NULL, 10);

	srand(seed);

	while(true) {
		GridMap map(width, height);

		for(int i = 0; i < num_obs; i++) {
			map.obstacles.emplace_back( (rand() % (width-1)) + 1, //x
									(rand() % (height-1)) + 1, //y
									(rand() % 3) - 1,		//dx
									(rand() % 3) - 1);		//dy
		}

		Traffic traffic(&map, 0, 0, width-1, height-1);
		Greedy<Traffic, true> greedy(0, NULL);
		Traffic::State start = traffic.initialstate();
		greedy.search(traffic, start);
		bool solvable = (greedy.res.ops.size() > 0);
		if(solvable) {
			traffic.output(stdout);
			break;
		}
		else {
			fprintf(stderr, "NOT SOLVABLE\n");
		}
	}
	
}
