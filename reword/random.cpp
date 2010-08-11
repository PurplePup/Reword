////////////////////////////////////////////////////////////////////
/*

File:			random.cpp

Class impl:		CRandom

Description:	A random number generator class

Author:			not sure?

Date:			?

Licence:		This program is free software; you can redistribute it and/or modify
				it under the terms of the GNU General Public License as published by
				the Free Software Foundation; either version 2 of the License, or
				(at your option) any later version.

				This software is distributed in the hope that it will be useful,
				but WITHOUT ANY WARRANTY; without even the implied warranty of
				MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
				GNU General Public License for more details.

				You should have received a copy of the GNU General Public License
				along with this program; if not, write to the Free Software
				Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/
////////////////////////////////////////////////////////////////////
#include "random.h"

#include <ctime>


CRandom::CRandom(void)
{
   rseed = 1;
   mti=CMATH_N+1;
}

// Returns a number from 0 to n (excluding n)
unsigned int CRandom::Random( unsigned int n )
{
    unsigned long y;
    static unsigned long mag01[2]={0x0, CMATH_MATRIX_A};

   if(n==0)
     return(0);

    /* mag01[x] = x * MATRIX_A for x=0,1 */

    if (mti >= CMATH_N) { /* generate N words at one time */
        int kk;

        if (mti == CMATH_N+1)   /* if sgenrand() has not been called, */
            SetRandomSeed(4357); /* a default initial seed is used   */

        for (kk=0;kk<CMATH_N-CMATH_M;kk++) {
            y = (mt[kk]&CMATH_UPPER_MASK)|(mt[kk+1]&CMATH_LOWER_MASK);
            mt[kk] = mt[kk+CMATH_M] ^ (y >> 1) ^ mag01[y & 0x1];
        }
        for (;kk<CMATH_N-1;kk++) {
            y = (mt[kk]&CMATH_UPPER_MASK)|(mt[kk+1]&CMATH_LOWER_MASK);
            mt[kk] = mt[kk+(CMATH_M-CMATH_N)] ^ (y >> 1) ^ mag01[y & 0x1];
        }
        y = (mt[CMATH_N-1]&CMATH_UPPER_MASK)|(mt[0]&CMATH_LOWER_MASK);
        mt[CMATH_N-1] = mt[CMATH_M-1] ^ (y >> 1) ^ mag01[y & 0x1];

        mti = 0;
    }

    y = mt[mti++];
    y ^= CMATH_TEMPERING_SHIFT_U(y);
    y ^= CMATH_TEMPERING_SHIFT_S(y) & CMATH_TEMPERING_MASK_B;
    y ^= CMATH_TEMPERING_SHIFT_T(y) & CMATH_TEMPERING_MASK_C;
    y ^= CMATH_TEMPERING_SHIFT_L(y);

    return (y%n);

}

void CRandom::SetRandomSeed(unsigned int n)
{
   /* setting initial seeds to mt[N] using         */
   /* the generator Line 25 of Table 1 in          */
   /* [KNUTH 1981, The Art of Computer Programming */
   /*    Vol. 2 (2nd Ed.), pp102]                  */
   mt[0]= n & 0xffffffff;
   for (mti=1; mti<CMATH_N; mti++)
     mt[mti] = (69069 * mt[mti-1]) & 0xffffffff;

   rseed = n;
}
unsigned int CRandom::GetRandomSeed(void)
{
   return(rseed);
}

void CRandom::Randomize(void)
{
   SetRandomSeed((unsigned int)time(NULL));
}

