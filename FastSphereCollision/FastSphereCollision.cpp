// FastSphereCollision.cpp : This file contains the 'main' function. Program execution begins and ends there.
// Option to use either:
// a) SIMD instructions to parallelize the distance calculation, or
// b) normal serialized instructions, or
// c) both to compare speed.
//
// Run with `--pin` to pin the process to core 0 for more reproducible numbers.
#include <algorithm>
#include <cstring>
#include <iostream>
#include <vector>

//***  Various knobs that control the operation of this code. Changes require rebuild. ***//

    // Performance run settings.
    static const int ITERATIONS = 1000000;  // this is the number of collision tests to run
    static const int PAGE_COUNT = 1;        // the number of pages of random position data to generate (disconnects data allocation from iteration count)
    static const int REP_COUNT  = 5;        // number of times to repeat each pass for min/median/max statistics

//*** End Knobs ***//

// OS-specific bits: Windows headers and the QPC shim for non-Windows.
#if defined( _WIN32 )
    #include <windows.h>
    #if defined( _MSC_VER )
        #define ALIGN(x) __declspec( align( x ) )
    #else
        #define ALIGN(x) __attribute__( ( aligned( x ) ) )
    #endif
#else
    #include <ctime>
    #if defined( __linux__ )
        #include <sched.h>
    #endif
    #define ALIGN(x) __attribute__( ( aligned( x ) ) )

    typedef struct
    {
        long long QuadPart;
    } LARGE_INTEGER;

    inline void QueryPerformanceFrequency( LARGE_INTEGER *lpFrequency )
    {
        lpFrequency->QuadPart = 1000000000LL; // nanoseconds per second
    }

    inline void QueryPerformanceCounter( LARGE_INTEGER *lpPerformanceCount )
    {
        struct timespec ts;
        if( clock_gettime( CLOCK_MONOTONIC, &ts ) != 0 )
        {
            lpPerformanceCount->QuadPart = 0;
            return;
        }

        lpPerformanceCount->QuadPart =
            static_cast< long long >( ts.tv_sec ) * 1000000000LL + ts.tv_nsec;
    }
#endif

// SIMD abstraction: x86_64 SSE3 or AArch64 NEON. Scalar fallback otherwise.
#if defined( __x86_64__ ) || defined( _M_X64 )
    #if defined( _MSC_VER )
        #include <intrin.h>
    #else
        #include <x86intrin.h>
    #endif
    #define SIMD_AVAILABLE 1

    using simd4f = __m128;

    inline simd4f simd_load(  const float *p )           { return _mm_load_ps( p ); }
    inline simd4f simd_set1(  float x )                  { return _mm_set1_ps( x ); }
    inline simd4f simd_add(   simd4f a, simd4f b )       { return _mm_add_ps( a, b ); }
    inline simd4f simd_sub(   simd4f a, simd4f b )       { return _mm_sub_ps( a, b ); }
    inline simd4f simd_mul(   simd4f a, simd4f b )       { return _mm_mul_ps( a, b ); }
    inline simd4f simd_cmpge( simd4f a, simd4f b )       { return _mm_cmpge_ps( a, b ); }
    inline float  simd_hsum( simd4f v )
    {
        simd4f s  = _mm_hadd_ps( v, v );
        simd4f s2 = _mm_hadd_ps( s, s );
        return _mm_cvtss_f32( s2 );
    }
    // Count how many lanes of a comparison mask are "true" (all-bits-1).
    inline int simd_mask_popcount( simd4f mask )
    {
        // movemask: high bit of each lane → bits 0-3 of an int. popcount the result.
        int bits = _mm_movemask_ps( mask );
        // Manual popcount on 4 bits.
        return ( bits & 1 ) + ( ( bits >> 1 ) & 1 ) + ( ( bits >> 2 ) & 1 ) + ( ( bits >> 3 ) & 1 );
    }
#elif defined( __aarch64__ ) || defined( _M_ARM64 )
    #include <arm_neon.h>
    #define SIMD_AVAILABLE 1

    using simd4f = float32x4_t;

    inline simd4f simd_load(  const float *p )           { return vld1q_f32( p ); }
    inline simd4f simd_set1(  float x )                  { return vdupq_n_f32( x ); }
    inline simd4f simd_add(   simd4f a, simd4f b )       { return vaddq_f32( a, b ); }
    inline simd4f simd_sub(   simd4f a, simd4f b )       { return vsubq_f32( a, b ); }
    inline simd4f simd_mul(   simd4f a, simd4f b )       { return vmulq_f32( a, b ); }
    inline simd4f simd_cmpge( simd4f a, simd4f b )       { return vreinterpretq_f32_u32( vcgeq_f32( a, b ) ); }
    inline float  simd_hsum(  simd4f v )                 { return vaddvq_f32( v ); }
    // Count how many lanes of a comparison mask are "true" (all-bits-1).
    inline int simd_mask_popcount( simd4f mask )
    {
        // vcgeq_f32 produces 0xFFFFFFFF for true lanes. Treat as int32, sum lanes,
        // negate (each true lane is -1 when interpreted signed) → popcount.
        int32x4_t m = vreinterpretq_s32_f32( mask );
        return -static_cast< int >( vaddvq_s32( m ) );
    }
