//
//  gvspcLS.cpp
//  cppgvspc
//
//  Created by Yitping Kok on 7/15/14.
//  Copyright (c) 2014 MPE. All rights reserved.
//

#include "gvspcLS.h"

gvspcLS::gvspcLS()
{
	init();
}

gvspcLS::gvspcLS(const std::vector<std::vector<double> >& A, const std::vector<double>& b)
{
	init();
	std::vector<double> w;
	set(A, b, w);
}

gvspcLS::gvspcLS(const std::vector<std::vector<double> >& A, const std::vector<double>& b, const std::vector<double>& w)
{
	init();
	set(A, b, w);
}

gvspcLS::gvspcLS(int m, int n)
{
	init();
	nrow = m; ncol = n;
	A = gsl_matrix_alloc(nrow, ncol);
	b = gsl_vector_alloc(nrow);
}

gvspcLS::~gvspcLS()
{
	clear();
}

void gvspcLS::setA(int i, int j, double aij) { gsl_matrix_set(A, i, j, aij); }
void gvspcLS::setb(int i, double bi) { gsl_vector_set(b, i, bi); }

const gvspcLS& gvspcLS::set(const std::vector<std::vector<double> >& A, const std::vector<double>& b, const std::vector<double>& w)
{
	if (A.size() != b.size())
	{
		std::cerr << "A and b sizes mismatch" << std::endl;
		return *this;
	}
	if (!w.empty() && (w.size() != b.size()))
	{
		std::cerr << "w and b sizes mismatch" << std::endl;
		return *this;
	}
	
	clear(); // just to be sure
	
	nrow = A.size();
	ncol = A[0].size();
	
	this->A = gsl_matrix_alloc(nrow, ncol);
	this->b = gsl_vector_alloc(nrow);
	if (!w.empty()) this->W = gsl_matrix_calloc(nrow, nrow);
	for (int i=0; i<nrow; i++)
	{
		gsl_vector_set(this->b, i, b[i]);
		if (!w.empty())
			gsl_matrix_set(this->W, i, i, (w[i] == 0) ? 0 : 1/w[i]);
		for (int j=0; j<ncol; j++)
			gsl_matrix_set(this->A, i, j, A[i][j]);
	}
	
	return *this;
}

const std::vector<double>& gvspcLS::solve()
{
	if ((A == NULL) || (b == NULL))
	{
		std::cerr << "A and/or b are NULL" << std::endl;
		return x;
	}
	
	gsl_matrix *C  = gsl_matrix_alloc(ncol, ncol);
	gsl_matrix *U  = gsl_matrix_alloc(ncol, ncol);
	gsl_matrix *V  = gsl_matrix_alloc(ncol, ncol);
	gsl_vector *s  = gsl_vector_alloc(ncol);
	gsl_vector *a  = gsl_vector_alloc(ncol);
	gsl_vector *r  = gsl_vector_alloc(nrow);
	gsl_vector *Mb = gsl_vector_alloc(ncol);
	gsl_matrix *WA = NULL;
	gsl_vector *Wb = NULL;
	
	if (W != NULL)
	{
//		std::cout << "diag(W) = " << std::endl;
//		for (int i=0; i<nrow; i++)
//			std::cout << " " << ((i == 0) ? "" : ",") << gsl_matrix_get(W,i,i);
//		std::cout << std::endl;
		WA = gsl_matrix_alloc(nrow, ncol);
		Wb = gsl_vector_alloc(nrow);
		// first, compute the inverse covariance matrix of x
		gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, W, A, 0.0, WA);
		gsl_blas_dgemm(CblasTrans, CblasNoTrans, 1.0, A, WA, 0.0, C);
		// then, compute t(A)*W*b -> Mb
		gsl_blas_dgemv(CblasNoTrans, 1.0, W, b, 0.0, Wb);
		gsl_blas_dgemv(CblasTrans, 1.0, A, Wb, 0.0, Mb);
	}
	else
	{
		// first, compute the inverse covariance matrix of x
		gsl_blas_dgemm(CblasTrans, CblasNoTrans, 1.0, A, A, 0.0, C);
		// then, compute t(A)*b -> Mb
		gsl_blas_dgemv(CblasTrans, 1.0, A, b, 0.0, Mb);
	}
	
	// then, decompose C
	gsl_matrix_memcpy(U, C);
	gsl_linalg_SV_decomp(U, V, s, a);

	// apply threshold for singular values
	sv_threshold(s, ncol);
	
	// solve it!
	gsl_linalg_SV_solve(U, V, s, Mb, a);

	// save x
	x.resize(ncol);
	for (int i=0; i<ncol; i++) x[i] = gsl_vector_get(a, i);

	// compute covariance of x by inverting C
	// C <- Cov(x), var_x <- sigma(residual)^2*diag(C)
	sv_inverse(U, V, s, C, ncol, ncol);
	var_x.resize(ncol);
	
	if (W != NULL)
	{
		for (int i=0; i<ncol; i++) var_x[i] = gsl_matrix_get(C, i, i);
	}
	else
	{
		// compute residuals
		gsl_blas_dgemv(CblasNoTrans, 1.0, A, a, 0.0, r);
		std::vector<double> resd(nrow,0);
		for (int i=0; i<nrow; i++)
			resd[i] = gsl_vector_get(r, i) - gsl_vector_get(b, i);
		double var_resd = gsl_stats_variance(resd.data(), 1, nrow);
		for (int i=0; i<ncol; i++) var_x[i] = var_resd*gsl_matrix_get(C, i, i);
	}

	

