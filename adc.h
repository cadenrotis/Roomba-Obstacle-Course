/*
 * adc.h
 *
 */

#include <stdint.h>
#include <inc/tm4c123gh6pm.h>

void adc_init(void);

int adc_read(void);
