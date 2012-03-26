#pragma once



typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef int int32_t;
typedef short int16_t;



uint32_t half_to_float( uint16_t h );
uint16_t half_from_float( uint32_t f );
uint16_t half_add( uint16_t arg0, uint16_t arg1 );

static inline uint16_t half_sub( uint16_t ha, uint16_t hb )
{
  // (a-b) is the same as (a+(-b))
  return half_add( ha, hb ^ 0x8000 );
}