//	std::cout << "var = " << std::endl;
//	for (int i=0; i<ncol; i++)
//		std::cout << " " << ((i == 0) ? "" : ",") << var_x[i];
//	std::cout << std::endl;
	
	if (W != NULL)
	{
		gsl_vector_free(Wb);
		gsl_matrix_free(WA);
	}
	gsl_vector_free(Mb);
	gsl_vector_free(r);
	gsl_vector_free(a);
	gsl_vector_free(s);
	gsl_matrix_free(V);
	gsl_matrix_free(U);
	gsl_matrix_free(C);
	
	return x;
}

const std::vector<double>& gvspcLS::get_x() { return x; }
const std::vector<double>& gvspcLS::get_var() { return var_x; }

void gvspcLS::info()
{
	int i, j;
	std::cout << "A = " << std::endl;
	if (A != NULL)
	{
		for (i=0; i<nrow; i++)
		{
			for (j=0; j<ncol; j++)
				std::cout << " " << ((j == 0) ? "" : ",") << gsl_matrix_get(A, i, j);
			std::cout << std::endl;
		}
	}
	else
	{
		std::cout << "[null]" << std::endl;
	}
	std::cout << "b = " << std::endl;
	if (b != NULL)
	{
		for (i=0; i<nrow; i++)
			std::cout << ((i==0) ? "" : ",") << gsl_vector_get(b, i);
		std:: cout << std::endl;
	}
	else
	{
		std::cout << "[null]" << std::endl;
	}
	std::cout << "W = " << std::endl;
	if (W != NULL)
	{
		for (i=0; i<nrow; i++)
		{
			double xxx = gsl_matrix_get(W, i, i);
			std::cout << " " << ((i == 0) ? "" : ",") << ((xxx == 0) ? 0 : 1/xxx);
		}
		std::cout << std::endl;
	}
	else
	{
		std::cout << "[null]" << std::endl;
	}
}

///// private /////

void gvspcLS::init()
{
	A = W = NULL; b = NULL;
	nrow = ncol = 0;
}

void gvspcLS::clear()
{
	if (A != NULL) gsl_matrix_free(A);
	if (b != NULL) gsl_vector_free(b);
	if (W != NULL) gsl_matrix_free(W);
}

void gvspcLS::sv_inverse(gsl_matrix *U, gsl_matrix *V, gsl_vector *s, gsl_matrix *Z, int m, int n)
{
	if ((U == NULL) || (V == NULL) || (s == NULL))
	{
		std::cerr << "unexpected null matrices/vectors" << std::endl;
		return;
	}

	double si;
	gsl_matrix *S = gsl_matrix_calloc(n, n);
	gsl_matrix *X = gsl_matrix_alloc(n, n);

	// diag(S) <- s
	for (int i=0; i<n; i++)
	{
		si = gsl_vector_get(s,i);
		gsl_matrix_set(S, i, i, (si == 0) ? 0 : 1/si);
	}
	gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, V, S, 0.0, X);
	gsl_blas_dgemm(CblasNoTrans, CblasTrans, 1.0, X, U, 0.0, Z);
	

	gsl_matrix_free(X);
	gsl_matrix_free(S);
}

void gvspcLS::sv_threshold(gsl_vector *s, int n)
{
	for (int i=0; i<n; i++)
		if ((gsl_vector_get(s,i) < NUMERIC_SINGULAR_VALUE) &&
				(gsl_vector_get(s,i) > -NUMERIC_SINGULAR_VALUE))
			gsl_vector_set(s,i,0);
}
