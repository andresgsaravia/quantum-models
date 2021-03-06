#ifndef EIGEN_SPARSE_QR_H
#define EIGEN_SPARSE_QR_H
// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2012-2013 Desire Nuentsa <desire.nuentsa_wakam@inria.fr>
// Copyright (C) 2012-2013 Gael Guennebaud <gael.guennebaud@inria.fr>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.


namespace Eigen {

template<typename MatrixType, typename OrderingType> class SparseQR;
template<typename SparseQRType> struct SparseQRMatrixQReturnType;
template<typename SparseQRType> struct SparseQRMatrixQTransposeReturnType;
template<typename SparseQRType, typename Derived> struct SparseQR_QProduct;
namespace internal {
  template <typename SparseQRType> struct traits<SparseQRMatrixQReturnType<SparseQRType> >
  {
    typedef typename SparseQRType::MatrixType ReturnType;
  };
  template <typename SparseQRType> struct traits<SparseQRMatrixQTransposeReturnType<SparseQRType> >
  {
    typedef typename SparseQRType::MatrixType ReturnType;
  };
  template <typename SparseQRType, typename Derived> struct traits<SparseQR_QProduct<SparseQRType, Derived> >
  {
    typedef typename Derived::PlainObject ReturnType;
  };
} // End namespace internal

/**
  * \ingroup SparseQR_Module
  * \class SparseQR
  * \brief Sparse left-looking rank-revealing QR factorization
  * 
  * This class is used to perform a left-looking  rank-revealing QR decomposition 
  * of sparse matrices. When a column has a norm less than a given tolerance
  * it is implicitly permuted to the end. The QR factorization thus obtained is 
  * given by A*P = Q*R where R is upper triangular or trapezoidal. 
  * 
  * P is the column permutation which is the product of the fill-reducing and the
  * rank-revealing permutations. Use colsPermutation() to get it.
  * 
  * Q is the orthogonal matrix represented as Householder reflectors. 
  * Use matrixQ() to get an expression and matrixQ().transpose() to get the transpose.
  * You can then apply it to a vector.
  * 
  * R is the sparse triangular or trapezoidal matrix. This occurs when A is rank-deficient.
  * matrixR().topLeftCorner(rank(), rank()) always returns a triangular factor of full rank.
  * 
  * \tparam _MatrixType The type of the sparse matrix A, must be a column-major SparseMatrix<>
  * \tparam _OrderingType The fill-reducing ordering method. See the \link OrderingMethods_Module 
  *  OrderingMethods \endlink module for the list of built-in and external ordering methods.
  * 
  * 
  */
template<typename _MatrixType, typename _OrderingType>
class SparseQR
{
  public:
    typedef _MatrixType MatrixType;
    typedef _OrderingType OrderingType;
    typedef typename MatrixType::Scalar Scalar;
    typedef typename MatrixType::RealScalar RealScalar;
    typedef typename MatrixType::Index Index;
    typedef SparseMatrix<Scalar,ColMajor,Index> QRMatrixType;
    typedef Matrix<Index, Dynamic, 1> IndexVector;
    typedef Matrix<Scalar, Dynamic, 1> ScalarVector;
    typedef PermutationMatrix<Dynamic, Dynamic, Index> PermutationType;
  public:
    SparseQR () : m_isInitialized(false),m_analysisIsok(false),m_lastError(""),m_useDefaultThreshold(true)
    { }
    
    SparseQR(const MatrixType& mat) : m_isInitialized(false),m_analysisIsok(false),m_lastError(""),m_useDefaultThreshold(true)
    {
      compute(mat);
    }
    void compute(const MatrixType& mat)
    {
      analyzePattern(mat);
      factorize(mat);
    }
    void analyzePattern(const MatrixType& mat);
    void factorize(const MatrixType& mat);
    
    /** \returns the number of rows of the represented matrix. 
      */
    inline Index rows() const { return m_pmat.rows(); }
    
    /** \returns the number of columns of the represented matrix. 
      */
    inline Index cols() const { return m_pmat.cols();}
    
    /** \returns a const reference to the \b sparse upper triangular matrix R of the QR factorization.
      */
    const /*SparseTriangularView<MatrixType, Upper>*/MatrixType matrixR() const { return m_R; }
    /** \returns the number of columns in the R factor 
     * \warning This is not the rank of the matrix. It is provided here only for compatibility 
     */
    Index rank() const 
    {
      eigen_assert(m_isInitialized && "The factorization should be called first, use compute()");
      return m_nonzeropivots; 
    }
    
