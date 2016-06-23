#ifndef MYLIBRARY_ANY_INCLUDED
#define MYLIBRARY_ANY_INCLUDED

#if defined(_MSC_VER)
# pragma once
#endif

// what:  对boost::any的扩展，使any具有序列化的能力
// who:   张潇健
// when:  2016/05/21

#include <algorithm>

#include "boost/config.hpp"
#include <boost/type_index.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/type_traits/decay.hpp>
#include <boost/type_traits/remove_cv.hpp>
#include <boost/type_traits/add_reference.hpp>
#include <boost/type_traits/is_reference.hpp>
#include <boost/type_traits/is_const.hpp>
#include <boost/throw_exception.hpp>
#include <boost/static_assert.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/is_const.hpp>
#include <boost/mpl/if.hpp>

class serializable_any
{
public: // structors

	serializable_any() BOOST_NOEXCEPT
		: content(0)
	{
	}

	template<typename ValueType>
	serializable_any(const ValueType & value)
		: /*content(new holder<ValueType>(value))*/
		content(new holder<
		BOOST_DEDUCED_TYPENAME boost::remove_cv<BOOST_DEDUCED_TYPENAME boost::decay<const ValueType>::type>::type
		>(value))
	{
	}

	serializable_any(const serializable_any & other)
		: content(other.content ? other.content->clone() : 0)
	{
	}

//#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
//	// Move constructor
//	serializable_any(serializable_any&& other) BOOST_NOEXCEPT
//		: content(other.content)
//	{
//		other.content = 0;
//	}
//
//	// Perfect forwarding of ValueType
//	template<typename ValueType>
//	serializable_any(ValueType&& value
//		, typename boost::disable_if<boost::is_same<serializable_any&, ValueType> >::type* = 0 // disable if value has type `any&`
//		, typename boost::disable_if<boost::is_const<ValueType> >::type* = 0) // disable if value has type `const ValueType&&`
//		: content(new holder< typename decay<ValueType>::type >(static_cast<ValueType&&>(value)))
//	{
//	}
//#endif

	~serializable_any() BOOST_NOEXCEPT
	{
		delete content;
	}

public: // modifiers

	serializable_any & swap(serializable_any & rhs) BOOST_NOEXCEPT
	{
		std::swap(content, rhs.content);
		return *this;
	}


#ifdef BOOST_NO_CXX11_RVALUE_REFERENCES
		template<typename ValueType>
	serializable_any & operator=(const ValueType & rhs)
	{
		serializable_any(rhs).swap(*this);
		return *this;
	}

	serializable_any & operator=(serializable_any rhs)
	{
		serializable_any(rhs).swap(*this);
		return *this;
	}

#else 
		serializable_any & operator=(const serializable_any& rhs)
	{
		serializable_any(rhs).swap(*this);
		return *this;
	}

	// move assignement
	serializable_any & operator=(serializable_any&& rhs) BOOST_NOEXCEPT
	{
		rhs.swap(*this);
		serializable_any().swap(rhs);
		return *this;
	}

		// Perfect forwarding of ValueType
		template <class ValueType>
	serializable_any & operator=(ValueType&& rhs)
	{
		serializable_any(static_cast<ValueType&&>(rhs)).swap(*this);
		return *this;
	}
#endif

public: // queries

	bool empty() const BOOST_NOEXCEPT
	{
		return !content;
	}

		void clear() BOOST_NOEXCEPT
	{
		serializable_any().swap(*this);
	}

		const boost::typeindex::type_info& type() const BOOST_NOEXCEPT
	{
		return content ? content->type() : boost::typeindex::type_id<void>().type_info();
	}

	void pack(std::ostream& os) const
	{
		if (content){
			content->pack(os);
		}
	}

	void unpack(std::istream& is)
	{
		if (content){
			content->unpack(is);
		}
	}

#ifndef BOOST_NO_MEMBER_TEMPLATE_FRIENDS
private: // types
#else
public: // types (public so any_cast can be non-friend)
#endif

	class placeholder
	{
	public: // structors

		virtual ~placeholder()
		{
		}

	public: // queries

		virtual const boost::typeindex::type_info& type() const BOOST_NOEXCEPT = 0;

		virtual placeholder * clone() const = 0;

		virtual void pack(std::ostream&) const = 0;
		
		virtual void unpack(std::istream&) = 0;

	};

	template<typename ValueType>
	class holder : public placeholder
	{
	public: // structors

		holder(const ValueType & value)
			: held(value)
		{
		}

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
		holder(ValueType&& value)
			: held(static_cast<ValueType&&>(value))
		{
		}
#endif
	public: // queries

