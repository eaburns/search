#include <stdio.h>
#include <stdlib.h>

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

	fprintf(stdout, "%d %d\n", width, height);
	for(int i = 0; i < num_obs; i++) {
		int x = (rand() % (width-1)) + 1;
		int y = (rand() % (height-1)) + 1;
		int dx = (rand() % 3) - 1;
		int dy = (rand() % 3) - 1;
		fprintf(stdout, "%d %d %d %d\n", x, y, dx, dy);
	}
}
