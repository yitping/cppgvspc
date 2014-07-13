//
//  gvspcV2PM.cpp
//  cppgvspc
//
//  Created by Yitping Kok on 7/11/14.
//  Copyright (c) 2014 MPE. All rights reserved.
//

#include <iostream>
#include <fstream>
#include "gvspcV2PM.h"

gvspcV2PM::gvspcV2PM()
{
	std::cout << "1 V2PM created" << std::endl;
}

gvspcV2PM::gvspcV2PM(const gvspcV2PM& B)
{
	copy(B);
	std::cout << "1 V2PM copied" << std::endl;
}

gvspcV2PM::gvspcV2PM(std::vector<gvspcPix>& phot,
										 double sum_phot[],
										 std::vector<int> t[],
										 double a[])
{
	set(phot, sum_phot, t, a);
	std::cout << "1 V2PM (full) created" << std::endl;
}

gvspcV2PM::gvspcV2PM(const std::vector<std::vector<double> >& BM)
{
	set(BM);
	std::cout << "1 V2PM (full) created" << std::endl;
}

gvspcV2PM::~gvspcV2PM()
{
	std::cout << "1 V2PM destroyed" << std::endl;
}

int gvspcV2PM::set(std::vector<gvspcPix>& phot,
									 double sum_phot[],
									 std::vector<int> t[],
									 double a[])
{
	int i, j, k, l;
	int n_bl = phot[0].num_bl();
	int n_ph = phot[0].num_ph();
	
	int n_tel = phot.size();
	int n_row = n_ph*n_bl;
	int n_col = n_tel+2*n_bl;

	double cos_ps_l, sin_ps_l, vis_exp;
	
	std::cout << "n_ph = " << n_ph << ", n_bl = " << n_bl << std::endl;
	std::cout << "n_tel = " << n_tel << std::endl;
	M.resize(n_row);
	for (i=0; i<n_row; i++) M[i].resize(n_col,0);
	e.resize(n_row,0); // use zero as default for now
	
	std::cout << "n_row = " << M.size() << ", n_col = " << M[1].size() << std::endl;
	
	std::cout << "computing V2PM (phot)..." << std::endl;
	for (j=0; j<n_tel; j++) std::cout << "sum[" << j << "] = " << sum_phot[j] << std::endl;
	
	for (i=0; i<n_bl; i++) for (k=0; k<n_ph; k++)
	{
		l = i*n_ph+k;
		for (j=0; j<2; j++)
		{
			M[l][t[j][i]-1] = phot[t[j][i]-1][l]/sum_phot[t[j][i]-1];
		}
	}
	
	std::cout << "computing V2PM (intf)..." << std::endl;
	
	for (i=0; i<n_bl; i++) for (k=0; k<n_ph; k++)
	{
		l = i*n_ph+k;
		cos_ps_l = cosd(a[l]);
		sin_ps_l = sind(a[l]);
		std::cout << (((i==0) && (k==0)) ? "" : ",") << cos_ps_l;
		vis_exp  = 2*sqrt(phot[t[0][i]-1][l]*phot[t[1][i]-1][l]);
		vis_exp /= sum_phot[t[0][i]-1]+sum_phot[t[1][i]-1];
		M[l][n_tel+0*n_bl+i] =  vis_exp*cos_ps_l;
		M[l][n_tel+1*n_bl+i] = -vis_exp*sin_ps_l;
	}
	std::cout << std::endl;
	std::cout << "pi/2 = " << M_PI_2 << std::endl;
	
	std::cout << "done..." << std::endl;
	
	return 0;
}

int gvspcV2PM::set(const std::vector<std::vector<double> >& BM)
{
	M = BM;
	return 0;
}

gvspcV2PM& gvspcV2PM::operator=(const gvspcV2PM& B)
{
	copy(B);
	return *this;
}

int gvspcV2PM::nrow() { return M.size(); }
int gvspcV2PM::ncol() { return M[0].size(); }

int gvspcV2PM::save_to_file(const char *fname, const char *label, int append)
{
	std::ofstream csv(fname, (append == 1) ? std::ios::app : std::ios::out);
	csv << "# " << label << ", " << M.size() << ", " << M[0].size() << std::endl;
	for (int i=0; i<M.size(); i++)
	{
		for (int j=0; j<M[i].size(); j++) csv << ((j == 0) ? "" : ",") << M[i][j];
		csv << std::endl;
	}
	csv.close();
	return 0;
}


///// private /////

void gvspcV2PM::copy(const gvspcV2PM& B)
{
	M = B.M;
	e = B.e;
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


