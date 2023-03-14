#ifndef SEGMENTS_H
#define SEGMENTS_H

#include <rkhv/segments.h>
#include <rkhv/stdint.h>

void segments_setup(void);
void segments_set_new_segs(uint16_t cs, uint16_t ds);

#endif
