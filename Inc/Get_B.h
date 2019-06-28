#ifndef __GET_B_H
#define __GET_B_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
  
#define MaxStreamSize 1700

typedef struct{
	uint8_t d;
	uint8_t p;
} Data;

uint8_t insbit(Data *buf, uint8_t *ibit, uint8_t *ibyte, uint8_t *par, uint8_t b);

uint8_t ConvertStream_B(uint16_t *stream, uint16_t ns, Data *data, uint8_t nd, uint8_t *ost );
uint8_t getStream_B(uint16_t *stream, uint16_t *n);

#endif      //__GET_B_H
