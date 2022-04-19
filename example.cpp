#include "bitstream.hpp"

int main()
{
	BitStream num1( "000123456700c08912789af1234000897650" ),
          	  num2( "ffffffffffffffffffffffffffffffffffff" );

    num1 ^= num2;
    num1.print();
    num1 <<= 511;
    num1.print();
    ( ~num2[ 12]).print();
    (num1 ^ num2).print();
    num1 <<= 20;
    num1.print();
    ((num2 >> 127) | (num2 << 255)).print();
    num2.print();
    ((num2 >> 100) & (num1 << 20)).print();
    num2.print();	
}
