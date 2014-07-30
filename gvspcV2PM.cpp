//
//  gvspcV2PM.cpp
//  cppgvspc
//
//  Created by Yitping Kok on 7/11/14.
//  Copyright (c) 2014 MPE. All rights reserved.
//

#include "gvspcV2PM.h"

gvspcV2PM::gvspcV2PM()
{
}

gvspcV2PM::gvspcV2PM(const gvspcV2PM& B)
{
	copy(B);
}

gvspcV2PM::gvspcV2PM(const std::vector<gvspcPix>& phot,
										 double sum_phot[],
										 const std::vector<int> t[],
										 double a[])
{
	set(phot, sum_phot, t, a);
}

gvspcV2PM::gvspcV2PM(const std::vector<std::vector<double> >& BM)
{
	set(BM);
}

gvspcV2PM::~gvspcV2PM()
{
}

int gvspcV2PM::set(const std::vector<gvspcPix>& phot,
									 double sum_phot[],
									 const std::vector<int> t[],
									 double a[])
{
	int i, j, k, l;
	int n_bl  = phot[0].num_bl();
	int n_ph  = phot[0].num_ph();
	int n_tel = phot.size();
	int n_row = n_ph*n_bl;
	int n_col = n_tel+2*n_bl;

	double cos_ps_l, sin_ps_l, vis_exp;
	
	M.resize(n_row);
	vis.resize(n_col);
	for (i=0; i<n_row; i++) M[i].resize(n_col,0);
	
	for (i=0; i<n_bl; i++) for (k=0; k<n_ph; k++)
	{
		l = i*n_ph+k;
		for (j=0; j<2; j++)
		{
			M[l][t[j][i]] = phot[t[j][i]][l]/sum_phot[t[j][i]];
		}
	}
	
	for (i=0; i<n_bl; i++) for (k=0; k<n_ph; k++)
	{
		l = i*n_ph+k;
		cos_ps_l = cosd(a[l]);
		sin_ps_l = sind(a[l]);
		vis_exp  = 2*sqrt(phot[t[0][i]][l]*phot[t[1][i]][l]);
		vis_exp /= sum_phot[t[0][i]]+sum_phot[t[1][i]];
		M[l][n_tel+0*n_bl+i] =  vis_exp*cos_ps_l;
		M[l][n_tel+1*n_bl+i] = -vis_exp*sin_ps_l;
	}
	
	return 0;
}

int gvspcV2PM::set(const std::vector<std::vector<double> >& BM)
{
	M = BM;
	vis.resize(M[0].size());
	return 0;
}

const std::vector<std::vector<double> >& gvspcV2PM::data() const
{
	return M;
}

gvspcV2PM& gvspcV2PM::operator=(const gvspcV2PM& B)
{
	copy(B);
	return *this;
}

int gvspcV2PM::nrow() { return M.size(); }
int gvspcV2PM::ncol() { return M[0].size(); }


///// private /////

void gvspcV2PM::copy(const gvspcV2PM& B)
{
	M = B.M;
	vis = B.vis;
}

double gvspcV2PM::cosd(double d)
{
	if ((d - (int) d) != 0.0)
		return cos(d*M_PI/180);
	else
		return (!((int) d%360)) ? 1.0 : ((!((int) d%180)) ? -1.0 : ((!((int) d%90)) ? 0 : cos(d*M_PI/180)));
}

double gvspcV2PM::sind(double d)
{
	if ((d - (int) d) != 0.0)
		return sin(d*M_PI/180);
	else
		return (!((int) d%180)) ? 0.0 : sin(d*M_PI/180);
}


