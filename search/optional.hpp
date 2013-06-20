// Copyright © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.
// Testing for some optional domain methods.
// This code relies on "substitution failure is not an error" crap
// in C++.  It is really one of the most ugly hacks that I have
// ever seen.  Let's keep it fairly well isolated
//
// When a search algorithm checks for optional methods you
// should make sure to have it output datafile pairs that say
// yes or no for each optional method.  Otherwise, there is no
// real way to know which options were used on a given run.
//
// Special thanks to Steve McCoy for figuring this out for me.

template <class D> struct methods {
	typedef char yes[1], no[2];

	template<class U, typename U::Cost (U::*)(typename U::State&)> struct HASD {};
	template<class U> static yes& hasd(HASD<U, &U::d>*);
	template<class> static no& hasd(...);
	// methods<SomeDomain>::d is true iff SomeDomain has a d method
	static const bool d = sizeof(hasd<D>(NULL)) == sizeof(yes);

	template<class U, typename U::Cost (U::*)(typename U::State&)> struct HASHNEAR {};
	template<class U> static yes& hashnear(HASHNEAR<U, &U::hnear>*);
	template<class> static no& hashnear(...);
	// methods<SomeDomain>::hnear is true iff SomeDomain has a
	// hnear method
	static const bool hnear = sizeof(hashnear<D>(NULL)) == sizeof(yes);
};