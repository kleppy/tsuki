#ifndef PIC_H
#define PIC_H

#include <stdint.h>

void pic_init(void);
void pic_send_eoi(uint8_t irq);
void pic_remap(uint8_t master_offset, uint8_t slave_offset);
void pic_clear_masks(void);

#endif