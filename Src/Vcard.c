
#include <stdlib.h>
#include "Vcard.h"
#include "Get_A.h"
#include "Put_A.h"
#include "pn532.h"
#include "Macros.h"


extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern SPI_HandleTypeDef hspi1;
extern UART_HandleTypeDef huart1;
extern __IO uint32_t uwTick;
uint16_t streamB[900],nstrB;
uint16_t streamB_1[900],nstrB_1;
uint16_t streamNAK[] = {763,83,83,83,423,83,83,83,83,83,83,83,763,83,83,83,83}, nstrNAK = 17;

uint8_t data[20][20];
uint8_t ndata[20];
uint8_t nd;
uint8_t uid[8] = {0xEB, 0xD5, 0x2A, 0xC5},uidLength = 4,nuid;  //  UID
char Tx[512];
uint16_t  fv_1;


void InitPN532_1(){

	uint16_t Adr;
	uint8_t r1;
	static uint8_t r2 =0x13;				// ”силение сигнала

	CS_LOW(&hspi1);
	HAL_Delay(1);
	CS_HIGH(&hspi1);
	while (!(fv_1 = getFirmwareVersion(&hspi1)));
	SAM_VirtualCard(&hspi1);
	HAL_Delay(1000);
	Adr = 0x6106;							// CIU_RFCfg register (6316h)
	readRegister(&hspi1, &Adr, &r1, 1);
	r1 = r2;
	writeRegister(&hspi1, Adr, r1);

}


void InitVirtCard(){
  
  HAL_TIM_OC_Start  ( &htim1, TIM_CHANNEL_4 );
  HAL_TIM_PWM_Start  ( &htim2, TIM_CHANNEL_4 );

}


void conv(uint32_t *a, uint8_t *d){
	*a = d[0];
	for(int i = 1; i < 4; i++){
		*a <<= 8;
		*a += d[i];
	}
}


void prndata(){

//	return;
	uint8_t t, i, j;
	char *p;
	for(i = 0; i < (nd); i++){
		p = Tx;
		t = sprintf((char *)p,"\n\rReader  ");
		p += t;
		for(j = 0; j < ndata[i]; j++){
			t = sprintf((char *)p," 0x%2.2X",data[i][j]);
			p += t;
		}
		HAL_UART_Transmit(&huart1, (uint8_t *)Tx, (p - Tx), 1000);
	}
}


uint8_t VirtCard(void){

	static uint8_t w = 0;
	uint32_t TagChal;
	uint8_t dt[4];
	uint8_t k[] = {'A','B'};
	uint8_t ask1[] = {0x04, 0x00,}, n1= 2;
	uint8_t ask2[] = {0x08, 0xb6, 0xdd }, n3 = 3;
	uint8_t tmp, *p;
	uint8_t cnt[170], ncnt;
	uint8_t  lastbit = 0;
	uint8_t scs, t, i;

	nd = 0;

	__HAL_TIM_SET_COUNTER(&htim1, 0);
	srand(uwTick);

	while(!(scs = getStream_A(cnt, &ncnt, 1000)) || !ncnt){			// ќжидаем 1-й посылки
		if(!w){
			t = sprintf((char *)Tx,"\n\rWaiting for  Reader");
			HAL_UART_Transmit(&huart1,(uint8_t *)Tx,t,1000);
			w = 1;
		}
		else{
			tmp = ',';
			HAL_UART_Transmit(&huart1,&tmp,1,1000);
		}
	}

	scs = ConvertStream_A(cnt, ncnt, data[nd], &ndata[nd], &lastbit);
	if ((ndata[nd] != 1) || (data[nd][0] != 0x26) || !scs) {
		return 2;
	}
	w = 0;
	nd++;
	PutData_A(ask1, n1, lastbit);

	scs = getStream_A(cnt, &ncnt, 200);
	if (!scs || !ncnt) {
		return 1;
	}
	scs = ConvertStream_A(cnt, ncnt, data[nd], &ndata[nd], &lastbit);
	if ((ndata[nd] != 2) || !scs || (data[nd][0] != 0x93)
			|| (data[nd][1] != 0x20)) {
		return 3;
	}

	B10_STROBE;

	nd++;
	PutData_A(uid, nuid, lastbit);

	scs = getStream_A(cnt, &ncnt, 200);
	if (!scs || !ncnt) {
		return 1;
	}
	scs = ConvertStream_A(cnt, ncnt, data[nd], &ndata[nd], &lastbit);
	if ((ndata[nd] != 9) || !scs || (data[nd][0] != 0x93) || (data[nd][1] != 0x70)) {
		return 4;
	}
	nd++;
	PutData_A(ask2, n3, lastbit);

	while (1) {
		scs = getStream_A(cnt, &ncnt, 200);
		if (!scs || !ncnt) {
			return 1;
		}
		scs = ConvertStream_A(cnt, ncnt, data[nd], &ndata[nd], &lastbit);
		if ((ndata[nd] == 1) && (data[nd][0] == 0x26) && scs) {
			return 5;
		}
		if ((ndata[nd] == 4) && scs
				&& ((data[nd][0] == 0x60) || (data[nd][0] == 0x61))) {
			TagChal = rand();
			p = (uint8_t *) (&TagChal);
			for (i = 0; i < 4; i++) {
				dt[3 - i] = *(p++);
			}
			nd++;
			PutData_A(dt, 4, lastbit);                              // Send  Tag Challenge
			break;
		}
		nd++;
		PutStream_A(streamNAK, nstrNAK, lastbit);					// Send NAK
	}

	scs = getStream_A(cnt, &ncnt, 200);
	if (!scs) {
		return 6;
	}
	scs = ConvertStream_A(cnt, ncnt, data[nd], &ndata[nd], &lastbit);
	if ((ndata[nd] != 8) || !scs) {
		return 7;
	}
	nd++;
	prndata();

	uint32_t AB1, AB2;
	conv(&AB1, data[nd - 1]);
	conv(&AB2, &data[nd - 1][4]);

	t = sprintf((char *) Tx,"\n\r\n\r\
Block  %d            Key  %c\n\r\
UID                 0x%2.2X%2.2X%2.2X%2.2X\n\r\
Tag Challenge       0x%8.8X\n\r\
Reader Challenge    0x%8.8X\n\r\
Reader Response     0x%8.8X\n\r",data[nd-2][1],\
k[data[nd-2][0] - 0x60], uid[0],uid[1],uid[2],uid[3], TagChal,AB1,AB2);
	HAL_UART_Transmit(&huart1, (uint8_t *)Tx, t, 1000);

	w = 0;
    HAL_Delay(1000);
    return 0;
}


void vc(){

	uint8_t err,t;
	if((err = VirtCard())){
		t = sprintf((char *) Tx,"\n\rERROR     %d\n\r",err);
		HAL_UART_Transmit(&huart1, (uint8_t *)Tx, t, 1000);
		prndata();
		HAL_Delay(1000);
	}
}
