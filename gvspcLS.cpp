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
	std::cout << "1 gvspcLS created" << std::endl;
}

gvspcLS::gvspcLS(const std::vector<std::vector<double> >& A, const std::vector<double>& b)
{
	init();
	set(A, b);
	std::cout << "1 gvspcLS (full) created" << std::endl;
}

gvspcLS::gvspcLS(int m, int n)
{
	init();
	nrow = m; ncol = n;
	A = gsl_matrix_alloc(nrow, ncol);
	b = gsl_vector_alloc(nrow);
	std::cout << "1 gvspcLS (full) created" << std::endl;
}

gvspcLS::~gvspcLS()
{
	clear();
	std::cout << "1 gvspcLS destroyed" << std::endl;
}

void gvspcLS::setA(int i, int j, double aij) { gsl_matrix_set(A, i, j, aij); }
void gvspcLS::setb(int i, double bi) { gsl_vector_set(b, i, bi); }

const gvspcLS& gvspcLS::set(const std::vector<std::vector<double> >& A, const std::vector<double>& b)
{
	if (A.size() != b.size())
	{
		std::cerr << "A and b size mismatch" << std::endl;
		return *this;
	}
	
	clear(); // just to be sure
	
	nrow = A.size();
	ncol = A[0].size();
	
	this->A = gsl_matrix_alloc(nrow, ncol);
	this->b = gsl_vector_alloc(nrow);
	for (int i=0; i<nrow; i++)
	{
		gsl_vector_set(this->b, i, b[i]);
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
		return xx;
	}
	
	gsl_matrix *V = gsl_matrix_alloc(ncol, ncol);
	gsl_vector *s = gsl_vector_alloc(ncol);
	gsl_vector *w = gsl_vector_alloc(ncol);
	gsl_vector *x = gsl_vector_alloc(ncol);
	
	gsl_linalg_SV_decomp(A, V, s, w);
	for (int i=0; i<ncol; i++)
		if ((gsl_vector_get(s,i) < NUMERIC_SINGULAR_VALUE) && (gsl_vector_get(s,i) > -NUMERIC_SINGULAR_VALUE))
			gsl_vector_set(s,i,0);
	gsl_linalg_SV_solve(A, V, s, b, x);
	
	xx.resize(ncol);
	for (int j=0; j<ncol; j++) xx[j] = gsl_vector_get(x, j);
	
	gsl_vector_free(x);
	gsl_vector_free(w);
	gsl_vector_free(s);
	gsl_matrix_free(V);
	
	return xx;
}

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
		{
			std::cout << " " << gsl_vector_get(b, i) << std::endl;
		}
	}
	else
	{
		std::cout << "[null]" << std::endl;
	}
}

///// private /////

void gvspcLS::init()
{
	A = NULL; x = b = NULL;
	nrow = ncol = 0;
}

void gvspcLS::clear()
{
	if (A != NULL) gsl_matrix_free(A);
	if (x != NULL) gsl_vector_free(x);
	if (b != NULL) gsl_vector_free(b);
}