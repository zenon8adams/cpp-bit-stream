#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cinttypes>
#include <string>

#include "bitstream.hpp"

#define SWAP(x, y) { (x) ^= (y); (y) ^= (x); (x) ^= (y); }

BitStream::BitStream( uint64_t nblocks): n_block( nblocks )
    {
        if( n_block > 0 )
        head = allocBlock( n_block );
    }

BitStream::BitStream( const std::string& w )
{
    BitStream rep = fromStrm( w );
    swap( rep );
}

BitStream::BitStream( const BitStream& copy )
{
    *this = clone( copy );
}

BitStream& BitStream::operator=( BitStream copy )
{
    copy.swap( *this );
    return *this;
}

BitStream BitStream::operator<<( uint64_t count ) const
{
    return shl( count );
}

BitStream BitStream::operator>>( uint64_t count ) const
{
    return shr( count );
}

BitStream BitStream::operator~()
{
    BitStream result = clone( *this );
    for( Block *p = result.head; p ; p = p->next )
    {
        p->bstr.__b[ 0] ^= -1ULL;
        p->bstr.__b[ 1] ^= -1ULL;
        p->bstr.__b[ 2] ^= -1ULL;
        p->bstr.__b[ 3] ^= -1ULL;
    }
    return result;
}

BitStream BitStream::operator[]( uint64_t pos ) const
{
    if( pos >= n_block * BLOCK_SIZE )
        return false;
    BitStream result;
    result.head = allocBlock( n_block );
    result.n_block = n_block;
    Block *psrc = head,
        *pdst = result.head;
    seek( psrc, pos / BLOCK_SIZE );
    seek( pdst, pos / BLOCK_SIZE );
    int64_t npos = pos % BLOCK_SIZE;
    size_t i = npos / UNIT_SIZE;
    npos = npos % UNIT_SIZE;
    uint64_t val = psrc->bstr.__b[ i] &
        ((uint64_t)INT64_MIN >> npos);
    pdst->bstr.__b[ i] = val;
    return result;
}

bool BitStream::toBool() const
{
    uint64_t sum = 0;
    for( Block *p = head; p ; p = p->next )
    {
        sum |= p->bstr.__b[ 0];
        sum |= p->bstr.__b[ 1];
        sum |= p->bstr.__b[ 2];
        sum |= p->bstr.__b[ 3];
        if( sum )
            return true;
    }
    return false;
}

void BitStream::swap( BitStream& rhs )
{
    using std::swap;
    swap( head, rhs.head );
    swap( n_block, rhs.n_block );
}

size_t BitStream::numBlocks() const
{
    return n_block;
}

void BitStream::resize( uint64_t newsize )
{
    if( newsize == n_block )
        return;
    if( newsize > n_block )
    {
        Block *p  = allocBlock( newsize - n_block ),
            *pe = head;
        seek( pe, n_block-1 );
        pe->next = p;
    }
    else if( newsize < n_block )
    {
        BitStream bs = clone( newsize );
        swap( bs );
    }
    n_block = newsize;
}

DEBUG_VIS void BitStream::print() const
{
    BitStream out = clone( *this );
    transpose( out );
    for( const Block *p = out.head; p ; p = p->next )
        printUnit( p->bstr );

    putchar('\n');
}

BitStream::~BitStream()
{
    freeBlock( head, n_block );
}

BitStream::Block *BitStream::allocBlock( uint64_t count )
{
    if( count == 0 )
        return nullptr;

    Block *p = (Block *)calloc( 1, sizeof(Block) );
    if( p == nullptr )
        return nullptr;
    uint64_t i = 1;
    for( Block *pn = p; i < count; ++i )
    {
        pn->next = (Block *)calloc( 1, sizeof(Block) );
        pn = pn->next;
    }
    return p;
}

void BitStream::freeBlock( Block *&b, uint64_t count )
{
    while( count-- )
    {
        Block *next = b->next;
        free( b );
        b = next;
    }
    b = nullptr;
}

void BitStream::seek( Block *&p, uint64_t count ) const
{
    int64_t limit = count - 2,
        i = 0;
    for( ; i < limit; i += 3 )
    {
        p = p->next;
        p = p->next;
        p = p->next;
    }
    for( ; i < count; ++i )
        p = p->next;
}

