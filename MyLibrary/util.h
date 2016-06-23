#pragma once

#include <iostream>

//#include <boost/polymorphic_cast.hpp>

template <class Target, class Source>
inline Target polymorphic_cast_noexcept(Source x)
{
#ifdef NDEBUG
	return static_cast<Target>(x);
#else
	return dynamic_cast<Target>(x);
#endif
}

template <class Target, class Source>
inline Target polymorphic_cast(Source& x)
{
	return polymorphic_cast_noexcept<Target>(x);
}

template <class Target, class Source>
inline Target polymorphic_cast(Source* x)
{
	Target tmp = polymorphic_cast_noexcept<Target>(x);
	if ( tmp == 0 ) 
		throw std::bad_cast();
	return tmp;
}