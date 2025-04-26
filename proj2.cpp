#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

// Global Variables - "State" of the system
int	NowYear =       2025;		      // 2025- 2030
int	NowMonth =      0;		        // 0 - 11

float	NowPrecip;		              // inches of rain per month
float	NowTemp;		                // temperature this month
float	NowHeight =   5;		        // grain height in inches
int	NowNumDeer =    2;		        // number of deer in the current population
// int NowNumWolves = 1;

unsigned int seed = 0;

// Constants
const float GRAIN_GROWS_PER_MONTH =	       12.0;
const float ONE_DEER_EATS_PER_MONTH =		    1.0;

const float AVG_PRECIP_PER_MONTH =		      7.0;	// average
const float AMP_PRECIP_PER_MONTH =		      6.0;	// plus or minus
const float RANDOM_PRECIP =			            2.0;	// plus or minus noise

const float AVG_TEMP =				              60.0;	// average
const float AMP_TEMP =				              20.0;	// plus or minus
const float RANDOM_TEMP =			              10.0;	// plus or minus noise

const float MIDTEMP =				                40.0;
const float MIDPRECIP =				              10.0;