#else
    #define SIMD_AVAILABLE 0
#endif

static const int FLOAT_PER_OBJECT = 3;

class Vec3
{
private:
    ALIGN(16) float m_Comps[ 4 ];
    static const int m_CompsSize = sizeof( m_Comps ) / sizeof( m_Comps[ 0 ] );

public:
    Vec3()
    {
        ::memset( m_Comps, 0, m_CompsSize );
    }

    Vec3( float x, float y, float z )
    {
        m_Comps[ 0 ] = x;
        m_Comps[ 1 ] = y;
        m_Comps[ 2 ] = z;
        m_Comps[ 3 ] = 0.0f; // needs to always be zero, SSE registers are 128-bits
    }

    Vec3( const float comps[3] )
    {
        for( int idx = 0; idx < 3; ++idx )
        {
            m_Comps[ idx ] = comps[ idx ];
        }
        m_Comps[ 3 ] = 0.0f; // needs to always be zero, SSE registers are 128-bits
    }

    float Distance2( const Vec3 &v, const bool use_simd )
    {
        float retVal;

        // Counter-intuitive result: at -O3, the scalar branch below tends to beat
        // the SIMD branch on this workload. Disassembly explains it:
        //
        //   - The scalar path collapses to two `fmadd` (fused multiply-add)
        //     instructions plus two `fsub`. FMA does mul+add in one op, so the
        //     three squared diffs accumulate in just 2 fused ops.
        //
        //   - The SIMD path pays three taxes the scalar path avoids:
        //       1. Lane-3 zeroing — the compiler can't prove m_Comps[3] stays 0
        //          across the load, so it re-zeros lane 3 every call.
        //       2. Horizontal sum — combining the 4 lanes back to a scalar takes
        //          two `faddp` (or vaddvq_f32) with a serial dependency chain.
        //       3. Wasted lane — only 3 of 4 SIMD lanes hold useful data.
        //
        // SIMD wins when you can amortize that setup over many lanes of work
        // (e.g. SoA: test 4 collision pairs in parallel, no horizontal sum, no
        // wasted lane). For a single 3D distance per call, scalar + FMA is hard
        // to beat on a modern out-of-order core.
#if SIMD_AVAILABLE
        if( use_simd )
        {
            simd4f pos0 = simd_load( m_Comps );
            simd4f pos1 = simd_load( v.m_Comps );

            simd4f diff = simd_sub( pos1, pos0 );
            simd4f sqr  = simd_mul( diff, diff );

            retVal = simd_hsum( sqr );
        }
        else
#endif
        {
            float a = v.m_Comps[ 0 ] - m_Comps[ 0 ];
            float b = v.m_Comps[ 1 ] - m_Comps[ 1 ];
            float c = v.m_Comps[ 2 ] - m_Comps[ 2 ];

            retVal = ( ( a * a ) + ( b * b ) + ( c * c ) );
        }

        return retVal;
    }
};

class MyObject
{
private:
    Vec3  m_Pos;
    float m_Radius;

public:
    MyObject()
    {}

    MyObject( const Vec3 &startPos, float size ) :
        m_Pos( startPos ),
        m_Radius( size )
    {}

    MyObject( const float startPos[ 3 ], float size ) :
        m_Pos( startPos ),
        m_Radius( size )
    {}

    const float GetRadius( void )
    {
        return m_Radius;
    }

    const Vec3& GetPosition( void )
    {
        return m_Pos;
    }

    void SetPosition( const Vec3& newPos )
    {
        m_Pos = newPos;
    }

    bool IsCollision( MyObject& thing2, const bool use_simd )
    {
        const Vec3 &p = thing2.GetPosition();
        float      r  = m_Radius + thing2.GetRadius();
        float      d2 = m_Pos.Distance2( p, use_simd );

        return ( ( r * r ) >= d2 );
    }
};

// Pin the current process to core 0 for reproducible benchmarking. Optional.
static bool PinToCoreZero( void )
{
#if defined( _WIN32 )
    return SetProcessAffinityMask( GetCurrentProcess(), 1 ) != 0;
#elif defined( __linux__ )
    cpu_set_t set;
    CPU_ZERO( &set );
    CPU_SET( 0, &set );
    return sched_setaffinity( 0, sizeof( set ), &set ) == 0;
#else
    return false; // macOS and other platforms: no hard pinning available
#endif
}

