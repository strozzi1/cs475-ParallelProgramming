#include <stdio.h>
#include <xmmintrin.h>
#include <omp.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#define SSE_WIDTH		4

#ifndef NUMT
#define NUMT 1
#endif

#ifndef ARRSIZE
#define ARRSIZE	516000
#endif

#define NUMTRIES 10

float SimdMulSum( float *, float *, int );
float nonSIMDMulSum( float *, float *, int );

float		Ranf(float, float);
int		Ranf(int, int);
void		TimeOfDaySeed();

const float RMIN = 0.0;
const float RMAX = 100.0;

int main (int argc, char *argv[ ] ){
	int CHUNK_SIZE = ARRSIZE / NUMT;
	float sum = 0.;
	//float a[ARRSIZE];
	//float b[ARRSIZE];
	float *a = new float [ARRSIZE];
	float *b = new float [ARRSIZE];
	
	TimeOfDaySeed();

	for(int i=0; i<ARRSIZE; i++){
		a[i] = Ranf(RMIN, RMAX);
		b[i] = Ranf(RMIN, RMAX);
	}
	
	
	float maxMMPS = 0.;
	for(int tries=0; tries<NUMTRIES; tries++){
		double time0 = omp_get_wtime();


		sum = SimdMulSum(a, b, ARRSIZE);
		//sum = nonSIMDMulSum(a, b, ARRSIZE);


		double time1 =omp_get_wtime();

		double megaMultsPerSecond = (double)ARRSIZE / (time1 - time0) / 1000000.;
		if(megaMultsPerSecond > maxMMPS)
			maxMMPS = megaMultsPerSecond;

		
	}

	/*for(int i=0; i<ARRSIZE; i++){
		a[i] = Ranf(RMIN, RMAX);
		b[i] = Ranf(RMIN, RMAX);
	}*/

	float sum2;
	float maxMMPS2 = 0.;
	for(int tries=0; tries<NUMTRIES; tries++){
		double time0 = omp_get_wtime();		
		sum2 = nonSIMDMulSum(a, b, ARRSIZE);
		double time1 =omp_get_wtime();

		double megaMultsPerSecond = (double)ARRSIZE / (time1 - time0) / 1000000.;
		if(megaMultsPerSecond > maxMMPS2)
			maxMMPS2 = megaMultsPerSecond;

		
	}

	//PRINTS: ARRAYSIZE, SIMDspeed, nonSIMDspeed, SIMDsum, nonSIMDsum
	printf("%d, %f, %f, %f, %f\n", ARRSIZE, maxMMPS, maxMMPS2, sum, sum2);
    return 0;
}



float
SimdMulSum( float *a, float *b, int len )
{
	float sum[4] = { 0., 0., 0., 0. };
	int limit = ( len/SSE_WIDTH ) * SSE_WIDTH;
	register float *pa = a;
	register float *pb = b;

	__m128 ss = _mm_loadu_ps( &sum[0] );
	for( int i = 0; i < limit; i += SSE_WIDTH )
	{
		ss = _mm_add_ps( ss, _mm_mul_ps( _mm_loadu_ps( pa ), _mm_loadu_ps( pb ) ) );
		pa += SSE_WIDTH;
		pb += SSE_WIDTH;
	}
	_mm_storeu_ps( &sum[0], ss );

	for( int i = limit; i < len; i++ )
	{
		sum[0] += a[i] * b[i];
	}

	return sum[0] + sum[1] + sum[2] + sum[3];
}

float
nonSIMDMulSum(float *a, float *b, int len)
{
	float sum = 0.;

	for (int i = 0; i < len; i++){
		sum += a[i] * b[i];
	}
	return sum;
}







float Ranf(float low, float high)
{
		float r = (float)rand();               // 0 - RAND_MAX
		float t = r / (float)RAND_MAX;       // 0. - 1.

		return   low + t * (high - low);
}

int
Ranf(int ilow, int ihigh)
{
	float low = (float)ilow;
	float high = ceil((float)ihigh);

	return (int)Ranf(low, high);
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