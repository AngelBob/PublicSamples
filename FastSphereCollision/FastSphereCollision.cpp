// FastSphereCollision.cpp : This file contains the 'main' function. Program execution begins and ends there.
// Option to use either:
// a) SSE instructions to parallelize the distance calculation, or
// b) normal serialized instructions, or
// c) both to compare speed.

#include <Windows.h>

#include <iostream>

// Include the SSE intrinsic functions
#include <intrin.h>

//***  Various knobs that control the operation of this code. Changes require rebuild. ***//

    // SSE (parallel) vs. Scalar (serial) calculations
    bool USE_SSE = true;                    // turn this off to use scalar math

    // Performance run settings.
    static const bool PERF_RUN  = false;    // turn this on to check the performance (use Release build)
    static const int ITERATIONS = 1000000;  // this is the number of collision tests to run
    static const int PAGE_COUNT = 1;        // the number of pages of random position data to generate (disconnects data allocation from iteration count)

//*** End Knobs ***//

static const int FLOAT_PER_OBJECT = 3;

class Vec3
{
private:
    __declspec( align( 16 ) ) float m_Comps[ 4 ];
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

    float Distance2( const Vec3 &v )
    {
        float retVal;

        if( USE_SSE )
        {
            __m128 pos0 = _mm_load_ps( m_Comps );
            __m128 pos1 = _mm_load_ps( v.m_Comps );

            __m128 diff = _mm_sub_ps( pos1, pos0 );
            __m128 sqr  = _mm_mul_ps( diff, diff );

            __m128 sum  = _mm_hadd_ps( sqr, sqr );
            __m128 sum2 = _mm_hadd_ps( sum, sum );

            retVal = _mm_cvtss_f32( sum2 );
        }
        else
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

    bool IsCollision( MyObject& thing2 )
    {
        const Vec3 &p = thing2.GetPosition();
        float      r  = m_Radius + thing2.GetRadius();
        float      d2 = m_Pos.Distance2( p );

        return ( ( r * r ) >= d2 );
    }
};

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
    for( iter = 0; iter < objCount; iter += 6 )
    {
        float floatList[ FLOAT_PER_OBJECT ];
        for( int idx = 0; idx < FLOAT_PER_OBJECT; ++idx )
        {
            // Multiply by 2x radius to get some spacing between the spheres.
            floatList[ idx ] = ( float( rand() ) / max ) * radius * 2;
        }

        ::memcpy_s( pPosList + iter, sizeof( float ) * FLOAT_PER_OBJECT, floatList, sizeof( float ) * FLOAT_PER_OBJECT );
    }

    LARGE_INTEGER et, freq;
    int hitCount = 0;
    QueryPerformanceFrequency( &freq );
    et.QuadPart = 0;

    while( 1 )
    {
        for( iter = 0; iter < ITERATIONS; ++iter )
        {
            // 2 Randomly positioned objects, ensure the position data does not run past the end of the data allocation
            int posIndex = ( iter * 6 ) % objCount;
            MyObject obj1( pPosList + posIndex, radius );
            MyObject obj2( pPosList + posIndex + FLOAT_PER_OBJECT, radius );

            // Catch the start time
            LARGE_INTEGER time[ 2 ];
            QueryPerformanceCounter( &time[ 0 ] );

            bool isHit = obj1.IsCollision( obj2 );

            // Catch the end time, and increment the elapsed time.
            QueryPerformanceCounter( &time[ 1 ] );
            et.QuadPart += time[ 1 ].QuadPart - time[ 0 ].QuadPart;

            if( isHit )
            {
                ++hitCount;
            }
        }

        // Do fancy math to convert elapsed time to microseconds.
        et.QuadPart *= 1000000;
        et.QuadPart /= freq.QuadPart;

        std::cout << "Time to iterate " << iter << " tests " << et.QuadPart;

        if( USE_SSE )
        {
            // Output results of SSE run
            std::cout << " usec with SSE";
            std::cout << " (" << hitCount << " collisions).\n";

            // Go again, this time without SSE
            hitCount = 0;
            USE_SSE = false;
        }
        else
        {
            // Output results of scalar run.
            // NOTE:  hitCount must be the same between the two runs.
            std::cout << " usec without SSE";
            std::cout << " (" << hitCount << " collisions).\n";

            // Exit the loop
            break;
        }

    }

    ::free( pPosList );
}

void CollisionLogicTest( void )
{
    const float radius = 0.5f;
    const Vec3 testPositions[][ 2 ] =
    {
        // Collisions
        { { 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, },
        { { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, },
        { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, },

        { { 0.0f, 0.0f, 0.0f }, { 0.7071f, 0.7071f, 0.0f    }, },
        { { 0.0f, 0.0f, 0.0f }, { 0.0f   , 0.7071f, 0.7071f }, },
        { { 0.0f, 0.0f, 0.0f }, { 0.7071f, 0.0f   , 0.7071f }, },

        { { 0.0f, 0.0f, 0.0f }, { 0.5773f, 0.5773f, 0.5773f }, },

        // No collision
        { { 0.0f, 0.0f, 0.0f }, { 1.1f, 0.0f, 0.0f }, },
        { { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.1f, 0.0f }, },
        { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.1f }, },

        { { 0.0f, 0.0f, 0.0f }, { 0.7072f, 0.7072f, 0.0f    }, },
        { { 0.0f, 0.0f, 0.0f }, { 0.0f   , 0.7072f, 0.7072f }, },
        { { 0.0f, 0.0f, 0.0f }, { 0.7072f, 0.0f   , 0.7072f }, },

        { { 0.0f, 0.0f, 0.0f }, { 0.5774f, 0.5774f, 0.5774f }, },

        { { 1.1f, 2.2f, 3.3f }, { 4.4f, 5.5f, 6.6f }, },
    };
    static const int testCount = sizeof( testPositions ) / sizeof( testPositions[ 0 ] );

    for( int i = 0; i < testCount; ++i )
    {
        MyObject obj1( testPositions[ i ][ 0 ], radius );
        MyObject obj2( testPositions[ i ][ 1 ], radius );

        if( obj1.IsCollision( obj2 ) )
        {
            std::cout << "Collision detected, ouch.\n";
        }
        else
        {
            std::cout << "No Collision, you're safe.\n";
        }
    }
}

int main()
{
    if constexpr( PERF_RUN )
    {
        // Perf run wants to run both SSE and scalar, so need to over-ride USE_SSE setting here.
        USE_SSE = true;
        CollisionTimer();
    }
    else
    {
        CollisionLogicTest();
    }
}
