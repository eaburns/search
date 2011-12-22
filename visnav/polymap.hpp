#include "../utils/geom.hpp"
#include <vector>
#include <cstdio>

struct Image;

// A PolyMap is a map that contains a set of polygons.
// It can be used to make visibility queries in a plane
// with respect to ploygonal obstacles.
struct PolyMap {

	PolyMap(FILE*);

	PolyMap(std::vector<Polygon> ps) : polys(ps) { }

	// Constructs a polygon map from a bitmap of obstacles.
	PolyMap(bool[], unsigned int, unsigned int);

	// input reads the visibility map from a file.
	void input(FILE*);

	// output writes the visibility map to the given file.
	void output(FILE*) const;

	// draw draws the polygons for the visibility map to
	// the given image.
	void draw(Image&, double lwidth = 1) const;

	// scale scales the visibility map by the
	// given factors in both the x and y directions.
	void scale(double, double);

	// translate translates the visibility map by the
	// give x and y values.
	void translate(double, double);

	// min returns the minimum x and y coordinate
	// in the map;
	Point min(void) const;

	// max returns the maximum x and y coordinate
	// in the map.
	Point max(void) const;

	// obstructed returns true if the given point lies
	// within one of the obstacles.
	bool obstructed(const Point &pt) {
		for (unsigned int i = 0; i < polys.size(); i++) {
			if (polys[i].contains(pt))
				return true;
		}
		return false;
	}

	// isvisible returns true if the two points are visible
	// from eachother.
	// 
	// This function assumes that both points are not
	// obstructed.
	bool isvisible(const Point&, const Point&) const;

	std::vector<Polygon> polys;
};