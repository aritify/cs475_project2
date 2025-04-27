#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

// Global Variables - "State" of the system
int	NowYear =       2025;		      	// 2025- 2030
int	NowMonth =      0;		        	// 0 - 11

float	NowPrecip;		              	// inches of rain per month
float	NowTemp;		                // temperature this month
float	NowHeight =   5.;		        // grain height in inches
int		NowNumDeer =    2;		        // number of deer in the current population
// int NowNumWolves = 1;

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
void 	Watcher();
void 	MyAgent();
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

    omp_set_num_threads( 3 );	// same as # of sections
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
    		Watcher( );
    	}
    
    	/*#pragma omp section
    	{
    		MyAgent( );	// your own
    	}*/
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
	
		// DoneComputing barrier:
		WaitBarrier( );
	
		// DoneAssigning barrier:
		WaitBarrier( );
	
		// DonePrinting barrier:
		WaitBarrier( );
	}
}


void
Grain()
{
  
}


void
Watcher()
{
  
}


void
MyAgent()
{
  
}


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
