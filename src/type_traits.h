/*
 * Nightfall - Real-time strategy game
 *
 * Copyright (c) 2008 Marcus Klang, Alexander Toresson and Leonard Wickmark
 * 
 * This file is part of Nightfall.
 * 
 * Nightfall is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Nightfall is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Nightfall.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef TYPE_TRAITS_H
#define TYPE_TRAITS_H

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
