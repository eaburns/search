template <class D> struct Lrtastar : public SearchAlgorithm<D> {
	typedef typename D::State State;
	typedef typename D::PackedState PackedState;
	typedef typename D::Cost Cost;
	typedef typename D::Oper Oper;
	typedef typename D::Edge Edge;

	struct Node : SearchNode<D>{
		Node() : inLSS(false) {}

		static bool pred(Node *a, Node *b) {
			if (a->f == b->f)
				return a->g > b->g;
			return a->f < b->f;
		}

		static Cost prio(Node *n) { return n->f; }

		static Cost tieprio(Node *n) { return n->g; }

		Cost f, h;
		bool inLSS;
	};

	Lrtastar(int argc, const char *argv[]) 
		: SearchAlgorithm<D>(argc, argv), seen(30000001), lssSize(0), infCost(std::numeric_limits<int>::max()) {
		for (int i = 0; i < argc; i++) {
			if (i < argc - 1 && strcmp(argv[i], "-lsssize") == 0)
				lssSize = strtod(argv[++i], NULL);
		}

		if (lssSize < 1)
			fatal("Must specify an expansion limit for LSS  ≥1 using -lsssize");

		nodes = new Pool<Node>();
	}

	~Lrtastar() {}

	void search(D &d, typename D::State &s0) {

		this->start();
		seen.init(d);
	
		Node* currentNode = initStart(d, s0);
		State buf, &currentState = d.unpack(buf, currentNode->packed);

		std::vector<Node*> LSS;
		while(!d.isgoal(currentState) && !SearchAlgorithm<D>::limit()) {
			LSS.clear();

			generateLSS(d, currentNode, LSS);

			valueUpdateStep(d, LSS);


			while(currentNode->inLSS) {

				Node* minSucc = NULL;
				Cost minCost = infCost;
				unsigned int opIndex = 0;
				currentState = d.unpack(buf, currentNode->packed);

				typename D::Operators ops(d, currentState);

				for(unsigned int i = 0; i < ops.size(); i++) {

					Edge edge(d, currentState, ops[i]);
					Node* succ = getChildNode(d, currentNode, currentState, edge, ops[i]);

					Cost newH = edge.cost + succ->h;
					if(newH < minCost) {
						minCost = newH;
						minSucc = succ;
						opIndex = i;
					}
				}
				this->res.ops.push_back(ops[opIndex]);
				currentNode = minSucc;
			}

			currentState = d.unpack(buf, currentNode->packed);

			for(unsigned int i = 0; i < LSS.size(); i++) {
				if(LSS[i] == NULL) break;
				LSS[i]->inLSS = false;
			}
		}

		this->finish();

		if (!d.isgoal(currentState)) {
			this->res.ops.clear();
			return;
		}

		// Rebuild the path from the operators to avoid storing very long
		// paths as we go.
		seen.clear();
		this->res.path.push_back(s0);
		for (auto it = this->res.ops.begin(); it != this->res.ops.end(); it++) {
			State copy = this->res.path.back();
			typename D::Edge e(d, copy, *it);
			this->res.path.push_back(e.state);
		}
		std::reverse(this->res.ops.begin(), this->res.ops.end());
		std::reverse(this->res.path.begin(), this->res.path.end());
	
	}

protected:

	bool generateLSS(D& d, Node* current, std::vector<Node*>& LSS) {
		OpenList<Node, Node, Cost> open;
	 	ClosedList<SearchNode<D>, SearchNode<D>, D> closed(lssSize);

		open.push(current);
		
		unsigned int curExp = 1;
		LSS.resize(lssSize, NULL);
		LSS[0] = current;

		while (!open.empty() && curExp < lssSize) {
			Node *n = open.pop();

			State buf, &state = d.unpack(buf, n->packed);

			if(d.isgoal(state)) return true;

			n->inLSS = true;
			LSS[curExp] = n;

			expand(d, n, state, open, closed);
			curExp++;
		}
		return false;
	}

/* all of these Node* better be pointing into the node pool */
	void valueUpdateStep(D& d, std::vector<Node*>& LSS) {

		std::vector<Cost> backup(lssSize);

		for(unsigned int i = 0; i < LSS.size(); i++) {
			if(LSS[i] == NULL) { 
				LSS.resize(i); backup.resize(i);
				break; 
			}
			Node* n = LSS[i];
			backup[i] = n->h;
			n->h = infCost;
			assert(n->h == infCost);
		}

/* this is terribly inefficient */

		bool done = false;
		while(!done) {
			done = true;
			Node* u = NULL;
			Cost minCostOutter = infCost;
			unsigned int minIndex = 0;

			for(unsigned int i = 0; i < LSS.size(); i++) {
				if(LSS[i]->h != infCost) continue;

				done = false;

				Cost minCostInner = infCost;

				State buf, &state = d.unpack(buf, LSS[i]->packed);
				typename D::Operators ops(d, state);

				//looking for the min edge out of the current node
				for(unsigned int j = 0; j < ops.size(); j++) {
					Edge edge(d, state, ops[j]);
					Node* succ = getChildNode(d, LSS[i], state, edge, ops[j]);

					Cost newH = succ->h == infCost ? infCost : edge.cost + succ->h;
					if(newH < minCostInner) {
						minCostInner = newH;
					}
				}

				// looking for the node that minimizes that max
				Cost newH = backup[i] > minCostInner ? backup[i] : minCostInner;
				if(newH < minCostOutter) {
					minCostOutter = newH;
					u = LSS[i];
					minIndex = i;
				}
			}

			//u will be null because there was no state to update
			if(done) return; 

			assert(u);
			u->h = backup[minIndex] > minCostOutter ? backup[minIndex] : minCostOutter;

			u->f = u->g + u->h;

			if(u->h ==  infCost) return;
		}
	}

	Node* initStart(D &d, State &s0) {
		Node *n0 = nodes->construct();
		d.pack(n0->packed, s0);

		n0->g = Cost(0);
		n0->h = d.h(s0);
		n0->f = n0->h;
		n0->pop = n0->op = D::Nop;
		n0->parent = NULL;
		return n0;
	}

	void expand(D &d, Node *n, State &state, OpenList<Node, Node, Cost>& open,
	 	ClosedList<SearchNode<D>, SearchNode<D>, D>& closed) {

		typename D::Operators ops(d, state);
		for (unsigned int i = 0; i < ops.size(); i++) {
			if (ops[i] == n->pop)
				continue;
			considerkid(d, n, state, ops[i], open, closed);
		}
	}

	void considerkid(D &d, Node *parent, State &state, Oper op, OpenList<Node, Node, Cost>& open,
	 	ClosedList<SearchNode<D>, SearchNode<D>, D>& closed) {
		Node *kid = nodes->construct();
		assert (kid);
		typename D::Edge e(d, state, op);
		kid->g = parent->g + e.cost;
		d.pack(kid->packed, e.state);

		unsigned long hash = d.hash(kid->packed);
		Node *dup = static_cast<Node*>(closed.find(kid->packed, hash));
		if (dup) {
			if (kid->g >= dup->g) {
				nodes->destruct(kid);
				return;
			}
			bool isopen = open.mem(dup);
			if (isopen)
				open.pre_update(dup);
			dup->f = dup->f - dup->g + kid->g;
			dup->update(kid->g, parent, op, e.revop);
			if (isopen) {
				open.post_update(dup);
			}
			else {
				open.push(dup);
			}
			nodes->destruct(kid);
		}
		else {
			kid->h = d.h(e.state);
			kid->f = kid->g + kid->h;
			kid->update(kid->g, parent, op, e.revop);
			seen.add(kid, hash);
			closed.add(kid, hash);
			open.push(kid);
		}
	}

	Node* getChildNode(D& d, Node* node, State& state, Edge& edge, Oper op) {
		Node* child = nodes->construct();

		d.pack(child->packed, edge.state);
		unsigned long hash = d.hash(child->packed);

		Node *dup = static_cast<Node*>(seen.find(child->packed, hash));

		if(dup) {
			nodes->destruct(child);
			child = dup; 
		}
		else {
			child->g = node->g + edge.cost;
			child->h =d.h(edge.state);
			child->f = child->g + child->h;
			child->update(child->g, node, op, edge.revop);
			seen.add(child, hash);
		}

		return child;
	}

	Pool<Node> *nodes;
	ClosedList<SearchNode<D>, SearchNode<D>, D> seen;
	unsigned int lssSize;
	Cost infCost;
};