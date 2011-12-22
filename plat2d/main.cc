#include "closedlist.hpp"
#include "../search/main.hpp"
#include "../utils/utils.hpp"
#include <cstdio>
#include <cerrno>

const char *lvl = NULL;

static void parseargs(int, const char*[]);

int main(int argc, const char *argv[]) {
	parseargs(argc, argv);

	FILE *infile = stdin;
	if (lvl) {
		infile = fopen(lvl, "r");
		if (!infile)
			fatalx(errno, "Failed to open %s for reading", lvl);
		dfpair(stdout, "level", "%s", lvl);
	}

	Plat2d d(infile);
	if (infile != stdin)
		fclose(infile);

	Result<Plat2d> res = search<Plat2d>(d, argc, argv);

	if (res.path.size() == 0)
		return 0;

	std::vector<unsigned int> controls;
	Player p(2 * Tile::Width + Player::Offx, 2 * Tile::Height + Player::Offy,
	Player::Width, Player::Height);
	for (int i = res.ops.size() - 1; i >= 0; i--) {
		controls.push_back(res.ops[i]);
		p.act(d.lvl, res.ops[i]);
		assert(p == res.path[i].player);
	}
	const Player &final = res.path[0].player;
	assert(d.lvl.majorblk(final.body.bbox).tile.flags & Tile::Down);

	dfpair(stdout, "final x loc", "%g", final.body.bbox.min.x);
	dfpair(stdout, "final y loc", "%g", final.body.bbox.min.y);
	dfpair(stdout, "controls", "%s", controlstr(controls).c_str());

	return 0;
}

static void parseargs(int argc, const char *argv[]) {
	for (int i = 1; i < argc; i++) {
		if (i < argc - 1 && strcmp(argv[i], "-lvl") == 0)
			lvl = argv[++i];
	}
}
