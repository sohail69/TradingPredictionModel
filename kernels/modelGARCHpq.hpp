/****************************************************\
!  Extra templated maths functions
!
!  Basic maths functions used for calculations
!  of different things
!
\****************************************************/  

// Exponential function
float MyExp(float x){ return exp(x); };


// SQRT function
float MySqrt(float x){ return sqrt(x); };


// Normal distribution function
float NormalDist(float Val, float Mean, float Var){
  float Pi = 3.14159265359;
  N = 1.0/MySqrt(2.0*Pi*Var);
  N = N*MyExp( -(Val-Mean)*(Val-Mean)/(2.0*Var) );
  return N;
}

// Iterators for I and J
unsigned iteratorI(unsigned l, unsigned P){return l/P;};

unsigned iteratorJ(unsigned l, unsigned P){
  unsigned M = l/P;
  return (l - M*P);
};

/****************************************************\
!  RNG calculations
!
!  RNG used for random sampling
!
\****************************************************/  
unsigned int _pcg6432_uint(pcg6432_state* state){
  unsigned long int oldstate = *state;
  *state = oldstate * 6364136223846793005UL + 0xda3e39cb94b95bdbUL;
	uint xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
	uint rot = oldstate >> 59u;
	return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}
#define pcg6432_double(state) (pcg6432_ulong(state)*PCG6432_DOUBLE_MULTI)


/****************************************************\
!  GARCH calculations
!
!  Calculates the GARCH coefficients
!
\****************************************************/  
template<int p, int q>
void GARCH_calcResiduals(const float *alphat_betat
                       , float *residual){

  //Calculate the energy functional
  float Pi = sigmat02 - omega;
  for(unsigned I=0; I<p; I++)
    Pi = Pi - alphat[I]*sigmat2[I];

  for(unsigned I=0; I<q; I++)
    Pi = Pi - betat[I]*epsilont2[I];

  //Calculate the residuals
  for(unsigned I=0; I<p; I++)
    residual[I] = 2.0*Pi*sigmat2[I];

  for(unsigned I=0; I<q; I++)
    residual[I+p] = 2.0*Pi*epsilont2[I];
};

