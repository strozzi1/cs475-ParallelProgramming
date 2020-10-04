kernel
void
ArrayMult( global const float *dA, global const float *dB, global float *dC, global float * dD)
{
	int gid = get_global_id( 0 );

	dD[gid] = (dA[gid] * dB[gid]) + dC[gid];
}

kernel void
ArrayMultReduce( global const float *dA, global const float *dB, local float *prods, global float *dC )
{
	int gid = get_global_id( 0 ); // 0 .. total_array_size-1
	int numItems = get_local_size( 0 ); // # work-items per work-group
	int tnum = get_local_id( 0 ); // thread (i.e., work-item) number in this work-group
	// 0 .. numItems-1
	int wgNum = get_group_id( 0 ); // which work-group number this is in
	prods[ tnum ] = dA[ gid ] * dB[ gid ]; // multiply the two arrays together
	// now add them up – come up with one sum per work-group
	// it is a big performance benefit to do it here while “prods” is still available – and is local
	// it would be a performance hit to pass “prods” back to the host then bring it back to the device for reduction
}