    /** \returns an expression of the matrix Q as products of sparse Householder reflectors.
      * You can do the following to get an actual SparseMatrix representation of Q:
      * \code
      * SparseMatrix<double> Q = SparseQR<SparseMatrix<double> >(A).matrixQ();
      * \endcode
      */
    SparseQRMatrixQReturnType<SparseQR> matrixQ() const 
    { return SparseQRMatrixQReturnType<SparseQR>(*this); }
    
    /** \returns a const reference to the fill-in reducing permutation that was applied to the columns of A
      */
    const PermutationType colsPermutation() const
    { 
      eigen_assert(m_isInitialized && "Decomposition is not initialized.");
      return m_outputPerm_c;
    }
    
    /**
     * \returns A string describing the type of error
     */
    std::string lastErrorMessage() const
    {
      return m_lastError; 
    }
    /** \internal */
    template<typename Rhs, typename Dest>
    bool _solve(const MatrixBase<Rhs> &B, MatrixBase<Dest> &dest) const
    {
      eigen_assert(m_isInitialized && "The factorization should be called first, use compute()");
      eigen_assert(this->rows() == B.rows() && "SparseQR::solve() : invalid number of rows in the right hand side matrix");

      Index rank = this->rank();
      // Compute Q^T * b;
      Dest y,b;
      y = this->matrixQ().transpose() * B; 
      b = y;
      // Solve with the triangular matrix R
      y.topRows(rank) = this->matrixR().topLeftCorner(rank, rank).template triangularView<Upper>().solve(b.topRows(rank));
      y.bottomRows(y.size()-rank).setZero();

      // Apply the column permutation
      if (m_perm_c.size())  dest.topRows(cols()) =  colsPermutation() * y.topRows(cols());
      else                  dest = y.topRows(cols());
      
      m_info = Success;
      return true;
    }
    
    /** Set the threshold that is used to determine the rank and the null Householder 
     * reflections. Precisely, if the norm of a householder reflection is below this 
     * threshold, the entire column is treated as zero.
     */
    void setPivotThreshold(const RealScalar& threshold)
    {
      m_useDefaultThreshold = false;
      m_threshold = threshold;
    } 
    /** \returns the solution X of \f$ A X = B \f$ using the current decomposition of A.
      *
      * \sa compute()
      */
    template<typename Rhs>
    inline const internal::solve_retval<SparseQR, Rhs> solve(const MatrixBase<Rhs>& B) const 
    {
      eigen_assert(m_isInitialized && "The factorization should be called first, use compute()");
      eigen_assert(this->rows() == B.rows() && "SparseQR::solve() : invalid number of rows in the right hand side matrix");
      return internal::solve_retval<SparseQR, Rhs>(*this, B.derived());
    }
    
    /** \brief Reports whether previous computation was successful.
      *
      * \returns \c Success if computation was succesful,
      *          \c NumericalIssue if the QR factorization reports a numerical problem
      *          \c InvalidInput if the input matrix is invalid
      *
      * \sa iparm()          
      */
    ComputationInfo info() const
    {
      eigen_assert(m_isInitialized && "Decomposition is not initialized.");
      return m_info;
    }
    