// Convert a tick count to microseconds using the QPC frequency.
static long long TicksToUsec( long long ticks, long long freq )
{
    return ( ticks * 1000000LL ) / freq;
}

// Print min/median/max statistics for a labeled pass.
static void PrintStats( const char *label, std::vector< long long > &usecs, int hitCount )
{
    std::sort( usecs.begin(), usecs.end() );
    const long long minUs = usecs.front();
    const long long maxUs = usecs.back();
    const long long medUs = usecs[ usecs.size() / 2 ];
    std::cout << label << ": min " << minUs
              << " / med " << medUs
              << " / max " << maxUs
              << " usec (" << hitCount << " collisions over "
              << usecs.size() << " runs)\n";
}

void CollisionTimer( void )
{
    // Allocate 4k pages for the random location data
    float *pPosList = static_cast< float * >( ::malloc( PAGE_COUNT * 4096 ) );
    if( !pPosList )
    {
        std::cout << "Float allocation failed\n";
        return;
    }

    // Fill the allocated space with floats
    static const int objCount = ( PAGE_COUNT * 4096 ) / ( sizeof( float ) * FLOAT_PER_OBJECT );

    int iter;
    static const float radius = 1.0f;
    static const float max = float( RAND_MAX );

    // Generate same list of "rands" each go round
    srand( 3234 );
    for( iter = 0; iter < objCount; iter += FLOAT_PER_OBJECT )
    {
        float floatList[ FLOAT_PER_OBJECT ];
        for( int idx = 0; idx < FLOAT_PER_OBJECT; ++idx )
        {
            // Multiply by 2x radius to get some spacing between the spheres.
            floatList[ idx ] = ( float( rand() ) / max ) * radius * 2;
        }

        ::memcpy( pPosList + iter, floatList, sizeof( float ) * FLOAT_PER_OBJECT );
    }

    // --- Pre-pack data into SoA layout for the third pass --------------------
    // For each iteration i, we need: obj1 = pPosList[ posIndex .. +2 ],
    //                                obj2 = pPosList[ posIndex+3 .. +5 ].
    // posIndex walks pPosList by 6 floats per iteration and wraps when it would
    // run past the end. (Originally written as `(i * 6) % objCount`, but the
    // mod adds a few cycles per iteration that the SoA hot loop doesn't pay —
    // a running counter with a branch-reset is fairer and just as correct.)
    //
    // Layout: a single buffer split into "blocks" of 4 iterations' worth of
    // data. Each block is 24 floats = 96 bytes, organized as 6 lanes of 4
    // contiguous floats each:
    //
    //     [ x1[0..3] | y1[0..3] | z1[0..3] | x2[0..3] | y2[0..3] | z2[0..3] ]
    //
    // The SoA hot loop reads one block per iteration with 6 aligned 16-byte
    // loads at fixed offsets 0/4/8/12/16/20. One base pointer, fully linear
    // access — friendly to the L1 prefetcher and to register pressure.
    static_assert( ITERATIONS % 4 == 0, "ITERATIONS must be a multiple of 4 for SoA pass" );
    static const int FLOATS_PER_BLOCK = 24;
    const int blockCount = ITERATIONS / 4;
    const size_t soaBytes = sizeof( float ) * FLOATS_PER_BLOCK * blockCount;
    float *soa = static_cast< float * >( ::malloc( soaBytes ) );
    if( !soa )
    {
        std::cout << "SoA allocation failed\n";
        ::free( pPosList );
        return;
    }
    {
        int posIndex = 0;
        for( int i = 0; i < ITERATIONS; ++i )
        {
            float *block = soa + ( i / 4 ) * FLOATS_PER_BLOCK;
            int lane = i & 3;
            block[  0 + lane ] = pPosList[ posIndex + 0 ]; // x1
            block[  4 + lane ] = pPosList[ posIndex + 1 ]; // y1
            block[  8 + lane ] = pPosList[ posIndex + 2 ]; // z1
            block[ 12 + lane ] = pPosList[ posIndex + 3 ]; // x2
            block[ 16 + lane ] = pPosList[ posIndex + 4 ]; // y2
            block[ 20 + lane ] = pPosList[ posIndex + 5 ]; // z2
            posIndex += 6;
            if( posIndex + 6 > objCount ) posIndex = 0;
        }
    }

    LARGE_INTEGER freq, t0, t1;
    QueryPerformanceFrequency( &freq );
    int hitCount = 0;
    int firstHitCount = -1;
    std::vector< long long > timings;
    timings.reserve( REP_COUNT );

    // --- Pass 1: scalar -----------------------------------------------------
    timings.clear();
    for( int rep = 0; rep < REP_COUNT; ++rep )
    {
        hitCount = 0;
        QueryPerformanceCounter( &t0 );
        int posIndex = 0;
        for( iter = 0; iter < ITERATIONS; ++iter )
        {
            MyObject obj1( pPosList + posIndex, radius );
            MyObject obj2( pPosList + posIndex + FLOAT_PER_OBJECT, radius );

            if( obj1.IsCollision( obj2, /*use_simd=*/false ) )
            {
                ++hitCount;
            }
            posIndex += 6;
            if( posIndex + 6 > objCount ) posIndex = 0;
        }
        QueryPerformanceCounter( &t1 );
        timings.push_back( TicksToUsec( t1.QuadPart - t0.QuadPart, freq.QuadPart ) );
        if( firstHitCount < 0 ) firstHitCount = hitCount;
        else if( hitCount != firstHitCount )
            std::cout << "WARNING: hit count drift in scalar pass: " << hitCount << " vs " << firstHitCount << "\n";
    }
    PrintStats( "scalar         ", timings, hitCount );

    // --- Pass 2: naive SIMD (one collision per iteration) -------------------
#if SIMD_AVAILABLE
    timings.clear();
    for( int rep = 0; rep < REP_COUNT; ++rep )
    {
        hitCount = 0;
        QueryPerformanceCounter( &t0 );
        int posIndex = 0;
        for( iter = 0; iter < ITERATIONS; ++iter )
        {
            MyObject obj1( pPosList + posIndex, radius );
            MyObject obj2( pPosList + posIndex + FLOAT_PER_OBJECT, radius );

            if( obj1.IsCollision( obj2, /*use_simd=*/true ) )
            {
                ++hitCount;
            }
            posIndex += 6;
            if( posIndex + 6 > objCount ) posIndex = 0;
        }
        QueryPerformanceCounter( &t1 );
        timings.push_back( TicksToUsec( t1.QuadPart - t0.QuadPart, freq.QuadPart ) );
        if( hitCount != firstHitCount )
            std::cout << "WARNING: hit count drift in naive SIMD pass: " << hitCount << " vs " << firstHitCount << "\n";
    }
    PrintStats( "naive SIMD     ", timings, hitCount );
#else
    std::cout << "Naive SIMD pass skipped (no SIMD_AVAILABLE on this target)\n";
#endif

    // --- Pass 3: SoA SIMD (4 collisions per iteration) ----------------------
    // This is the layout SIMD actually likes: each lane is an independent
    // collision, every lane carries useful data, no horizontal sum needed,
    // and the squared-distance accumulation can use FMA when available.
#if SIMD_AVAILABLE
    {
        const simd4f r2 = simd_set1( ( radius + radius ) * ( radius + radius ) );
        timings.clear();
        for( int rep = 0; rep < REP_COUNT; ++rep )
        {
            hitCount = 0;
            QueryPerformanceCounter( &t0 );
            const float *block = soa;
            for( iter = 0; iter < ITERATIONS; iter += 4 )
            {
                simd4f x1 = simd_load( block +  0 );
                simd4f y1 = simd_load( block +  4 );
                simd4f z1 = simd_load( block +  8 );
                simd4f x2 = simd_load( block + 12 );
                simd4f y2 = simd_load( block + 16 );
                simd4f z2 = simd_load( block + 20 );
                block += FLOATS_PER_BLOCK;

                simd4f dx = simd_sub( x2, x1 );
                simd4f dy = simd_sub( y2, y1 );
                simd4f dz = simd_sub( z2, z1 );

                // d2 = dx*dx + dy*dy + dz*dz — compiler should fuse to FMA at -O3.
                simd4f d2 = simd_mul( dx, dx );
                d2 = simd_add( d2, simd_mul( dy, dy ) );
                d2 = simd_add( d2, simd_mul( dz, dz ) );

                simd4f hits = simd_cmpge( r2, d2 );
                hitCount += simd_mask_popcount( hits );
            }
            QueryPerformanceCounter( &t1 );
            timings.push_back( TicksToUsec( t1.QuadPart - t0.QuadPart, freq.QuadPart ) );
            if( hitCount != firstHitCount )
                std::cout << "WARNING: hit count drift in SoA SIMD pass: " << hitCount << " vs " << firstHitCount << "\n";
        }
        PrintStats( "SoA SIMD (x4)  ", timings, hitCount );
    }
#else
    std::cout << "SoA SIMD pass skipped (no SIMD_AVAILABLE on this target)\n";
#endif

    ::free( soa );
    ::free( pPosList );
}

int main( int argc, char *argv[] )
{
    bool pin = false;
    for( int i = 1; i < argc; ++i )
    {
        if( std::strcmp( argv[ i ], "--pin" ) == 0 ) pin = true;
    }
    if( pin )
    {
        if( PinToCoreZero() )
            std::cout << "Pinned to core 0.\n";
        else
            std::cout << "Pinning requested but not available on this platform.\n";
    }
    CollisionTimer();
}
