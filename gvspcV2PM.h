//
//  gvspcV2PM.h
//  cppgvspc
//
//  Created by Yitping Kok on 7/11/14.
//  Copyright (c) 2014 MPE. All rights reserved.
//

#ifndef cppgvspc_gvspcV2PM_h
#define cppgvspc_gvspcV2PM_h

#include <vector>
#include "gvspcConst.h"
#include "gvspcPix.h"

class gvspcV2PM
{
	std::vector<std::vector<double> > M;
	std::vector<double> e;
	
public:
	gvspcV2PM();
	gvspcV2PM(const gvspcV2PM& B);
	// this can be inefficient because not all elements in this huge gvspcPix are used
	gvspcV2PM(std::vector<gvspcPix>& phot,
						double sum_phot[],
						std::vector<int> t[],
						double a[]);
	gvspcV2PM(const std::vector<std::vector<double> >& BM);
	~gvspcV2PM();
	
	int set(std::vector<gvspcPix>& phot,
					double sum_phot[],
					std::vector<int> t[],
					double a[]);
	int set(const std::vector<std::vector<double> >& BM);
	const std::vector<std::vector<double> >& get() const;
	
	gvspcV2PM& operator=(const gvspcV2PM& B);
	
	int nrow();
	int ncol();
	
	int save_to_file(const char *csv, const char *label, int append);
	
private:
	void init();
	void copy(const gvspcV2PM& B);
	double cosd(double d);
	double sind(double d);
	
};

#endif
