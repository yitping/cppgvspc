//
//  gvspcLS.h
//  cppgvspc
//
//  Created by Yitping Kok on 7/15/14.
//  Copyright (c) 2014 MPE. All rights reserved.
//

#ifndef cppgvspc_gvspcLS_h
#define cppgvspc_gvspcLS_h

#include <iostream>
#include <vector>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_cblas.h>
#include <gsl/gsl_blas.h>

#define NUMERIC_SINGULAR_VALUE 1e-6

class gvspcLS
{
	gsl_matrix *A;
	gsl_vector *x;
	gsl_vector *b;
	std::vector<double> xx;
	int nrow;
	int ncol;
	
public:
	gvspcLS();
	gvspcLS(const std::vector<std::vector<double> >& A, const std::vector<double>& b);
	gvspcLS(int m, int n);
	~gvspcLS();
	
	void setA(int i, int j, double aij);
	void setb(int i, double bi);
	const gvspcLS& set(const std::vector<std::vector<double> >& A, const std::vector<double>& b);
	const gvspcLS& set(const std::vector<std::vector<double> >& A, const std::vector<double>& b, const std::vector<double>& w);
	const std::vector<double>& solve();
	
	void info();
	
private:
	void init();
	void clear();
};

#endif
