

__kernel void modelGARCH(__global const float *stockHistory
                       , __global const uint nStocks
                       , __global const uint nHistory
                       , __global float *stockPrediction){

   int tid = get_local_id(0);
}