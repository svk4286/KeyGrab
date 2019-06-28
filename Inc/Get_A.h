
#ifndef __GET_A_H
#define __GET_A_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "Get_B.h"
  
uint8_t getStream_A(uint8_t *cnt, uint8_t *n, uint32_t timeout);
uint8_t ConvertStream_A(uint8_t *cnt, uint8_t n, Data *data, uint8_t *nbuf, uint8_t *lastbit);
uint8_t waitRF( uint32_t timeout );

#endif      //__GET_A_H