  protected:
    bool m_isInitialized;
    bool m_analysisIsok;
    bool m_factorizationIsok;
    mutable ComputationInfo m_info;
    std::string m_lastError;
    QRMatrixType m_pmat;            // Temporary matrix
    QRMatrixType m_R;               // The triangular factor matrix
    QRMatrixType m_Q;               // The orthogonal reflectors
    ScalarVector m_hcoeffs;         // The Householder coefficients
    PermutationType m_perm_c;       //  Fill-reducing  Column  permutation
    PermutationType m_pivotperm;    // The permutation for rank revealing
    PermutationType m_outputPerm_c;       //The final column permutation
    RealScalar m_threshold;         // Threshold to determine null Householder reflections
    bool m_useDefaultThreshold;        // Use default threshold
    Index m_nonzeropivots;             // Number of non zero pivots found 
    IndexVector m_etree;            // Column elimination tree
    IndexVector m_firstRowElt;      // First element in each row
    template <typename, typename > friend struct SparseQR_QProduct;
    
};

/** \brief Preprocessing step of a QR factorization 
  * 
  * In this step, the fill-reducing permutation is computed and applied to the columns of A
  * and the column elimination tree is computed as well. Only the sparcity pattern of \a mat is exploited.
  * \note In this step it is assumed that there is no empty row in the matrix \a mat
  */
template <typename MatrixType, typename OrderingType>
void SparseQR<MatrixType,OrderingType>::analyzePattern(const MatrixType& mat)
{
  // Compute the column fill reducing ordering
  OrderingType ord; 
  ord(mat, m_perm_c); 
  Index n = mat.cols();
  Index m = mat.rows();
  
  if (!m_perm_c.size())
  {
    m_perm_c.resize(n);
    m_perm_c.indices().setLinSpaced(n, 0,n-1);
  }
  
  // Compute the column elimination tree of the permuted matrix
  m_outputPerm_c = m_perm_c.inverse();
  internal::coletree(mat, m_etree, m_firstRowElt, m_outputPerm_c.indices().data());
  
  m_R.resize(n, n);
  m_Q.resize(m, n);
  // Allocate space for nonzero elements : rough estimation
  m_R.reserve(2*mat.nonZeros()); //FIXME Get a more accurate estimation through symbolic factorization with the etree
  m_Q.reserve(2*mat.nonZeros());
  m_hcoeffs.resize(n);
  m_analysisIsok = true;
}

/** \brief Perform the numerical QR factorization of the input matrix
  * 
  * The function SparseQR::analyzePattern(const MatrixType&) must have been called beforehand with
  * a matrix having the same sparcity pattern than \a mat.
  * 
  * \param mat The sparse column-major matrix
  */
template <typename MatrixType, typename OrderingType>
void SparseQR<MatrixType,OrderingType>::factorize(const MatrixType& mat)
{
  eigen_assert(m_analysisIsok && "analyzePattern() should be called before this step");
  Index m = mat.rows();
  Index n = mat.cols();
  IndexVector mark(m); mark.setConstant(-1);  // Record the visited nodes
  IndexVector Ridx(n), Qidx(m);               // Store temporarily the row indexes for the current column of R and Q
  Index nzcolR, nzcolQ;       // Number of nonzero for the current column of R and Q
  ScalarVector tval(m); 
  bool found_diag;
    
  m_pmat = mat;
  m_pmat.uncompress(); // To have the innerNonZeroPtr allocated
  for (int i = 0; i < n; i++)
  {
    Index p = m_perm_c.size() ? m_perm_c.indices()(i) : i;
    m_pmat.outerIndexPtr()[p] = mat.outerIndexPtr()[i]; 
    m_pmat.innerNonZeroPtr()[p] = mat.outerIndexPtr()[i+1] - mat.outerIndexPtr()[i]; 
  }
  
  // Compute the default threshold.
  if(m_useDefaultThreshold) 
  {
    RealScalar infNorm = 0.0;
    for (int j = 0; j < n; j++) 
    {
      //FIXME No support for mat.col(i).maxCoeff())
      for(typename MatrixType::InnerIterator it(m_pmat, j); it; ++it)
        infNorm = (std::max)(infNorm, (std::abs)(it.value()));
    }
    m_threshold = 20 * (m + n) *  infNorm *std::numeric_limits<RealScalar>::epsilon();
  }
  
  m_pivotperm.resize(n);
  m_pivotperm.indices().setLinSpaced(n, 0, n-1); // For rank-revealing
  
  // Left looking rank-revealing QR factorization : Compute a column of R and Q at a time
  Index rank = 0; // Record the number of valid pivots
  for (Index col = 0; col < n; col++)
  {
    mark.setConstant(-1);
    m_R.startVec(col);
    m_Q.startVec(col);
    mark(rank) = col;
    Qidx(0) = rank;
    nzcolR = 0; nzcolQ = 1;
    found_diag = false;  tval.setZero(); 
    // Symbolic factorization : Find the nonzero locations of the column k of the factors R and Q
    // i.e All the nodes (with indexes lower than rank) reachable through the col etree rooted at node k
    for (typename MatrixType::InnerIterator itp(m_pmat, col); itp || !found_diag; ++itp)
    {
      Index curIdx = rank ;
      if (itp) curIdx = itp.row();
      if(curIdx == rank) found_diag = true;
      // Get the nonzeros indexes  of the current column of R
      Index st = m_firstRowElt(curIdx); // The traversal of the etree starts here 
      if (st < 0 )
      {
        m_lastError = " Empty row found during Numerical factorization ";
        m_info = NumericalIssue;
        return;
      }
      // Traverse the etree 
      Index bi = nzcolR;
      for (; mark(st) != col; st = m_etree(st))
      {
        Ridx(nzcolR) = st; // Add this row to the list 
        mark(st) = col; // Mark this row as visited
        nzcolR++;
      }
      // Reverse the list to get the topological ordering
      Index nt = nzcolR-bi;
      for(int i = 0; i < nt/2; i++) std::swap(Ridx(bi+i), Ridx(nzcolR-i-1));
       
      // Copy the current (curIdx,pcol) value of the input mat
      if (itp) tval(curIdx) = itp.value();
      else tval(curIdx) = Scalar(0.);
      
      // Compute the pattern of Q(:,k)
      if (curIdx > rank && mark(curIdx) != col ) 
      {
        Qidx(nzcolQ) = curIdx; // Add this row to the pattern of Q
        mark(curIdx) = col; // And mark it as visited
        nzcolQ++;
      }
    }
    // Browse all the indexes of R(:,col) in reverse order
    for (Index i = nzcolR-1; i >= 0; i--)
    {
      Index curIdx = m_pivotperm.indices()(Ridx(i));
      // Apply the <curIdx> householder vector  to tval
      Scalar tdot(0.);
      //First compute q'*tval
      for (typename QRMatrixType::InnerIterator itq(m_Q, curIdx); itq; ++itq)
      {
        tdot += internal::conj(itq.value()) * tval(itq.row());
      }
      tdot *= m_hcoeffs(curIdx);
      // Then compute tval = tval - q*tau
      for (typename QRMatrixType::InnerIterator itq(m_Q, curIdx); itq; ++itq)
      {
        tval(itq.row()) -= itq.value() * tdot;
      }
      // Detect fill-in for the current column of Q
      if((m_etree(Ridx(i)) == rank) )
      {
        for (typename QRMatrixType::InnerIterator itq(m_Q, curIdx); itq; ++itq)
        {
          Index iQ = itq.row();
          if (mark(iQ) != col)
          {
            Qidx(nzcolQ++) = iQ; // Add this row to the pattern of Q
            mark(iQ) = col; //And mark it as visited
          }
        }
      }
    } // End update current column
        
    // Compute the Householder reflection for the current column
    RealScalar sqrNorm =0.;
    Scalar tau; RealScalar beta;
    Scalar c0 = (nzcolQ) ? tval(Qidx(0)) : Scalar(0.);
    //First, the squared norm of Q((col+1):m, col)
    for (Index itq = 1; itq < nzcolQ; ++itq)
    {
      sqrNorm += internal::abs2(tval(Qidx(itq)));
    }
    
    if(sqrNorm == RealScalar(0) && internal::imag(c0) == RealScalar(0))
    {
      tau = RealScalar(0);
      beta = internal::real(c0);
      tval(Qidx(0)) = 1;
     }
    else
    {
      beta = std::sqrt(internal::abs2(c0) + sqrNorm);
      if(internal::real(c0) >= RealScalar(0))
        beta = -beta;
      tval(Qidx(0)) = 1;
      for (Index itq = 1; itq < nzcolQ; ++itq)
        tval(Qidx(itq)) /= (c0 - beta);
      tau = internal::conj((beta-c0) / beta);
        
    }
    // Insert values in R
    for (Index  i = nzcolR-1; i >= 0; i--)
    {
      Index curIdx = Ridx(i);
      if(curIdx < rank) 
      {
        m_R.insertBackByOuterInnerUnordered(col, curIdx) = tval(curIdx);
        tval(curIdx) = Scalar(0.);
      }
    }
    if(std::abs(beta) >= m_threshold) {
      m_R.insertBackByOuterInner(col, rank) = beta;
      rank++;
      // The householder coefficient
      m_hcoeffs(col) = tau;
      /* Record the householder reflections */
      for (Index itq = 0; itq < nzcolQ; ++itq)
      {
        Index iQ = Qidx(itq);    
        m_Q.insertBackByOuterInnerUnordered(col,iQ) = tval(iQ);
        tval(iQ) = Scalar(0.);
      }    
    } else {
      // Zero pivot found : Move implicitly this column to the end
      m_hcoeffs(col) = Scalar(0);
      for (Index j = rank; j < n-1; j++) 
        std::swap(m_pivotperm.indices()(j), m_pivotperm.indices()[j+1]);
      // Recompute the column elimination tree
      internal::coletree(m_pmat, m_etree, m_firstRowElt, m_pivotperm.indices().data());
    }
  }
  // Finalize the column pointers of the sparse matrices R and Q
  m_Q.finalize(); m_Q.makeCompressed();
  m_R.finalize();m_R.makeCompressed();
  
  m_nonzeropivots = rank;
  
  // Permute the triangular factor to put the 'dead' columns to the end
  MatrixType tempR(m_R);
  m_R = tempR * m_pivotperm;
  
  
  // Compute the inverse permutation
  IndexVector iperm(n);
  for(int i = 0; i < n; i++) iperm(m_perm_c.indices()(i)) = i;
  // Update the column permutation
  m_outputPerm_c.resize(n);
  for (Index j = 0; j < n; j++)  
    m_outputPerm_c.indices()(j) = iperm(m_pivotperm.indices()(j));
  
  m_isInitialized = true; 
  m_factorizationIsok = true;
  m_info = Success;
  
  
}

namespace internal {
  
template<typename _MatrixType, typename OrderingType, typename Rhs>
struct solve_retval<SparseQR<_MatrixType,OrderingType>, Rhs>
  : solve_retval_base<SparseQR<_MatrixType,OrderingType>, Rhs>
{
  typedef SparseQR<_MatrixType,OrderingType> Dec;
  EIGEN_MAKE_SOLVE_HELPERS(Dec,Rhs)

  template<typename Dest> void evalTo(Dest& dst) const
  {
    dec()._solve(rhs(),dst);
  }
};

} // end namespace internal

template <typename SparseQRType, typename Derived>
struct SparseQR_QProduct : ReturnByValue<SparseQR_QProduct<SparseQRType, Derived> >
{
  typedef typename SparseQRType::QRMatrixType MatrixType;
  typedef typename SparseQRType::Scalar Scalar;
  typedef typename SparseQRType::Index Index;
  // Get the references 
  SparseQR_QProduct(const SparseQRType& qr, const Derived& other, bool transpose) : 
  m_qr(qr),m_other(other),m_transpose(transpose) {}
  inline Index rows() const { return m_transpose ? m_qr.rows() : m_qr.cols(); }
  inline Index cols() const { return m_other.cols(); }
  
