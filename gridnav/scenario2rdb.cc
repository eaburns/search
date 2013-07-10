// © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.

#include "../utils/utils.hpp"
#include "../rdb/rdb.hpp"
#include <fstream>
#include <cerrno>

int main(int argc, char *argv[]) {
	if (argc < 3)
		fatal("usage: scenario2rdb <rdb root> <scenario file>");

	std::ifstream in(argv[2]);
	
	std::string verstr;
	unsigned int vernum;
	in >> verstr >> vernum;
	if (verstr != "version" || vernum != 1)
		fatal("Bad scenario version: %s %d", verstr.c_str(), vernum);

	unsigned int n = 0;
	while (in) {
		std::string bucket, mapfile, width, height, x0, y0, x1, y1, cost;
		in >> bucket >> mapfile >> width >> height >> x0 >> y0 >> x1 >> y1 >> cost;

		if (mapfile == "")
			continue;

		RdbAttrs attrs;
		attrs.push_back("scenario file", basename(argv[2]));
		attrs.push_back("bucket", bucket);
		attrs.push_back("map", basename(mapfile));

		char num[128];
		unsigned int i = snprintf(num, sizeof(num), "%d", n);
		if (i >= sizeof(num))
			fatal("Too many digits");
		attrs.push_back("num", num);
		n++;

		std::string path = pathfor(argv[1], attrs);
		printf("%s\n", path.c_str());
		ensuredir(path);

		FILE *f = fopen(path.c_str(), "w");
		if (!f)
			fatalx(errno, "Failed to open %s for writing", path.c_str());
		fprintf(f, "#start data file format 4\n");
		dfpair(f, "scenario file", argv[2]);
		dfpair(f, "num", num);
		dfpair(f, "map", mapfile.c_str());
		dfpair(f, "optimal cost", cost.c_str());
		dfpair(f, "start x", x0.c_str());
		dfpair(f, "start y", y0.c_str());
		dfpair(f, "goal x", x1.c_str());
		dfpair(f, "goal y", y1.c_str());
		fprintf(f, "#end data file format 4\n");
		fclose(f);
	}
	in.close();

	return 0;	
}