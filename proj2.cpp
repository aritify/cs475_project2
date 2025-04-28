#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

// Global Variables - "State" of the system
int	NowYear =      	 2025;		      	// 2025- 2030
int	NowMonth =      	0;		        // 0 - 11

float	NowPrecip;		              	// inches of rain per month
float	NowTemp;		                // temperature this month
float	NowHeight = 	7.;		        // grain height in inches
int		NowNumDeer =   	4;		        // number of deer in the current population
int 	NowNumWolf = 	0;

// Barrier Global Variables
omp_lock_t		Lock;
volatile int	NumInThreadTeam;
volatile int	NumAtBarrier;
volatile int	NumGone;

// Seed (Global)
unsigned int seed = 0;

// Constants
const float GRAIN_GROWS_PER_MONTH =	       		12.0;
const float ONE_DEER_EATS_PER_MONTH =		   	1.0;

const float AVG_PRECIP_PER_MONTH =		      	7.0;	// average
const float AMP_PRECIP_PER_MONTH =		      	6.0;	// plus or minus
const float RANDOM_PRECIP =			            2.0;	// plus or minus noise

const float AVG_TEMP =				            60.0;	// average
const float AMP_TEMP =				            20.0;	// plus or minus
const float RANDOM_TEMP =			            10.0;	// plus or minus noise

const float MIDTEMP =				            40.0;
const float MIDPRECIP =				            10.0;

// Function Prototypes
void 	Deer();
void 	Grain();
void 	Wolf();
void 	Watcher();
float 	SQR( float x );
void	InitBarrier( int );
void	WaitBarrier( );
float	Ranf( float, float );
int		Ranf( int, int );


// main program:
int
main( int argc, char *argv[ ] )
{
#ifndef _OPENMP
	fprintf( stderr, "No OpenMP support!\n" );
	return 1;
#endif

    omp_set_num_threads( 4 );	// or 4. Same as # of sections
	InitBarrier( 4 );			// or 4
    #pragma omp parallel sections
    {
    	#pragma omp section
    	{
    		Deer( );
    	}
    
    	#pragma omp section
    	{
    		Grain( );
    	}

		#pragma omp section
    	{
    		Wolf( );	// your own
    	}
    
    	#pragma omp section
    	{
    		Watcher( );
    	}
    
    }       // implied barrier -- all functions must return in order
            // to allow any of them to get past here
    return 0;
}


void
Deer()
{
	while( NowYear < 2031 )
	{
		// compute a temporary next-value for this quantity
		// based on the current state of the simulation:
		int nextNumDeer = NowNumDeer;
		int carryingCapacity = (int)( NowHeight );
		if( nextNumDeer < carryingCapacity ) {
        	nextNumDeer++;
		}
		else
        	if( nextNumDeer > carryingCapacity )
                nextNumDeer--;

		if( nextNumDeer < 0 )
        	nextNumDeer = 0;
	
		// DoneComputing barrier:
		WaitBarrier( );
		NowNumDeer = nextNumDeer;		// Assign the calculated number of deer to the global for the next global cycle
	
		// DoneAssigning barrier:
		WaitBarrier( );
	
		// DonePrinting barrier:
		WaitBarrier( );
	}
}


void
Grain()
{
	while( NowYear < 2031 )
	{
		// compute a temporary next-value for this quantity
		// based on the current state of the simulation:
		float tempFactor = exp(   -SQR(  ( NowTemp - MIDTEMP ) / 10.  )   );
		float precipFactor = exp(   -SQR(  ( NowPrecip - MIDPRECIP ) / 10.  )   );

		float nextHeight = NowHeight;
		nextHeight += tempFactor * precipFactor * GRAIN_GROWS_PER_MONTH;
		nextHeight -= (float)NowNumDeer * ONE_DEER_EATS_PER_MONTH;
		if( nextHeight < 0. ) nextHeight = 0.;

		// DoneComputing barrier:
		WaitBarrier( );
		NowHeight = nextHeight;			// Set next height to the global NowHeight for next month

		// DoneAssigning barrier:
		WaitBarrier( );

		// DonePrinting barrier:
		WaitBarrier( );
	}
}


