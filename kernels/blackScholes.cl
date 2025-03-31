
struct pDstruct;

__kernel void blackScholes(__global pDstruct data){
   int tid = get_local_id(0);
   dVdt_vec[tid] = V_vec[tid]/dt;
}