BitStream BitStream::shl( uint64_t count ) const
{
    BitStream result;
    if( count == 0 )
        return clone( *this );
    result.head = allocBlock( n_block );
    result.n_block = n_block;
    Block *pdst = result.head,
        *psrc = head;
    if( count >= BLOCK_SIZE * n_block )
        return result;
    uint64_t factor = count / UNIT_SIZE,
        shift = count % UNIT_SIZE,
        i = 0;

    seek( pdst, factor >> 2 );
    factor &= 03;

    uint64_t cur,
        mask = (1ULL << shift) - 1,
        prev = 0,
        end  = factor + (n_block * BLOCK_SIZE - count) / UNIT_SIZE;

    while( factor <= end )
    {
        cur = prev | (psrc->bstr.__b[ i & 03] >> shift);
        pdst->bstr.__b[ factor++ & 03] = cur;
        prev = (psrc->bstr.__b[ i++ & 03] & mask) << (UNIT_SIZE - shift);

        if( (factor & 03) == 0)
            pdst = pdst->next;
        if( (i & 03) == 0 )
            psrc = psrc->next;
    }

    return result;
}

BitStream BitStream::shr( uint64_t count ) const
{
    BitStream result = clone( *this );

    transpose( result );
    result = result.shl( count );
    transpose( result );

    return result;

}

BitStream BitStream::clone( const BitStream& src, uint64_t limit )
{
    BitStream dst( MIN(src.n_block, limit) );
    Block *pdst = dst.head,
        *psrc = src.head;
    for( uint64_t i = 0; i < src.n_block; ++i )
    {
        memcpy( &pdst->bstr, &psrc->bstr, sizeof(Unit) );
        pdst = pdst->next;
        psrc = psrc->next;
    }
    return dst;
}


void DEBUG_VIS BitStream::printUnit( const Unit& u )
{
    static const char* fmt[] = { "%.16jx", "%.16jx" };
    printf( fmt[ u.__b[ 0] > 0], u.__b[ 0] );
    printf( fmt[ u.__b[ 1] > 0], u.__b[ 1] );
    printf( fmt[ u.__b[ 2] > 0], u.__b[ 2] );
    printf( fmt[ u.__b[ 3] > 0], u.__b[ 3] );
}

void BitStream::transpose( BitStream& src )
{
    uint64_t n_block = src.n_block;
    Unit *stack = (Unit *)calloc( n_block, sizeof(Unit) );
    uint64_t i = 1;
    for( Block *p = src.head; p ; p = p->next )
    {
        memcpy( stack + n_block - i, &p->bstr, sizeof(Unit) );

        REV64( stack[ n_block - i].__b[ 0] )
        REV64( stack[ n_block - i].__b[ 1] )
        REV64( stack[ n_block - i].__b[ 2] )
        REV64( stack[ n_block - i].__b[ 3] )

        ++i;
    }

    Unit *ps = stack;
    for( Block *p = src.head; p ; p = p->next )
    {
        SWAP(ps->__b[ 0], ps->__b[ 3])
        SWAP(ps->__b[ 1], ps->__b[ 2])
        memcpy( &p->bstr, ps++, sizeof(Unit) );
    }

    free( stack );
}

BitStream BitStream::fromStrm( const std::string& w )
{
    BitStream result(0);
    Block *p = nullptr;
    Unit u;
    memset( &u, 0, sizeof(Unit) );
    size_t size = w.size(), j = 0;
    uint64_t rep = 0;
    bool dirty = false;
    for( size_t i = 1; i <= size; ++i )
    {
        dirty = true;
        uint8_t c = tolower(w[ size - i]);
        c = (c >= 'a' && c <= 'f') ? 10 + c - 'a' : c - '0';
        c = (c & 05) << 1 | (c >> 1) & 05;
        c = (c & 03) << 2 | (c >> 2) & 03;
        rep |= (uint64_t)c << (UNIT_SIZE - (i << 2));

        if( (i & 0x0F) == 0 )
        {
            u.__b[ j++] = rep;
            rep = 0;
        }

        if( j == 4 )
        {
            if( p == nullptr )
            {
                p = result.head = allocBlock( 1 );
                memcpy( &p->bstr, &u, sizeof(Unit) );
            }
            else
            {
                p->next = allocBlock( 1 );
                memcpy( &p->next->bstr, &u, sizeof(Unit) );
            }
            ++result.n_block;
            memset( &u, 0, sizeof(Unit) );
            j = 0;
            dirty = false;
        }
    }

    if( dirty )
    {
        u.__b[ j] += rep;
        if( result.head == nullptr )
        {
            result.head = p = allocBlock( 1 );
            memcpy( &p->bstr, &u, sizeof(Unit) );
        }
        else
        {
            p->next = allocBlock( 1 );
            memcpy( &p->next->bstr, &u, sizeof(Unit) );
        }
        ++result.n_block;
    }

    return result;
}
