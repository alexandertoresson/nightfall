#ifndef __TYPE_TRAITS_H__
#define __TYPE_TRAITS_H__

struct true_type {};
struct false_type {};

template <bool a, bool b>
struct tt_or
{
	enum {value = true};
};

template <>
struct tt_or<false, false>
{
	enum {value = false};
};

#define TYPE_TRAIT_MAIN(trait) template<typename T> struct trait { enum {value = false}; };
#define TYPE_TRAIT_SPEC(trait, type) template<> struct trait<type> { enum {value = true}; };
#define TYPE_TRAIT_OR(trait, trait1, trait2) template<typename T> struct trait { enum {value = tt_or<trait1<T>::value, trait2<T>::value >::value}; };

TYPE_TRAIT_MAIN(is_float)
TYPE_TRAIT_SPEC(is_float, float)
TYPE_TRAIT_SPEC(is_float, double)
TYPE_TRAIT_SPEC(is_float, long double)

TYPE_TRAIT_MAIN(is_integral)
TYPE_TRAIT_SPEC(is_integral, unsigned char)
TYPE_TRAIT_SPEC(is_integral, unsigned short)
TYPE_TRAIT_SPEC(is_integral, unsigned int)
TYPE_TRAIT_SPEC(is_integral, unsigned long)

TYPE_TRAIT_SPEC(is_integral, signed char)
TYPE_TRAIT_SPEC(is_integral, signed short)
TYPE_TRAIT_SPEC(is_integral, signed int)
TYPE_TRAIT_SPEC(is_integral, signed long)

TYPE_TRAIT_SPEC(is_integral, char)
TYPE_TRAIT_SPEC(is_integral, bool)
TYPE_TRAIT_SPEC(is_integral, wchar_t)

TYPE_TRAIT_MAIN(is_void)
TYPE_TRAIT_SPEC(is_void, void)

TYPE_TRAIT_OR(is_arithmetic, is_integral, is_float)
TYPE_TRAIT_OR(is_fundamental, is_arithmetic, is_void)

#undef TYPE_TRAIT_MAIN
#undef TYPE_TRAIT_SPEC

#endif
