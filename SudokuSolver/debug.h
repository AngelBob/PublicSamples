#pragma once

#if defined _DEBUG
#define ASSERT( condition )                                                              \
if( !(condition) )                                                                       \
{                                                                                        \
    std::cout << "Assert failed @ " << __FILE__ << ":" << __LINE__ << " " << #condition; \
    __debugbreak();                                                                      \
}
#else
#define ASSERT( condition )
#endif

#if defined _DEBUG
static constexpr bool is_debug_enabled = true;
#else
static constexpr bool is_debug_enabled = false;
#endif