		virtual const boost::typeindex::type_info& type() const BOOST_NOEXCEPT
		{
			return boost::typeindex::type_id<ValueType>().type_info();
		}

			virtual placeholder * clone() const
		{
			return new holder(held);
		}

		virtual void pack(std::ostream& os) const
		{
			os << held;
		}

		virtual void unpack(std::istream& is)
		{
			is >> held;
		}

	public: // representation

		ValueType held;

	private: // intentionally left unimplemented
		holder & operator=(const holder &);
	};

#ifndef BOOST_NO_MEMBER_TEMPLATE_FRIENDS

private: // representation

	template<typename ValueType>
	friend ValueType * any_cast(serializable_any *) BOOST_NOEXCEPT;

	template<typename ValueType>
	friend ValueType * unsafe_any_cast(serializable_any *) BOOST_NOEXCEPT;

#else

public: // representation (public so any_cast can be non-friend)

#endif

	placeholder * content;

};

inline void swap(serializable_any & lhs, serializable_any & rhs) BOOST_NOEXCEPT
{
	lhs.swap(rhs);
}

class BOOST_SYMBOL_VISIBLE bad_any_cast :
#ifndef BOOST_NO_RTTI
	public std::bad_cast
#else
	public std::exception
#endif
{
public:
	virtual const char * what() const BOOST_NOEXCEPT_OR_NOTHROW
	{
		return "boost::bad_any_cast: "
		"failed conversion using boost::any_cast";
	}
};

template<typename ValueType>
ValueType * any_cast(serializable_any * operand) BOOST_NOEXCEPT
{
	return operand && operand->type() == boost::typeindex::type_id<ValueType>()
	? &static_cast<serializable_any::holder<BOOST_DEDUCED_TYPENAME boost::remove_cv<ValueType>::type> *>(operand->content)->held
	: 0;
}

	template<typename ValueType>
inline const ValueType * any_cast(const serializable_any * operand) BOOST_NOEXCEPT
{
	return any_cast<ValueType>(const_cast<serializable_any *>(operand));
}

	template<typename ValueType>
ValueType any_cast(serializable_any & operand)
{
	typedef BOOST_DEDUCED_TYPENAME boost::remove_reference<ValueType>::type nonref;


	nonref * result = any_cast<nonref>(&operand);
	if (!result)
		boost::throw_exception(bad_any_cast());

	// Attempt to avoid construction of a temporary object in cases when 
	// `ValueType` is not a reference. Example:
	// `static_cast<std::string>(*result);` 
	// which is equal to `std::string(*result);`
	typedef BOOST_DEDUCED_TYPENAME boost::mpl::if_<
		boost::is_reference<ValueType>,
		ValueType,
		BOOST_DEDUCED_TYPENAME boost::add_reference<ValueType>::type
	>::type ref_type;

	return static_cast<ref_type>(*result);
}

template<typename ValueType>
inline ValueType any_cast(const serializable_any & operand)
{
	typedef BOOST_DEDUCED_TYPENAME boost::remove_reference<ValueType>::type nonref;
	return any_cast<const nonref &>(const_cast<serializable_any &>(operand));
}

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
template<typename ValueType>
inline ValueType any_cast(serializable_any&& operand)
{
	BOOST_STATIC_ASSERT_MSG(
		boost::is_rvalue_reference<ValueType&&>::value /*true if ValueType is rvalue or just a value*/
		|| boost::is_const< typename boost::remove_reference<ValueType>::type >::value,
		"boost::any_cast shall not be used for getting nonconst references to temporary objects"
		);
	return any_cast<ValueType>(operand);
}
#endif


// Note: The "unsafe" versions of any_cast are not part of the
// public interface and may be removed at any time. They are
// required where we know what type is stored in the any and can't
// use typeid() comparison, e.g., when our types may travel across
// different shared libraries.
template<typename ValueType>
inline ValueType * unsafe_any_cast(serializable_any * operand) BOOST_NOEXCEPT
{
	return &static_cast<serializable_any::holder<ValueType> *>(operand->content)->held;
}

	template<typename ValueType>
inline const ValueType * unsafe_any_cast(const serializable_any * operand) BOOST_NOEXCEPT
{
	return unsafe_any_cast<ValueType>(const_cast<serializable_any *>(operand));
}


inline std::ostream& operator<<(std::ostream& os, const serializable_any& c)
{
	c.pack(os);
	return os;
}

inline std::istream& operator>>(std::istream& is, serializable_any& c)
{
	c.unpack(is);
	return is;
}

#endif //MYLIBRARY_ANY_INCLUDED