void
Wolf()
{
	while( NowYear < 2031 )
	{
		// compute a temporary next-value for this quantity
		// based on the current state of the simulation:
		int nextNumWolf = NowNumWolf;
		int carryingCapacity = NowNumDeer;
		if ( nextNumWolf < carryingCapacity ) {
			int wolfAttract = Ranf(0, 9);	// 10% chance to attract wolves if NumDeer is high enough
			if (wolfAttract == 0)
				nextNumWolf++;
		}
		else
			if ( nextNumWolf > carryingCapacity )
				nextNumWolf--;
		
		if( nextNumWolf < 0 )
			nextNumWolf = 0;

		// DoneComputing barrier:
		WaitBarrier( );
		NowNumWolf = nextNumWolf;
		NowNumDeer = NowNumDeer - nextNumWolf;
		if ( NowNumDeer < 0 )
			NowNumDeer = 0;

		// DoneAssigning barrier:
		WaitBarrier( );

		// DonePrinting barrier:
		WaitBarrier( );
	}
}


void
Watcher()
{
	FILE *csv;
	csv = fopen("output.csv", "w+");

	while( NowYear < 2031 )
	{
		// compute a temporary next-value for this quantity
		// based on the current state of the simulation:
		float ang = (  30.*(float)NowMonth + 15.  ) * ( M_PI / 180. );	// angle of earth around the sun

		float temp = AVG_TEMP - AMP_TEMP * cos( ang );
		NowTemp = temp + Ranf( -RANDOM_TEMP, RANDOM_TEMP );

		float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin( ang );
		NowPrecip = precip + Ranf( -RANDOM_PRECIP, RANDOM_PRECIP );
		if( NowPrecip < 0. )
			NowPrecip = 0.;
	
		// DoneComputing barrier:
		WaitBarrier( );		// Wait for Deer and Grain to finish assigning
	
		// DoneAssigning barrier:
		WaitBarrier( );
		/* Printing values to screen */
		/* Month, Temp, Precipitation, Grain Height, NumDeer, NumWolf */
		/* CSV output, Convcert from in>cm, F>C */
		fprintf(csv, "%5d, %5.2f, %5.2f, %5.2f, %5d, %5d\n",
			  (NowMonth+(NowYear-2025)*12), ( (5./9.)*( NowTemp-32 ) ), NowPrecip*2.54, NowHeight*2.54, NowNumDeer, NowNumWolf);

		/* Increment NowMonth and NowYear */
		if (NowMonth >= 11) {		// Months are 0-11
			NowMonth = 0;
			NowYear++;
		}
		else {
			NowMonth++;
		}
	
		// DonePrinting barrier:
		WaitBarrier( );
	}
	fclose(csv);
}


// Helper Functions
float
SQR( float x )
{
	return x*x;
}


// specify how many threads will be in the barrier:
// (also init's the Lock)
void
InitBarrier( int n )
{
	NumInThreadTeam = n;
	NumAtBarrier = 0;
	omp_init_lock( &Lock );
}


// have the calling thread wait here until all the other threads catch up:
void
WaitBarrier( )
{
	omp_set_lock( &Lock );
	{
		NumAtBarrier++;
		if( NumAtBarrier == NumInThreadTeam )
		{
			NumGone = 0;
			NumAtBarrier = 0;
			// let all other threads get back to what they were doing
			// before this one unlocks, knowing that they might immediately
			// call WaitBarrier( ) again:
			while( NumGone != NumInThreadTeam-1 );
			omp_unset_lock( &Lock );
			return;
		}
	}
	omp_unset_lock( &Lock );

	while( NumAtBarrier != 0 );		// this waits for the nth thread to arrive

	#pragma omp atomic
	NumGone++;						// this flags how many threads have returned
}


float
Ranf( float low, float high )
{
	float r = (float) rand();               // 0 - RAND_MAX
	float t = r  /  (float) RAND_MAX;       // 0. - 1.

	return   low  +  t * ( high - low );
}


int
Ranf( int ilow, int ihigh )
{
	float low = (float)ilow;
	float high = ceil( (float)ihigh );

	return (int) Ranf(low,high);
}
