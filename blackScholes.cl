

__kernel void blackScholes(__global const float *V_vec
                         , __global const float dt
                         , __global const uint neq
                         , __global float *dVdt_vec){

   int tid = get_local_id(0);
   dVdt_vec[tid] = V_vec[tid]/dt;
}
