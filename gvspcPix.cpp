//
//  gvspcPix.cpp
//  cppgvspc
//
//  Created by Yitping Kok on 7/11/14.
//  Copyright (c) 2014 MPE. All rights reserved.
//

#include "gvspcPix.h"

gvspcPix::gvspcPix() : n_ph(MAX_PHASE_SHIFTS), n_bl(NUM_BASELINES)
{
	init(0,0);
}

gvspcPix::gvspcPix(int n_ch, int n_pl) : n_ph(MAX_PHASE_SHIFTS), n_bl(NUM_BASELINES)
{
	init(n_ch, n_pl);
	resize(n_ch, n_pl);
}

gvspcPix::gvspcPix(const gvspcPix& B) : n_ph(MAX_PHASE_SHIFTS), n_bl(NUM_BASELINES)
{
	copy(B);
}

gvspcPix::gvspcPix(const gvspcPix& B, int j, int p) : n_ph(MAX_PHASE_SHIFTS), n_bl(NUM_BASELINES)
{
	copy(B, j, p);
}


gvspcPix::~gvspcPix()
{
}

void gvspcPix::resize(int n_ch, int n_pl)
{
	this->n_ch = n_ch;
	this->n_pl = n_pl;
	v.resize(n_ph*n_bl*n_ch*n_pl, 0);
	y.resize(n_ch*n_pl, 0);
}

int gvspcPix::num_ph() const { return n_ph; }
int gvspcPix::num_bl() const { return n_bl; }
int gvspcPix::num_ch() const { return n_ch; }
int gvspcPix::num_pl() const { return n_pl; }
long gvspcPix::size()  const { return v.size(); }
int gvspcPix::isNull() const { return ((n_ch == 0) || (n_pl == 0)) ? 1 : 0; }

int gvspcPix::set(cpl_image *img, int *idx, int cnr_ch, int cnr_io)
{
  int i, j, k, l, p;
  int nwd=3, nio=NUM_IO_OUTPUT, cen, isBad, numBadPixels=0;
  double val, sum;

  if ((idx == NULL) || (isNull()))
  {
		std::cerr << "cannot proceed, object is not properly initialized." << std::endl;
    return 1;
  }

  /* the format of idx is determined from the txt file it was read in */
  /* number of channel x number of IO output x number of polarization states */
  for (i=0; i<nio; i++) for (j=0; j<n_ch; j++) for (p=0; p<n_pl; p++)
	{
		/* get the center of slits */
		cen = idx[p*n_ch*nio + i*n_ch + j] + cnr_io;
		sum = 0.0;
		for (k=0; k<nwd; k++)
		{
			l = cen + k - nwd/2;
			val = cpl_image_get(img, j+1+cnr_ch, l, &isBad);
			if (isBad != 0)
				numBadPixels++;
			else
				sum += val;
		} /* k */
		if (i != 11)
			v[convert_img_indices(i, j, p, 0)] = sum;
		else
			y[convert_img_indices(i, j, p, 1)] = sum;
	}

  return 0;
}

int gvspcPix::set(double val, int k, int i, int j, int p)
{
	// k - phase, i - baseline, j - spectral channel, p - polarization
	v[convert_4D_indices(k,i,j,p)] = val;
	return 0;
}

gvspcPix gvspcPix::abs()
{
	gvspcPix abs_this = *this;
	for (int l=0; l<abs_this.v.size(); l++) if (abs_this.v[l] < 0) abs_this.v[l] = -abs_this.v[l];
	for (int l=0; l<abs_this.y.size(); l++) if (abs_this.y[l] < 0) abs_this.y[l] = -abs_this.y[l];
	return abs_this;
}

double gvspcPix::get(int k, int i, int j, int p)
{
	return v[convert_4D_indices(k,i,j,p)];
}

double gvspcPix::get(long i) { return v[i]; }

