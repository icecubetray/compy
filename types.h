#ifndef __SC_TYPES_H
#define __SC_TYPES_H




#include <stdint.h>
#include <stddef.h>

#include "./result.h"




// Quicksortable type.
typedef size_t sc_qs_t;

typedef enum sc_direction {
	SC_DIRECTION_FORWARD = 0,
	SC_DIRECTION_BACKWARD = 1
} sc_direction_t;




#endif
