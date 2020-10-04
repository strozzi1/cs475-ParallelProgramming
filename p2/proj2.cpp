#define _USE_MATH_DEFINES
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <omp.h>

// setting the number of threads:
#ifndef NUMT
#define NUMT	4
#endif

// setting the number of threads:
#ifndef NUMNODES
#define NUMNODES	512
#endif


#define XMIN     -1.
#define XMAX      1.
#define YMIN     -1.
#define YMAX      1.
#define N      	  4.

float Height( int, int );
void		TimeOfDaySeed();


int main( int argc, char *argv[ ] )
{
	
	
	TimeOfDaySeed();		// seed the random number generator

	omp_set_num_threads(NUMT);

	// the area of a single full-sized tile:

	float fullTileArea = (  ( ( XMAX - XMIN )/(float)(NUMNODES-1) )  *
				( ( YMAX - YMIN )/(float)(NUMNODES-1) )  );
	//float halfTileArea = fullTileArea/2.;
	//float quarterTileArea = halfTileArea/2.;
	// sum up the weighted heights into the variable "volume"
	// using an OpenMP for loop and a reduction:
	float volume =0.;
	float maxPerformance = 0.;
	double time0 = omp_get_wtime();
#pragma omp parallel for default(none) shared(fullTileArea) reduction(+:volume)
	for( int i = 0; i < NUMNODES*NUMNODES; i++ )
	{
		int iu = i % NUMNODES;
		int iv = i / NUMNODES;
		float z = Height( iu, iv );
		if( (iu != 0 ) &&( iu != (NUMNODES-1) ) && (iv !=0) && (iv !=(NUMNODES-1)) ){ //full
			volume+=z*fullTileArea*2.;
		}
		else if( (iu == 0 || iu == (NUMNODES-1) ) && (iv == 0 || iv == (NUMNODES-1) ) ){ //corner
			volume+= z*(fullTileArea/4.)*2.;
		}
		else{	//edge tile
			
			volume+=z*(fullTileArea/2.)*2.;
		}
	}
	double time1 = omp_get_wtime();

	double megaMults = (double)pow(NUMNODES, 2.) / (time1 - time0) / 1000000;
	
	
	
	printf("%f", volume);
	//printf("%f", megaMults);

}


float
Height( int iu, int iv )	// iu,iv = 0 .. NUMNODES-1
{
	float x = -1.  +  2.*(float)iu /(float)(NUMNODES-1);	// -1. to +1.
	float y = -1.  +  2.*(float)iv /(float)(NUMNODES-1);	// -1. to +1.

	float xn = pow( fabs(x), (double)N );
	float yn = pow( fabs(y), (double)N );
	float r = 1. - xn - yn;
	if( r < 0. )
	        return 0.;
	float height = pow( 1. - xn - yn, 1./(float)N );
	return height;
}

void
TimeOfDaySeed()
{
	struct tm y2k = { 0 };
	y2k.tm_hour = 0;   y2k.tm_min = 0; y2k.tm_sec = 0;
	y2k.tm_year = 100; y2k.tm_mon = 0; y2k.tm_mday = 1;

	time_t  timer;
	time(&timer);
	double seconds = difftime(timer, mktime(&y2k));
	unsigned int seed = (unsigned int)(1000.*seconds);    // milliseconds
	srand(seed);
}