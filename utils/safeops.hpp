// Copyright 2012 © Steve McCoy
// Licensed under the MIT license.
#include <limits>

template<class N>
struct BadFlow {
	N a, b;
	const char *op;
	bool over;

	BadFlow(N a, N b, const char *op, bool over)
		: a(a), b(b), op(op), over(over) {}
};

template<class N, bool S = std::numeric_limits<N>::is_signed>
struct safe_ops{
	static N add(N, N);
	static N sub(N, N);
	static N mul(N, N);
	static N div(N, N);
};

template<class N>
struct safe_ops<N, true>{
	typedef std::numeric_limits<N> n;
//	static_assert(std::numeric_limits<N>::is_integer, "safe_ops only supports integer types");

	static N add(N a, N b){
		if(a > 0 && b > 0 && n::max() - b < a)
			throw BadFlow<N>(a, b, "+", true);
		if(a < 0 && b < 0 && n::min() - b > a)
			throw BadFlow<N>(a, b, "+", false);
		return a + b;
	}

	static N sub(N a, N b){
		if(a > 0 && b < 0 && n::max() - b < a)
			throw BadFlow<N>(a, b, "-", true);
		if(a < 0 && b > 0 && n::min() + b > a)
			throw BadFlow<N>(a, b, "-", true);
		return a - b;
	}

	static N mul(N a, N b){
		if(a > 0 && b > 0 && n::max()/b < a)
			throw BadFlow<N>(a, b, "*", true);
		if(a < 0 && b > 0 && n::min()/a < b)
			throw BadFlow<N>(a, b, "*", false);
		if(a > 0 && b < 0 && n::min()/b < a)
			throw BadFlow<N>(a, b, "*", false);
		if(a < 0 && b < 0){
			if(a == n::min() || b == n::min())
				throw BadFlow<N>(a, b, "*", true);
			if(-(n::min()/b) > a)
				throw BadFlow<N>(a, b, "*", true);
		}
		return a * b;
	}

	static N div(N a, N b){
		if(b == 0)
			throw BadFlow<N>(a, b, "/", true);
		return a/b;
	}
};

template<class N>
struct safe_ops<N, false>{
	typedef std::numeric_limits<N> n;
//	static_assert(n::is_integer, "safe_ops only supports integer types");

	static N add(N a, N b){
		if(n::max() - b < a)
			throw BadFlow<N>(a, b, "+", true);
		return a + b;
	}

	static N sub(N a, N b){
		if(n::max() - b < a)
			throw BadFlow<N>(a, b, "-", true);
		return a - b;
	}

	static N mul(N a, N b){
		if(a > 0 && b > 0 && n::max()/b < a)
			throw BadFlow<N>(a, b, "*", true);
		return a * b;
	}

	static N div(N a, N b){
		if(b == 0)
			throw BadFlow<N>(a, b, "/", true);
		return a/b;
	}
};

template<class N>
N safe_add(N a, N b){
	return safe_ops<N>::add(a, b);
}

template<class N>
N safe_sub(N a, N b){
	return safe_ops<N>::sub(a, b);
}

template<class N>
N safe_mul(N a, N b){
	return safe_ops<N>::mul(a, b);
}

template<class N>
bool can_mul(N a, N b) {
	try {
		safe_mul(a, b);
	} catch (const BadFlow<N> &) {
		return false;
	}
	return true;
}

template<class N>
N safe_div(N a, N b){
	return safe_ops<N>::div(a, b);
}

// -----8<-----
/*
int main(){
	try{
		safe_mul((short)3, std::numeric_limits<short>::max());
	}catch(const BadFlow<short> &bf){
		return -1;
	}
	return 0;
}
*/