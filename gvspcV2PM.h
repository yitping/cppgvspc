//
//  gvspcV2PM.h
//  cppgvspc
//
//  Created by Yitping Kok on 7/11/14.
//  Copyright (c) 2014 MPE. All rights reserved.
//

#ifndef cppgvspc_gvspcV2PM_h
#define cppgvspc_gvspcV2PM_h

#include <iostream>
#include <fstream>
#include <vector>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_cblas.h>
#include <gsl/gsl_blas.h>
#include "gvspcPix.h"

class gvspcV2PM
{
	std::vector<std::vector<double> > M;
	std::vector<double> vis;
	
public:
	gvspcV2PM();
	gvspcV2PM(const gvspcV2PM& B);
	// this can be inefficient because not all elements in this huge gvspcPix are used
	gvspcV2PM(const std::vector<gvspcPix>& phot,
						double sum_phot[],
						const std::vector<int> t[],
						double a[]);
	gvspcV2PM(const std::vector<std::vector<double> >& BM);
	~gvspcV2PM();
	
	int set(const std::vector<gvspcPix>& phot,
					double sum_phot[],
					const std::vector<int> t[],
					double a[]);
	int set(const std::vector<std::vector<double> >& BM);
	const std::vector<std::vector<double> >& get() const;
	
	gvspcV2PM& operator=(const gvspcV2PM& B);
	
	int nrow();
	int ncol();
	
	const std::vector<double>& solve(const std::vector<double>& pix);
	const std::vector<double>& solve(const std::vector<double>& pix, const std::vector<double>& var);
	
private:
	void init();
	void copy(const gvspcV2PM& B);
	double cosd(double d);
	double sind(double d);
	
};

#endif