double gvspcPix::sum(int p)
{
	double sum=0.0;
	int begin_ix, end_ix, i;
	
	begin_ix = (p == 0) ? 0 : convert_4D_indices(0,0,0,1);
	end_ix   = (p == 0) ? convert_4D_indices(0,0,0,1) : v.size();
	for (i=begin_ix; i<end_ix; i++) sum += v[i];
	
	begin_ix = (p == 0) ? 0 : convert_4D_indices(0,0,0,1,1);
	end_ix   = (p == 0) ? convert_4D_indices(0,0,0,1,1) : y.size();
	for (i=begin_ix; i<end_ix; i++) sum += y[i];
	
	return sum;
}

gvspcPix& gvspcPix::operator=(const gvspcPix& B)
{
	copy(B);
	return *this;
}

double gvspcPix::operator[](long i) { return v[i]; }
double gvspcPix::operator[](long i) const { return v[i]; }

gvspcPix gvspcPix::operator+(const gvspcPix& B) const
{
	gvspcPix C;
	if ((n_ch != B.n_ch) || (n_pl != B.n_pl))
		std::cerr << "size mismatch" << std::endl;
	else
	{
		C.resize(n_ch, n_pl);
		for (long i=0; i<v.size(); i++) C.v[i] = v[i] + B.v[i];
		for (long i=0; i<y.size(); i++) C.y[i] = y[i] + B.y[i];
	}
	return C;
}

gvspcPix gvspcPix::operator-(const gvspcPix& B) const
{
	gvspcPix C;
	if ((n_ch != B.n_ch) || (n_pl != B.n_pl))
		std::cerr << "size mismatch" << std::endl;
	else
	{
		C.resize(n_ch, n_pl);
		for (long i=0; i<v.size(); i++) C.v[i] = v[i] - B.v[i];
		for (long i=0; i<y.size(); i++) C.y[i] = y[i] - B.y[i];
	}
	return C;
}

gvspcPix gvspcPix::operator*(double b) const
{
	gvspcPix C(n_ch, n_pl);
	for (long i=0; i<v.size(); i++) C.v[i] = b*v[i];
	for (long i=0; i<y.size(); i++) C.y[i] = b*y[i];
	return C;
}

gvspcPix gvspcPix::operator*(const gvspcPix& B) const
{
	gvspcPix C;
	if ((n_ch != B.n_ch) || (n_pl != B.n_pl))
		std::cerr << "size mismatch" << std::endl;
	else
	{
		C.resize(n_ch, n_pl);
		for (long i=0; i<v.size(); i++) C.v[i] = v[i] * B.v[i];
		for (long i=0; i<y.size(); i++) C.y[i] = y[i] * B.y[i];
	}
	return C;
}

gvspcPix& gvspcPix::operator+=(double b)
{
	for (long i=0; i<v.size(); i++) v[i] += b;
	for (long i=0; i<y.size(); i++) y[i] += b;
	return *this;
}

gvspcPix& gvspcPix::operator+=(const gvspcPix& B)
{
	if ((n_ch != B.n_ch) || (n_pl != B.n_pl))
		std::cerr << "size mismatch" << std::endl;
	else
	{
		for (long i=0; i<v.size(); i++) v[i] += B.v[i];
		for (long i=0; i<y.size(); i++) y[i] += B.y[i];
	}
	return *this;
}

gvspcPix& gvspcPix::operator*=(double b)
{
	for (long i=0; i<v.size(); i++) v[i] *= b;
	for (long i=0; i<y.size(); i++) y[i] *= b;
	return *this;
}

gvspcPix& gvspcPix::operator*=(const gvspcPix& B)
{
	if ((n_ch != B.n_ch) || (n_pl != B.n_pl))
		std::cerr << "size mismatch" << std::endl;
	else
	{
		for (long i=0; i<v.size(); i++) v[i] *= B.v[i];
		for (long i=0; i<y.size(); i++) y[i] *= B.y[i];
	}
	return *this;
}

