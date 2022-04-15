//
// Created by zener-diode on 15/04/2022.
//

#ifndef BITSTREAM_HPP
#define BITSTREAM_HPP

#define MIN(x, y) ((y) ^ (((x) ^ (y)) & -((x) < (y))))
#define MAX(x, y) ((x) ^ (((x) ^ (y)) & -((x) < (y))))

#ifdef __DEBUG
#define DEBUG_VIS __attribute__((__visibility__("hidden")))
#else
#define DEBUG_VIS
#endif

#include <cstdint>
#include <string>
class BitStream
{
    struct Unit;
    struct Block;
#define UNIT_SIZE (sizeof(uint64_t) << 3u)

#define BLOCK_SIZE (UNIT_SIZE << 2u)

#define CREATE_OPERATOR( name, op, cmp ) \
	\
	BitStream name( const BitStream& rhs ) const\
	{ \
		BitStream result; \
		if( n_block cmp rhs.n_block ) \
			result = clone( *this ); \
		else \
			result = clone( rhs ); \
		uint64_t n_pair = MIN( n_block, rhs.n_block ); \
		Block *l = head, *r = rhs.head, *dst = result.head; \
		for( uint64_t i = 0u; i < n_pair; ++i ) \
		{ \
			dst->bstr.__b[0] = l->bstr.__b[0] op r->bstr.__b[0]; \
			dst->bstr.__b[1] = l->bstr.__b[1] op r->bstr.__b[1]; \
			dst->bstr.__b[2] = l->bstr.__b[2] op r->bstr.__b[2]; \
			dst->bstr.__b[3] = l->bstr.__b[3] op r->bstr.__b[3]; \
			dst = dst->next; l = l->next; r = r->next; \
		} \
        result.resize( MAX( n_block, rhs.n_block ) ); \
        return result; \
	}

#define CREATE_ASSIGNMENT( name, op ) \
	\
	BitStream& name##=( const BitStream& rhs ) \
	{ \
		return *this = *this op rhs; \
	}

#define SHIFT_ASSIGNMENT( name, op ) \
    \
    BitStream& name##=( uint64_t count ) \
    { \
        return *this = *this op count; \
    }

#define REV64( qword ) \
    \
    do { \
        uint64_t m1  = 0x5555555555555555, \
                 m2  = 0x3333333333333333, \
                 m4  = 0x0F0F0F0F0F0F0F0F, \
                 m8  = 0x00FF00FF00FF00FF, \
                 m16 = 0x0000FFFF0000FFFF, \
                 m32 = 0x00000000FFFFFFFF; \
        \
        (qword) = ((qword) &  m1) << 1  | (((qword) >>  1) &  m1); \
        (qword) = ((qword) &  m2) << 2  | (((qword) >>  2) &  m2); \
        (qword) = ((qword) &  m4) << 4  | (((qword) >>  4) &  m4); \
        (qword) = ((qword) &  m8) << 8  | (((qword) >>  8) &  m8); \
        (qword) = ((qword) & m16) << 16 | (((qword) >> 16) & m16); \
        (qword) = ((qword) & m32) << 32 | (((qword) >> 32) & m32); \
        \
    }while(0);

public:

    explicit BitStream( uint64_t nblocks = 4 );

    explicit BitStream( const std::string& w );

    BitStream( const BitStream& copy );

    BitStream& operator=( BitStream copy );

    BitStream operator<<( uint64_t count ) const;

    BitStream operator>>( uint64_t count ) const;

    BitStream operator~();

    BitStream operator[]( uint64_t pos ) const;

    bool toBool() const;

    void swap( BitStream& rhs );

    size_t numBlocks() const;

    void resize( uint64_t newsize );

    DEBUG_VIS void print() const;

    CREATE_OPERATOR( operator|, |, > )
    CREATE_OPERATOR( operator^, ^, < )
    CREATE_OPERATOR( operator&, &, < )

    CREATE_ASSIGNMENT( operator|, |  )
    CREATE_ASSIGNMENT( operator^, ^  )
    CREATE_ASSIGNMENT( operator&, &  )

    SHIFT_ASSIGNMENT( operator<<, << )
    SHIFT_ASSIGNMENT( operator>>, >> )

    ~BitStream();

private:

    struct Unit
    {
        uint64_t __b[ 4];
    };

    struct Block
    {
        Unit bstr;
        Block *next;
    } *head = nullptr;
    uint64_t n_block{};

    static Block *allocBlock( uint64_t count );

    static void freeBlock( Block *&b, uint64_t count );

    void seek( Block *&p, uint64_t count ) const;

    BitStream shl( uint64_t count ) const;

    BitStream shr( uint64_t count ) const;

    static BitStream clone( const BitStream& src, uint64_t limit = -1 );


    static void DEBUG_VIS printUnit( const Unit& u );

    static void transpose( BitStream& src );

    static BitStream fromStrm( const std::string& w );

};


#endif //BITSTREAM_HPP
