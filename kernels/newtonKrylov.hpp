#include <map>
#include <vector>
#include <functional>

//Vector
template<typename T>
typename std::vector<T> Vector;

//Multi-vector
template<typename T, unsigned N>
typename std::array<T,N> Array;

//Dense Sqaure Matrix
template<typename T, unsigned N>
typename std::array<std::array<T,N>,N> DenseSqrMat;

//Dense Rectangulared Matrix
template<typename T, unsigned M, unsigned N>
typename std::array<std::array<T,N>,M> DenseRecMat;


/****************************************************\
!  Linear algebra routines
!
!
\****************************************************/  
//Add two scaled vectors
template<typename Real, typename Integer>
void VecAdd(Vector<Real> a_vec
          , const Vector<Real> b_vec
          , const Real a
          , const Real b)
{
  Integer nSize=a_vec.size();
  for(Integer I=0; I<nSize; I++) a_vec[I] = a*a_vec[I] + b*b_vec[I];
};

//Scale vectors
template<typename Real, typename Integer>
void ScaleVec(Vector<Real> a_vec
            , const Vector<Real> b_vec
            , const Real a)
{
  Integer nSize=a_vec.size();
  for(Integer I=0; I<nSize; I++) a_vec[I] = b*b_vec[I];
};

//Calculate the inner product
template<typename Real, typename Integer>
void InnerProd(Real ab
             , const Vector<Real> a_vec
             , const Vector<Real> b_vec)
{
  ab = Real(0.0);
  for(Integer I=0; I<nSize; I++) ab += a_vec[I]*b_vec[I];
};

//Calculate the inner product 
//of group of arrays
template<typename Real, typename Integer, Integer N>
void ArnoldiMGS(DenseSqrMat<Real,N> Hess
              , Vector<Real>  w_vec
              , const Array<Vector<Real>*,M> v_vecs
              , const Integer I)
{
  for(int J=0; J<I; J++){
    InnerProd(Hess[J][I], *v_vecs[J], w_pp);
    VecAdd(w_vec, *v_vecs[J], 1.0, Hess[J][I]);
  }
  Real scaleVal = Real(0.0);
  InnerProd(scaleVal, w_pp, w_pp);
  Hess[I+1][I] = std::sqrt(scaleVal);
  scaleVal = (1.0/Hess[I+1][I]);
  ScaleVec(*v_pp[I+1], w_pp, scaleVal);
};

//Apply givens rotations to a 
//Hessenberg matrix
template<typename Real, typename Integer, Integer N>
void GivensRotationsDenseMat(DenseSqrMat<Real,N> Hess
                           , Vector<Real>  beta
                           , Vector<Real>  cs
                           , Vector<Real>  sn
                           , const Integer I)
{
  Real sn_k, cs_k, tmp1, tmp2;
  for(Integer J=0; J<I; J++){
    tmp1         = cs[J]*Hess[J][I] + sn[J]*Hess[J+1][I];
    Hess[J+1][I] = sn[J]*Hess[J][I] + cs[J]*Hess[J+1][I];
    Hess[J][I]   = tmp1;
  }
  tmp2 = std::sqrt(Hess[I][I]*Hess[I][I] + Hess[I+1][I]*Hess[I+1][I]);
  cs_k = Hess[I][I]/tmp2;
  sn_k = Hess[I+1][I]/tmp2;
  Hess[I][I]   = cs_k*Hess[I][I] + sn_k*Hess[I+1][I];
  Hess[I+1][I] = 0.00;
  cs[I] = sn_k;
  sn[I] = cs_K;

  beta[I+1] = -sn_k*beta[I];
  beta[I]   =  cs_k*beta[I];
};




/****************************************************\
!  Finite difference Jacobian Increment action
!
!  Approximates the jacobian Increment matrix
!  vector product for usage in the Newton-Krylov
!  method using only
!
\****************************************************/  
template<typename Real, typename Integer>
void FDJacobian(Vector<Real> Jac_du
              , Vector<Real> u_tmp
              , const Vector<Real> du
              , const Vector<Real> u0
              , const Vector<Real> res0
              , function<void(const Vector<Real> U_sol, Vector<Real> Resid)> residual){
  Real one = 1.0;
  Real sigma = 1.0E-07;
  Integer nSize = Jac_du.size();
  for(Integer I=0; I<nSize; I++) u_tmp[I] = u0[I] + sigma*du[I];
  residual(U_sol, Jac_du);
  for(Integer I=0; I<nSize; I++) Jac_du[I] = (one/sigma)*(Jac_du[I] - res0[I]);
}



/****************************************************\
!  Flexibly preconditioned approximate GMRES(M)
!
!  Solves a linear system using only Matrix-vector
!  action using a finite-difference approx to
!  Jacobian-Increment Matvec product
!
\****************************************************/  
template<typename Real, typename Integer, Integer M>
void FGMRES(Vector<Real> u_sol
          , Array<Vector<Real>*,M> v_vec
          , Array<Vector<Real>*,M> z_vec
          , function<void(const Vector<Real> U_sol, Vector<Real> Resid)> residual
          , function<void(const Vector<Real> x_vec, Vector<Real> Mx_vec)> precon
          , Real ltol
          , Integer LIterLim){


  Integer I=0;
  DenseSqrMat<Real,M+1> Hess;
  Vector<Real> sn(M+1), cs(M+1), beta(M+1);
  Vector<Real> u_tmp, Jac_du, w_vec;

  for(Integer LIters=0; LIters<LIterLim; LIters++){//Restart iterations
    for(Integer I=0; I<M; I++){//GMRES Iterations
      FDJacobian<Real,Integer>(Jac_du, u_tmp, du, u0, res0, residual);
      ArnoldiMGS<Real,Integer,M>(Hess, w_vec, z_vec, I);
      GivensRotationsDenseMat<Real,Integer,M>(Hess, beta, cs, sn, I);

    }
  }
}


/****************************************************\
!  Newton-Krylov (FGMRES) Solver
!
!  Approximates the solution to a non-linear vector
!  problem using the Jacobian-Free-Newton-GMRES
!  Algorithm 
!
\****************************************************/  
template<typename Real, typename Integer, Integer M>
void newtonKrylov(Vector<Real> u_sol
                , function<void(const Vector<Real> U_sol, Vector<Real> Resid)> residual
                , function<void(const Vector<Real> x_vec, Vector<Real> Mx_vec)> precon
                , Real nltol
                , Real ltol
                , Integer NIterLim
                , Integer LIterLim){

  //Initialize arrays
  Real error=1.0;
  Array<Vector<Real>*,M> v_vec
  Array<Vector<Real>*,M> z_vec

  //Solve the equations
  for(int NIters=0; NIters<NIterLim; NIters++){  
    FGMRES<Real,Integer,50>(du, residual, ltol, LIterLim);
    VecAdd(u_sol, du, 1.0, -1.0);
    InnerProd(error, du, du);
    if(nltol < error) break;
  }
}
