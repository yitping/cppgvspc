#include <iostream>
#include <fstream>
#include "gvspcPix.h"

gvspcPix::gvspcPix()
{
	init(0,0);
	std::cout << "1 gvspcPix["<< n_ch << "," << n_pl << "] created!" << std::endl;
}

gvspcPix::gvspcPix(int n_ch, int n_pl)
{
	init(n_ch, n_pl);
	resize(n_ch, n_pl); std::cout << "1 gvspcPix["<< n_ch << "," << n_pl << "] created!" << std::endl;
}

gvspcPix::gvspcPix(const gvspcPix& other_pix)
{
	copy(other_pix);
	std::cout << "1 gvspcPix["<< n_ch << "," << n_pl << "] created!" << std::endl;
}

gvspcPix::gvspcPix(const gvspcPix& other_pix, int j, int p)
{
	copy(other_pix, j, p);
	std::cout << "1 gvspcPix["<< n_ch << "," << n_pl << "] created!" << std::endl;
}


gvspcPix::~gvspcPix() { std::cout << "1 gvspcPix destroyed!" << std::endl; }

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

double gvspcPix::get(int k, int i, int j, int p)
{
	return v[convert_4D_indices(k,i,j,p)];
}

double gvspcPix::get(long i) { return v[i]; }

double gvspcPix::sum()
{
	double sum=0.0;
	for (std::vector<double>::iterator i=v.begin(); i != v.end(); i++) sum += *i;
	for (std::vector<double>::iterator i=y.begin(); i != y.end(); i++) sum += *i;
	return sum;
}

gvspcPix& gvspcPix::operator=(const gvspcPix& other_pix)
{
	copy(other_pix);
	return *this;
}

double gvspcPix::operator[](long i) { return v[i]; }

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

int gvspcPix::save_to_file(std::string& fname, std::string& label, int append)
{
	return save_to_file(fname.c_str(), label.c_str(), append);
}

int gvspcPix::save_to_file(const char *fname, const char *label, int append)
{
	std::ofstream csv(fname, (append == 1) ? std::ios::app : std::ios::out);
	csv << "# " << label << "_v, " << v.size()/n_ph/n_bl << ", " << n_ph*n_bl << std::endl;
	for (int p=0; p<n_pl; p++) for (int j=0; j<n_ch; j++)
	{
		for (int i=0; i<n_bl; i++) for (int k=0; k<n_ph; k++)
			csv << (((i == 0) && (k == 0)) ? "" : ",") << v[convert_4D_indices(k,i,j,p)];
		csv << std::endl;
	}
	csv << "# " << label << "_y, " << y.size() << ", 1" << std::endl;
	for (int p=0; p<n_pl; p++) for (int j=0; j<n_ch; j++)
	{
		csv << y[convert_4D_indices(0,0,j,p,1)] << std::endl;
	}
	csv.close();
	return 0;
}


///// private /////

void gvspcPix::init(int n_ch, int n_pl)
{
	init(MAX_PHASE_SHIFTS, NUM_BASELINES, n_ch, n_pl);
}

void gvspcPix::init(int n_ph, int n_bl, int n_ch, int n_pl)
{
	this->n_ph = n_ph;
	this->n_bl = n_bl;
	this->n_ch = n_ch;
	this->n_pl = n_pl;
	if (!v.empty()) v.clear();
	if (!y.empty()) y.clear();
}

void gvspcPix::copy(const gvspcPix& other_pix)
{
	if (this == &other_pix) return; // avoid self assignment
	init(other_pix.n_ch, other_pix.n_pl);
	v = other_pix.v;
	y = other_pix.y;
}

void gvspcPix::copy(const gvspcPix& other_pix, int j, int p)
{
	if (this == &other_pix) return; // avoid self assignment
	init(1,1);
	if ((j >= other_pix.num_ch()) ||
			(p >= other_pix.num_pl())) return;
	long l0 = convert_4D_indices(0,0,j,p);
	long l1 = convert_4D_indices(0,0,j,p,1);
	for (int l=0; l<n_ph*n_bl; l++)
	{
		v.push_back(other_pix.v[l0+l]);
		y.push_back(other_pix.y[l1]);
	}
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

long gvspcPix::convert_4D_indices(int k, int i, int j, int p, int isYjunc) const
{
	return convert_4D_indices(k,i,j,p,isYjunc);
}

long gvspcPix::convert_4D_indices(int k, int i, int j, int p, int isYjunc)
{
	// k - phase, i - baseline, j - spectral channel, p - polarization
	return (isYjunc == 0) ? (p*n_ch*n_bl*n_ph + j*n_bl*n_ph + i*n_ph + k) : (p*n_ch + j);
}







