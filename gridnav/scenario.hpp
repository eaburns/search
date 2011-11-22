#include "gridmap.hpp"
#include "gridnav.hpp"
#include "../search/search.hpp"
#include <string>
#include <iostream>
#include <cstdio>

struct ScenarioEntry;

std::istream &operator>>(std::istream&, ScenarioEntry&);

struct Scenario;

struct ScenarioEntry {
	ScenarioEntry(Scenario &);
	Result<GridNav> run(Search<GridNav>*);
	void outputrow(FILE*, unsigned int, Result<GridNav>&);
private:
	friend std::istream &operator>>(std::istream &, ScenarioEntry &);

	Scenario &scen;
	unsigned int bucket;
	std::string mapfile;
	unsigned int w, h;
	unsigned int x0, y0;
	unsigned int x1, y1;
	float opt;
};

struct Scenario {

	Scenario(int, char *[]);
	~Scenario(void);

	void run(std::istream&);
	GridMap *getmap(std::string);

private:
	void checkver(std::istream&);
	void outputhdr(FILE*);

	int argc;
	char **argv;
	std::string maproot;
	GridMap *lastmap;
	Result<GridNav> res;
	int entry, nentries;
};