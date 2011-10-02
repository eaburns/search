#include "stn.hpp"
#include <utility>
#include <cassert>

Stn::Stn(unsigned int num) {
	grow(num+1);
	nodes[0].tozero = nodes[0].fromzero = 0;
}

Stn::Stn(const Stn &other) : nodes(other.nodes) { }

void Stn::grow(unsigned int num) {
	unsigned int oldsz = nodes.size();
	nodes.resize(oldsz + num);

	for (unsigned int i = oldsz; i < nodes.size(); i++)
		nodes[i].id = i;
}

bool Stn::add(const Constraint &c) {
	undos.resize(undos.size()+1);
	Undo &u = undos.back();

	addarcs(u, c);

	if (!propagate(u, c)) {
		undo();
		return false;
	}

	undos.push_back(u);
	return true;
}

bool Stn::eq(const Stn &o) const {
	if (nnodes() != o.nnodes())
		return false;

	for (unsigned int i = 0; i < nnodes(); i++)
		if (nodes[i].eq(o.nodes[i]))
			return false;

	return true;
}

bool Stn::Node::eq(const Stn::Node &o) const {
	if (id != o.id || tozero != o.tozero || fromzero != o.fromzero
		|| out.size() != o.out.size() || in.size() != o.in.size())
		return false;

	bool found = false;
	for (unsigned int i = 0; i < out.size() && !found; i++) {
	for (unsigned int j = 0; j < out.size() && !found; j++) {
		if (out[i].first->id == o.out[j].first->id
			 && out[i].second == o.out[j].second)
			found = true;
	}
	}
	if (!found)
		return false;

	found = false;
	for (unsigned int i = 0; i < in.size() && !found; i++) {
	for (unsigned int j = 0; j < in.size() && !found; j++) {
		if (in[i].first->id == o.in[j].first->id
			 && in[i].second == o.in[j].second)
			found = true;
	}
	}

	return found;	
}

void Stn::undo() {
	Undo &undo = undos.back();

	for (unsigned int i = 0; i < undo.popout.size(); i++) {
		Node &u = *undo.popout[i];
		assert (u.out.size() > 0);
		Node &v = *u.out.back().first;
		u.out.pop_back();
		assert (v.in.size() > 0);
		assert (v.in.back().first->id == u.id);
		v.in.pop_back();
	}

	for (unsigned int i = 0; i < undo.prevto.size(); i++)
		undo.prevto[i].first->tozero = undo.prevto[i].second;

	for (unsigned int i = 0; i < undo.prevfrom.size(); i++)
		undo.prevfrom[i].first->fromzero = undo.prevfrom[i].second;

	undos.pop_back();
}

bool Stn::propagate(Undo &u, const Constraint &c) {
	bool seen[nnodes()];
	bool toupdt[nnodes()];
	bool fromupdt[nnodes()];

	for (unsigned int i = 0; i < nnodes(); i++)
		toupdt[i] = fromupdt[i] = seen[i] = false;

	return proplower(u, toupdt, seen, nodes[c.i])
		&& propupper(u, fromupdt, seen, nodes[c.i])
		&& proplower(u, toupdt, seen, nodes[c.j])
		&& propupper(u, fromupdt, seen, nodes[c.j]);
}

void Stn::addarcs(Undo &undo, const Stn::Constraint &c) {
	if (c.b < inf()) {
		Arc out(&nodes[c.j], c.b);
		nodes[c.i].out.push_back(out);
		undo.popout.push_back(&nodes[c.i]);
		Arc in(&nodes[c.i], c.b);
		nodes[c.j].in.push_back(in);
	}
	if (c.a > neginf()) {
		Arc out(&nodes[c.i], -c.a);
		nodes[c.j].out.push_back(out);
		undo.popout.push_back(&nodes[c.j]);
		Arc in(&nodes[c.j], -c.a);
		nodes[c.i].in.push_back(in);
	}
}

bool Stn::proplower(Undo &undo, bool toupdt[], bool seen[], Node &u) {
	for (unsigned int i = 0; i < u.in.size(); i++) {
		Arc &a = u.in[i];
		Node &v = *a.first;
		Time dist = subclamp(u.tozero, a.second);

		if (dist <= v.tozero)
			continue;

		if (v.fromzero < dist || seen[v.id])
			return false;

		if (!toupdt[v.id]) {
			std::pair<Node*,Time> p(a.first, v.tozero);
			undo.prevto.push_back(p);
			toupdt[v.id] = true;
		}
		v.tozero = dist;

		seen[v.id] = true;
		if (!proplower(undo, toupdt, seen, v))
			return false;
		seen[v.id] = false;
	}

	return true;
}

bool Stn::propupper(Undo &undo, bool fromupdt[], bool seen[], Node &u) {
	for (unsigned int i = 0; i < u.out.size(); i++) {
		Arc &a = u.out[i];
		Node &v = *a.first;
		Time dist = addclamp(u.fromzero, a.second);

		if (dist >= v.fromzero)
			continue;

		if (v.tozero > dist || seen[v.id])
			return false;

		if (!fromupdt[v.id]) {
			std::pair<Node*,Time> p(a.first, v.fromzero);
			undo.prevfrom.push_back(p);
			fromupdt[v.id] = true;
		}
		v.fromzero = dist;

		seen[v.id] = true;
		if (!propupper(undo, fromupdt, seen, v))
			return false;
		seen[v.id] = false;
	}

	return true;
}