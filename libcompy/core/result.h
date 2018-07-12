#ifndef __COMPY_CORE_RESULT_H
#define __COMPY_CORE_RESULT_H




#include <stdint.h>




//! \brief Success.
#define COMPY_E_SUCCESS						0

//! \brief `NULL` pointer encountered.
#define COMPY_E_NULL						1

//! \brief Parameter invalid.
#define COMPY_E_PARAM						2

//! \brief Object locked.
#define COMPY_E_LOCKED						3

//! \brief Object not ready.
#define COMPY_E_NOT_READY					4

//! \brief Input/output error.
#define COMPY_E_IO							5

//! \brief Object is in an invalid state.
#define COMPY_E_STATE						6

//! \brief Invalid data.
#define COMPY_E_DATA						7




typedef unsigned int compy_result_t;




#endif
