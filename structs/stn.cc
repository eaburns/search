#include "stn.hpp"
#include <utility>

Stn::Stn(unsigned int num) : nodes(num+1) {
	nodes[0].tozero = nodes[0].fromzero = 0;

	for (unsigned int i = 0; i <= num; i++)
		nodes[i].id = i;
}

Stn::Stn(const Stn &other) : nodes(other.nodes) { }

void Stn::add(unsigned int num) {
	unsigned int oldsz = nodes.size();
	nodes.resize(nodes.size() + num);

	for (unsigned int i = oldsz; i < nodes.size(); i++)
		nodes[i].id = i;
}

void Stn::addarcs(const Stn::Constraint &c) {
	if (c.b < inf()) {
		Arc out(&nodes[c.j], c.b);
		nodes[c.i].out.push_back(out);
		Arc in(&nodes[c.i], c.b);
		nodes[c.j].in.push_back(in);
	}
	if (c.a > neginf()) {
		Arc out(&nodes[c.i], -c.a);
		nodes[c.j].out.push_back(out);
		Arc in(&nodes[c.j], -c.a);
		nodes[c.i].in.push_back(in);
	}
}

bool Stn::proplower(bool seen[], Node &u) {
	for (unsigned int i = 0; i < u.in.size(); i++) {
		Arc &a = u.in[i];
		Node &v = *a.first;
		Time dist = sub_clamp(u.tozero, a.second);

		if (dist <= v.tozero)
			continue;

		if (v.fromzero < dist || seen[v.id])
			return false;

		v.tozero = dist;
		seen[v.id] = true;
		if (!proplower(seen, v))
			return false;
		seen[v.id] = false;
	}

	return true;
}

bool Stn::propupper(bool seen[], Node &u) {
	for (unsigned int i = 0; i < u.out.size(); i++) {
		Arc &a = u.out[i];
		Node &v = *a.first;
		Time dist = add_clamp(u.fromzero, a.second);

		if (dist >= v.fromzero)
			continue;

		if (v.tozero > dist || seen[v.id])
			return false;

		v.fromzero = dist;
		seen[v.id] = true;
		if (!propupper(seen, v))
			return false;
		seen[v.id] = false;
	}

	return true;
}

bool Stn::propagate(const Constraint &c) {
	bool seen[nnodes()];
	for (unsigned int i = 0; i < nnodes(); i++)
		seen[i] = 0;

	return proplower(seen, nodes[c.i])
		&& propupper(seen, nodes[c.i])
		&& proplower(seen, nodes[c.j])
		&& propupper(seen, nodes[c.j]);
}

bool Stn::add(const Constraint &c) {
	addarcs(c);
	return propagate(c);
}