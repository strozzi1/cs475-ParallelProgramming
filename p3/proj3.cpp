
#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <math.h>
#include <time.h>



unsigned int seed = 0;

const float GRAIN_GROWS_PER_MONTH =		9.0;
const float ONE_DEER_EATS_PER_MONTH =		1.0;

const float AVG_PRECIP_PER_MONTH =		7.0;	// average
const float AMP_PRECIP_PER_MONTH =		6.0;	// plus or minus
const float RANDOM_PRECIP =			2.0;	// plus or minus noise

const float AVG_TEMP =				60.0;	// average
const float AMP_TEMP =				20.0;	// plus or minus
const float RANDOM_TEMP =			10.0;	// plus or minus noise

const float MIDTEMP =				40.0;
const float MIDPRECIP =				10.0;



omp_lock_t	Lock;
int		NumInThreadTeam;
int		NumAtBarrier;
int		NumGone;


/********************
* The state of the 
* System at any give time
* is represented by these global variable
*
*********************/

int	    NowYear;		// 2020 - 2025
int	    NowMonth;		// 0 - 11

float	NowPrecip;		// inches of rain per month
float	NowTemp;		// temperature this month
float	NowHeight;		// grain height in inches
int	    NowNumDeer;		// number of deer in the current population
int     NowNumWolves;



/*********
 * Function prototypes
 *
********/

float Ranf( unsigned int *,  float , float );
int Ranf( unsigned int *, int , int  );
void	InitBarrier( int );
void	WaitBarrier( );
float SQR( float  );

void GrainDeer( );
void Grain( );
void Watcher( );
void Agent( );
void computeNextWeather();



int main (int argc, char *argv[ ]){

    float x = Ranf( &seed, -1.f, 1.f );
    // starting date and time:
    NowMonth =    0;
    NowYear  = 2020;

    computeNextWeather();
    printf("Temp, Precip, Grain, Deer, Wolves\n");
    // starting state (feel free to change this if you want):
    NowNumDeer = 4;
    NowHeight =  30.;
    NowNumWolves = 1;

    omp_set_num_threads( 4 );	// same as # of sections
    //InitBarrier(3);
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            Grain( );
        }

        #pragma omp section
        {
            
            GrainDeer( );
        }

        #pragma omp section
        {
            Watcher( );
        }

        #pragma omp section
        {
            Agent( );	// your own
        }
    }       // implied barrier -- all functions must return in order
        // to allow any of them to get past here


    return 0;
}


void computeNextWeather(){
    float ang = (  30.*(float)NowMonth + 15.  ) * ( M_PI / 180. );

    float temp = AVG_TEMP - AMP_TEMP * cos( ang );
    NowTemp = temp + Ranf( &seed, -RANDOM_TEMP, RANDOM_TEMP );

    float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin( ang );
    NowPrecip = precip + Ranf( &seed,  -RANDOM_PRECIP, RANDOM_PRECIP );
    if( NowPrecip < 0. )
        NowPrecip = 0.;
}

void Grain( ){
    while( NowYear < 2026 )
    {
        // compute a temporary next-value for this quantity
        // based on the current state of the simulation:
        float tempFactor = exp(   -SQR(  ( NowTemp - MIDTEMP ) / 10.  )   );

        float precipFactor = exp(   -SQR(  ( NowPrecip - MIDPRECIP ) / 10.  )   );

        float nextHeight = NowHeight;
        nextHeight += tempFactor * precipFactor * GRAIN_GROWS_PER_MONTH;
        nextHeight -= (float)NowNumDeer * ONE_DEER_EATS_PER_MONTH;
        nextHeight = (nextHeight < 0 ) ? 0 : nextHeight;

        // DoneComputing barrier:
        #pragma omp barrier
        //assigning
        NowHeight = nextHeight;

        // DoneAssigning barrier:
        #pragma omp barrier
        //DO NOTHING

        // DonePrinting barrier:
        #pragma omp barrier
        //do nothing
    }

}



void GrainDeer( ){
    while( NowYear < 2026 )
    {
        // compute a temporary next-value for this quantity
        // based on the current state of the simulation:
        //COMPUTE NEXT
        int nextNumDeer = (NowNumDeer < NowHeight) ? NowNumDeer+1 : (NowNumDeer >NowHeight) ? NowNumDeer-1 : NowNumDeer;
        nextNumDeer -= NowNumWolves / 2;

        // DoneComputing barrier:
        #pragma omp barrier
        // assign NEXT
        
        NowNumDeer=nextNumDeer;
        // DoneAssigning barrier:
        #pragma omp barrier
        //DO NOTHING

        // DonePrinting barrier:
        #pragma omp barrier
        //DO NOTHING
    }

}



void Watcher( ){
    while( NowYear < 2026 )
    {
        
        //DO NOTHING

        // DoneComputing barrier:
        #pragma omp barrier
        //DO NOTHING

        // DoneAssigningBarrier
        #pragma omp barrier
        //print stuff!!!
        printf("%f, %f, %f, %d, %d\n", NowTemp, NowPrecip, NowHeight, NowNumDeer, NowNumWolves);
        NowMonth = (NowMonth == 11) ? 0 : NowMonth+1;
        NowYear = (NowMonth == 0) ? NowYear + 1 : NowYear;
        computeNextWeather();
        // DonePrinting barrier:
        #pragma omp barrier
        ///DO NOTHING
    }

}

void Agent( ){
    while( NowYear < 2026 )
    {
        // compute a temporary next-value for this quantity
        // based on the current state of the simulation:
        int nextNumWolves =((NowNumDeer/2) > NowNumWolves ) ? NowNumWolves + 1: (NowNumWolves > (NowNumDeer/2)) ? NowNumWolves-1 : NowNumWolves;

        // DoneComputing barrier:
        #pragma omp barrier
        //. . .
        NowNumWolves = nextNumWolves;

        // DoneAssigning barrier:
        #pragma omp barrier
        //. . .

        // Do NOTHING
        #pragma omp barrier
        // Do NOTHING
    }

}





/*
STUFF
AND
THINGS
 */


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

        while( NumAtBarrier != 0 );	// this waits for the nth thread to arrive

        #pragma omp atomic
        NumGone++;			// this flags how many threads have returned
}



float
Ranf( unsigned int *seedp,  float low, float high )
{
        float r = (float) rand_r( seedp );              // 0 - RAND_MAX

        return(   low  +  r * ( high - low ) / (float)RAND_MAX   );
}


int
Ranf( unsigned int *seedp, int ilow, int ihigh )
{
        float low = (float)ilow;
        float high = (float)ihigh + 0.9999f;

        return (int)(  Ranf(seedp, low,high) );

}

float
SQR( float x )
{
        return x*x;
}