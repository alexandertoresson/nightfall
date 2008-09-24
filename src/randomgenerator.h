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
// Mersenne Twister random number generator -- a C++ class MTRand
// Based on code by Makoto Matsumoto, Takuji Nishimura, and Shawn Cokus
// Richard J. Wagner  v1.0  15 May 2003  rjwagner@writeme.com

#ifndef MERSENNETWISTER_H
#define MERSENNETWISTER_H

// Not thread safe (unless auto-initialization is avoided and each thread has
// its own MTRand object)

#include <iostream>
#include <climits>
//#include <stdio.h>
#include <ctime>
#include <cmath>

const int N = 624;       // length of state vector
const int SAVE = N + 1;  // length of array for save()
const int M = 397;  // period parameter

class MTRand 
{
// Data
public:
	typedef unsigned long uint32;  // unsigned integer type, at least 32 bits	

protected:
	
	uint32 state[N];   // internal state
	uint32 *pNext;     // next value to get from state
	int left;          // number of values left before reload needed


//Methods
public:
	MTRand( const uint32& oneSeed );  // initialize with a simple uint32
	MTRand( uint32 *const bigSeed, uint32 const seedLength = N );  // or an array
	MTRand();  // auto-initialize with /dev/urandom or time() and clock()
	
	// Do NOT use for CRYPTOGRAPHY without securely hashing several returned
	// values together, otherwise the generator state can be learned after
	// reading 624 consecutive values.
	
	// Access to 32-bit random numbers
	double rand();                          // real number in [0,1]
	double rand( const double& n );         // real number in [0,n]
	double randExc();                       // real number in [0,1)
	double randExc( const double& n );      // real number in [0,n)
	double randDblExc();                    // real number in (0,1)
	double randDblExc( const double& n );   // real number in (0,n)
	uint32 randInt();                       // integer in [0,2^32-1]
	uint32 randInt( const uint32& n );      // integer in [0,n] for n < 2^32
	double operator()() { return rand(); }  // same as rand()
	
	// Access to 53-bit random numbers (capacity of IEEE double precision)
	double rand53();  // real number in [0,1)
	
	// Access to nonuniform random number distributions
	double randNorm( const double& mean = 0.0, const double& variance = 0.0 );
	
	// Re-seeding functions with same behavior as initializers
	void seed( const uint32 oneSeed );
	void seed( uint32 *const bigSeed, const uint32 seedLength = N );
	void seed();
	
	// Saving and loading generator state
	void save( uint32* saveArray ) const;  // to array of size SAVE
	void load( uint32 *const loadArray );  // from such array
	friend std::ostream& operator<<( std::ostream& os, const MTRand& mtrand );
	friend std::istream& operator>>( std::istream& is, MTRand& mtrand );

protected:
	void initialize( const uint32 oneSeed );
	void reload();
	uint32 hiBit( const uint32& u ) const { return u & 0x80000000UL; }
	uint32 loBit( const uint32& u ) const { return u & 0x00000001UL; }
	uint32 loBits( const uint32& u ) const { return u & 0x7fffffffUL; }
	uint32 mixBits( const uint32& u, const uint32& v ) const
		{ return hiBit(u) | loBits(v); }
	uint32 twist( const uint32& m, const uint32& s0, const uint32& s1 ) const
		{ return m ^ (mixBits(s0,s1)>>1) ^ ((0-loBit(s1)) & 0x9908b0dfUL); }
	static uint32 hash( time_t t, clock_t c );
};

#endif  // MERSENNETWISTER_H