  // Assign to a vector
  template<typename DesType>
  void evalTo(DesType& res) const
  {
    Index n = m_qr.cols(); 
    if (m_transpose)
    {
      eigen_assert(m_qr.m_Q.rows() == m_other.rows() && "Non conforming object sizes");
      // Compute res = Q' * other :
      res =  m_other;
      for (Index k = 0; k < n; k++)
      {
        Scalar tau = Scalar(0); 
        tau = m_qr.m_Q.col(k).dot(res); 
        tau = tau * m_qr.m_hcoeffs(k);
        for (typename MatrixType::InnerIterator itq(m_qr.m_Q, k); itq; ++itq)
        {
          res(itq.row()) -= itq.value() * tau;
        }
      }
    }
    else
    {
      eigen_assert(m_qr.m_Q.cols() == m_other.rows() && "Non conforming object sizes");
      // Compute res = Q * other :
      res = m_other;
      for (Index k = n-1; k >=0; k--)
      {
        Scalar tau = Scalar(0);
        tau = m_qr.m_Q.col(k).dot(res); 
        tau = tau * m_qr.m_hcoeffs(k);
        res -= tau * m_qr.m_Q.col(k);
      }
    }
  }
  
  const SparseQRType& m_qr;
  const Derived& m_other;
  bool m_transpose;
};

template<typename SparseQRType>
struct SparseQRMatrixQReturnType
{  
  SparseQRMatrixQReturnType(const SparseQRType& qr) : m_qr(qr) {}
  template<typename Derived>
  SparseQR_QProduct<SparseQRType, Derived> operator*(const MatrixBase<Derived>& other)
  {
    return SparseQR_QProduct<SparseQRType,Derived>(m_qr,other.derived(),false);
  }
  SparseQRMatrixQTransposeReturnType<SparseQRType> adjoint() const
  {
    return SparseQRMatrixQTransposeReturnType<SparseQRType>(m_qr);
  }
  // To use for operations with the transpose of Q
  SparseQRMatrixQTransposeReturnType<SparseQRType> transpose() const
  {
    return SparseQRMatrixQTransposeReturnType<SparseQRType>(m_qr);
  }
  const SparseQRType& m_qr;
};

template<typename SparseQRType>
struct SparseQRMatrixQTransposeReturnType
{
  SparseQRMatrixQTransposeReturnType(const SparseQRType& qr) : m_qr(qr) {}
  template<typename Derived>
  SparseQR_QProduct<SparseQRType,Derived> operator*(const MatrixBase<Derived>& other)
  {
    return SparseQR_QProduct<SparseQRType,Derived>(m_qr,other.derived(), true);
  }
  const SparseQRType& m_qr;
};

} // end namespace Eigen

#endif
