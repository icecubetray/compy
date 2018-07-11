#ifndef __SC_RESULT_H
#define __SC_RESULT_H




#include <stdint.h>




//! \brief Success.
#define SC_E_SUCCESS						0

//! \brief `NULL` pointer encountered.
#define SC_E_NULL							1

//! \brief Parameter invalid.
#define SC_E_PARAM							2

//! \brief Object locked.
#define SC_E_LOCKED							3

//! \brief Object not ready.
#define SC_E_NOT_READY						4

//! \brief Input/output error.
#define SC_E_IO								5

//! \brief Object is in an invalid state.
#define SC_E_STATE							6

//! \brief Invalid data.
#define SC_E_DATA							7




typedef unsigned int sc_result_t;




#endif
