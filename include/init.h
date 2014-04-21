#ifndef __BL_INIT_H
#define __BL_INIT_H

void bl_init(void);
void rcc_enable(void);
void rcc_disable(void);
void nvic_disable(void);
void nvic_enable(void);

#endif /* __BL_INIT_H */