gvspcPix gvspcPix::operator/(double b) const
{
	gvspcPix C(n_ch, n_pl);
	for (long i=0; i<v.size(); i++) C.v[i] = v[i]/b;
	for (long i=0; i<y.size(); i++) C.y[i] = y[i]/b;
	return C;
}

gvspcPix gvspcPix::operator/(const gvspcPix& B) const
{
	gvspcPix C;
	if ((n_ch != B.n_ch) || (n_pl != B.n_pl))
		std::cerr << "size mismatch" << std::endl;
	else
	{
		C.resize(n_ch, n_pl);
		for (long i=0; i<v.size(); i++) C.v[i] = v[i]/B.v[i];
		for (long i=0; i<y.size(); i++) C.y[i] = y[i]/B.y[i];
	}
	return C;
}

gvspcPix& gvspcPix::operator/=(double b)
{
	for (long i=0; i<v.size(); i++) v[i] /= b;
	for (long i=0; i<y.size(); i++) y[i] /= b;
	return *this;
}

gvspcPix& gvspcPix::operator/=(const gvspcPix& B)
{
	if ((n_ch != B.n_ch) || (n_pl != B.n_pl))
		std::cerr << "size mismatch" << std::endl;
	else
	{
		for (long i=0; i<v.size(); i++) v[i] /= B.v[i];
		for (long i=0; i<y.size(); i++) y[i] /= B.y[i];
	}
	return *this;
}

const std::vector<double>& gvspcPix::data()
{
	return v;
}

const std::vector<double>& gvspcPix::data(int j, int p)
{
	v_jp.assign(v.begin()+convert_4D_indices(0,0,j,p,0),
							v.begin()+convert_4D_indices(n_ph-1,n_bl-1,j,p,0)+1);
	return v_jp;
}


///// private /////

void gvspcPix::init(int n_ch, int n_pl)
{
	this->n_ch = n_ch;
	this->n_pl = n_pl;
	if (!v.empty()) v.clear();
	if (!y.empty()) y.clear();
}

void gvspcPix::copy(const gvspcPix& B)
{
	if (this == &B) return; // avoid self assignment
	init(B.n_ch, B.n_pl);
	v = B.v;
	y = B.y;
}

// TODO: this needs to be improved!
void gvspcPix::copy(const gvspcPix& B, int j, int p)
{
	if (this == &B) return; // avoid self assignment
	init(1,1);
	resize(1,1);
	if ((j >= B.num_ch()) || (p >= B.num_pl())) return;
	long l0 = p*B.num_ch()*n_bl*n_ph + j*n_bl*n_ph;
	long l1 = p*B.num_ch()+j;
	for (int l=0; l<n_ph*n_bl; l++)
	{
		v[l] = B.v[l0+l];
	}
	y[0]= B.y[l1];
}

long gvspcPix::convert_img_indices(int l, int j, int p, int isYjunc)
{
	// l - io output, j - spectral channel, p - polarization
	int i, k; // k - phase, i - baseline
	if (isYjunc == 1) // at the Y-junction
	{
		return p*n_ch + j;
	}
	else
	{
		i = (l < 11) ? l/4 : ((l > 11) ? (l+1)/4 : -1); // translate l-th IO to i-th baseline
		k = (l < 11) ? l%4 : ((l > 11) ? (l+1)%4 : -1); // translate l-th IO to k-th phase shift
		k = (k == 1) ? 2 : ((k == 2) ? 1 : k); // swap phase C and B
		return p*n_ch*n_bl*n_ph + j*n_bl*n_ph + i*n_ph + k;
	}
}

long gvspcPix::convert_4D_indices(int k, int i, int j, int p, int isYjunc)
{
	// k - phase, i - baseline, j - spectral channel, p - polarization
	return (isYjunc == 0) ? (p*n_ch*n_bl*n_ph + j*n_bl*n_ph + i*n_ph + k) : (p*n_ch + j);
}







