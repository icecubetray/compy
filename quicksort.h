#ifndef __SC_QUICKSORT_H
#define __SC_QUICKSORT_H




#include "./types.h"




#define SC_QS_MODE_ASCENDING				1
#define SC_QS_MODE_DESCENDING				0




typedef struct sc_qs_pair {
	sc_qs_t qsvalue;
	sc_qs_t tag;
} sc_qs_pair_t;




#ifdef __cplusplus
extern "C" {
#endif

	void sc_quicksort(sc_qs_pair_t *const array, size_t index, const size_t size, const int mode);

#ifdef __cplusplus
}
#endif




#endif
