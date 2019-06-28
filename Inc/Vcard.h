

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __VCARD_H
#define __VCARD_H

#ifdef __cplusplus
extern "C" {
#endif


#include "stm32f1xx_hal.h"
#include "Get_B.h"

uint8_t VirtCard();
void InitVirtCard();
void conv(uint32_t *a, Data *d);
void prndata(void);
void vc(void);

#ifdef __cplusplus
}
#endif

#endif /* __VCARD_H */


