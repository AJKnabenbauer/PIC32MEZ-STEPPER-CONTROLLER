/* 
 * File:   pulse_handler.h
 * Author: Andrew
 *
 * Created on May 17, 2021, 4:39 AM
 */

#ifndef PULSE_HANDLER_H
#define	PULSE_HANDLER_H

#include "definitions.h" 

#define PH_FUNC_NAME_I(a,b) a ## _ ## b
#define PH_FUNC_NAME(a,b) PH_FUNC_NAME_I(a,b)

#define PH_TMR TMR3

#define PH_TMR_Start            PH_FUNC_NAME(PH_TMR, Start)
#define PH_TMR_Stop             PH_FUNC_NAME(PH_TMR, Stop)
#define PH_TMR_PreiodSet        PH_FUNC_NAME(PH_TMR, PeriodSet)
#define PH_TMR_PreiodGet        PH_FUNC_NAME(PH_TMR, PeriodGet)
#define PH_TMR_CounterGet       PH_FUNC_NAME(PH_TMR, CounterGet)
#define PH_TMR_FrequencyGet     PH_FUNC_NAME(PH_TMR, FrequencyGet)

#define PH_OCMP OCMP4

#define PH_OCMP_Enable          PH_FUNC_NAME(PH_OCMP, Enable)
#define PH_OCMP_Disable         PH_FUNC_NAME(PH_OCMP, Disable)
#define PH_OCMP_CompareValueSet PH_FUNC_NAME(PH_OCMP, CompareValueSet)
#define PH_OCMP_CompareValueGet PH_FUNC_NAME(PH_OCMP, CompareValueGet)





#endif	/* PULSE_HANDLER_H */

