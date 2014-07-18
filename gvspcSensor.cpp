//
//  gvspcSensor.cpp
//  cppgvspc
//
//  Created by Yitping Kok on 7/11/14.
//  Copyright (c) 2014 MPE. All rights reserved.
//

#include "gvspcSensor.h"

// TODO: check for memory leaks
// TODO: use covariance matrix to solve v2pm

gvspcSensor::gvspcSensor() : n_tel(NUM_TELESCOPES), n_ph(MAX_PHASE_SHIFTS), n_bl(NUM_BASELINES)
{
	init();
}


gvspcSensor::~gvspcSensor()
{
}


int gvspcSensor::has_indices() { return indices_ready; }
int gvspcSensor::has_ps() { return ps_ready; }
int gvspcSensor::has_v2pms() { return v2pms_ready; }

int gvspcSensor::num_baselines() const { return n_bl; }
int gvspcSensor::num_polstates() const { return n_pl; }
int gvspcSensor::num_telescopes() const { return n_tel; }
int gvspcSensor::num_scichannel() const { return n_ch; }


int gvspcSensor::load_pixel_indices(char *filename)
{
  int csv_n_ch, csv_n_sl, csv_n_pl, csv_corner_ch, csv_corner_io;
	std::vector<std::vector<int> > temp_int;
	std::vector<std::vector<double> > temp_dbl;

	gvspcCsv csv(filename);
	
  // get the corner coordinate
  if (csv.read_as_int((char *) "corner", temp_int) == -1)
    return GVSPC_ERROR_INPUT_BAD;
  csv_corner_ch = temp_int[0][0];
  csv_corner_io = temp_int[0][1];
  cpl_msg_debug(cpl_func, "corner_ch = %d", csv_corner_ch);
  cpl_msg_debug(cpl_func, "corner_io = %d", csv_corner_io);

  // get the image size
  if (csv.read_as_int((char *) "cropsz", temp_int) == -1)
    return GVSPC_ERROR_INPUT_BAD;
  csv_n_ch = temp_int[0][0];
  csv_n_sl = temp_int[0][1];
  cpl_msg_debug(cpl_func, "n_ch = %d", csv_n_ch);
  cpl_msg_debug(cpl_func, "n_sl = %d", csv_n_sl);

  // check consistency
  if ((csv_n_sl % NUM_IO_OUTPUT) != 0)
  {
    cpl_msg_error(cpl_func, "unexpected number of polarization states.");
    return GVSPC_ERROR_MODE_UNKNOWN;
  }

  csv_n_pl = csv_n_sl/NUM_IO_OUTPUT;
  cpl_msg_debug(cpl_func, "n_pl = %d", csv_n_pl);

  // check consistency
  switch (csv_n_pl)
  {
    case 1:
    case 2:
      n_ch = csv_n_ch;
      n_pl = csv_n_pl;
			corner_ch = csv_corner_ch;
			corner_io = csv_corner_io;
      break;
    default:
      cpl_msg_error(cpl_func, "unexpected number of polarization states.");
      return GVSPC_ERROR_MODE_UNKNOWN;
  }

  // update the corner coordinate if 1st spectral channel is not at index 1
  if (csv.read_as_int((char *) "s_pix", temp_int) == -1)
    return GVSPC_ERROR_INPUT_BAD;
  corner_ch += temp_int[0][0];
  cpl_msg_debug(cpl_func, "corner_ch = %d", corner_ch);

  if (csv.read_as_dbl((char *) "s_um", temp_dbl) == -1)
    return GVSPC_ERROR_INPUT_BAD;
  bw  = 0.1591549*temp_dbl[0][0]*temp_dbl[0][n_ch-1]/(temp_dbl[0][n_ch-1]-temp_dbl[0][0]);
  bw *= 0.001; /* mm per radian */
  cpl_msg_debug(cpl_func, "bw (mm per radian) = %.6f", bw);
  
  // finally read it
	pixel_index.clear();
  if (csv.read_as_int((char *) "idx", temp_int) == -1)
  {
    cpl_msg_error(cpl_func, "unexpected error, freeing up unused memory.");
    return GVSPC_ERROR_FILE_IO;
  }
	for (int i=0; i<temp_int.size(); i++) for (int j=0; j<temp_int[i].size(); j++)
		pixel_index.push_back(temp_int[i][j]);
	cpl_msg_debug(cpl_func, "index = %d, %d, ..., %d (%d)",
								pixel_index[0], pixel_index[1],
								pixel_index.back(), (int) pixel_index.size());
	
	indices_ready = true;
	reinit();

  return GVSPC_ERROR_NONE;
}


int gvspcSensor::process_image(cpl_image *image, int type)
{
	if (image == NULL) return GVSPC_ERROR_INPUT_BAD;

	gvspcPix new_pix(n_ch, n_pl);
	if (new_pix.set(image, &pixel_index[0], corner_ch, corner_io) != 0)
	{
		std::cout << "a gvspcPix should be destroyed after this." << std::endl;
		return GVSPC_ERROR_SUBR_FAILED;
	}
	
	cpl_msg_debug(cpl_func, "img2Flux successful..");
	switch (type)
	{
		case 0:
			cpl_msg_debug(cpl_func, "adding instantaneous dark into fifo..");
			fifo_pix_dark.add(new_pix);
			break;
		case 1:
		default:
			fifo_pix_flux.add(new_pix-mean_dark);
			cpl_msg_debug(cpl_func, "adding instantaneous flux into fifo..");
	}
	
	return GVSPC_ERROR_NONE;
}


int gvspcSensor::save_dark()
{
	if (fifo_pix_dark.size() <= 2) return GVSPC_ERROR_INPUT_BAD;
	mean_dark = fifo_pix_dark.mean();
	var_dark  = fifo_pix_dark.var();
	fifo_pix_dark.clear();
	return GVSPC_ERROR_NONE;
}


int gvspcSensor::save_phot(int tel)
{
	if (fifo_pix_flux.size() <= 2) return GVSPC_ERROR_INPUT_BAD;
	mean_phot[tel-1] = fifo_pix_flux.mean();//-mean_dark;
	fifo_pix_flux.clear();
	return GVSPC_ERROR_NONE;
}


int gvspcSensor::set_default_ps()
{
	double default_ps[] = {0, 90, 180, 270, 0, 90, 180, 270, 0, 90, 180, 270, 0, 90, 180, 270, 0, 90, 180, 270, 0, 90, 180, 270};
	ps.assign(std::begin(default_ps), std::end(default_ps));
	cpl_msg_info(cpl_func, "using %d default phase shifts", (int) ps.size());
	ps_ready = true;
	return GVSPC_ERROR_NONE;
}

int gvspcSensor::set_ps(const std::vector<double>& ps)
{
	if (ps.size() == n_ph*n_bl) this->ps = ps;
	else
	{
		cpl_msg_info(cpl_func, "warning: phase shifts length mismatched");
		set_default_ps();
	}
	ps_ready = true;
	return GVSPC_ERROR_NONE;
}


int gvspcSensor::compute_v2pms()
{
	std::vector<double> sum(n_tel);
	std::vector<gvspcPix> subpix(n_tel);
	int t, p, j, l;
	
	for (p=0; p<n_pl; p++)
	{
		for (t=0; t<n_tel; t++) sum[t] = mean_phot[t].sum(p)/n_ch;
		for (j=0; j<n_ch; j++)
		{
			l = p*n_ch + j;
			// TODO: this is not efficient
			for (t=0; t<n_tel; t++) subpix[t] = gvspcPix(mean_phot[t],j,p);
			v2pms[l].set(subpix, sum.data(), tels, ps.data());
		}
	}
	
	return GVSPC_ERROR_NONE;
}

int gvspcSensor::load_v2pms(const char *filename)
{
	gvspcCsv csv(filename);
	
	if (!csv.has_linked_file())
	{
		std::cerr << "v2pm file not loaded" << std::endl;
		return GVSPC_ERROR_FILE_IO;
	}
	
	int n_var;
	
	if ((n_var = csv.has_variables()) == 0)
	{
		std::cerr << "v2pm file has no data" << std::endl;
		return GVSPC_ERROR_INPUT_BAD;
	}
	
	if (n_var != n_ch*n_pl)
	{
		std::cerr << "number of v2pms in file mismatched" << std::endl;
		return GVSPC_ERROR_INPUT_BAD;
	}
	
	std::cout << "loading v2pms" << std::endl;
	
	if (v2pms.size() != n_var) v2pms.resize(n_var);
	
	std::vector<std::vector<double> > v2pm;
	for (int i=0; i<n_var; i++)
	{
		csv.read_as_dbl(i, v2pm);
		v2pms[i].set(v2pm);
	}
	
	return GVSPC_ERROR_NONE;
}

int gvspcSensor::save_v2pms(const char *filename)
{
	if (v2pms.empty())
	{
		std::cerr << "no v2pms to write" << std::endl;
		return GVSPC_ERROR_INPUT_BAD;
	}
	
	int p, i, j, l;
	int m = v2pms[0].nrow();
	int n = v2pms[0].ncol();
	
	std::vector<std::vector<double> > data;
	data.resize(m);
	for (i=0; i<m; i++) data[i].resize(n);
	
	std::string label;
	gvspcCsv csv(filename, 1);
	std::cout << "writing v2pms to file" << std::endl;
	for (p=0; p<n_pl; p++) for (j=0; j<n_ch; j++)
	{
		l = p*n_ch + j;
		label = "v2pm" + std::to_string(j) + "_p" + std::to_string(p);
		csv.write(label.c_str(), v2pms[l].data());
	}
	return GVSPC_ERROR_NONE;
}

int gvspcSensor::compute_opd(int p, int novar)
{
	compute_fv(p, novar);
	compute_gd();
	return 0;
}

int gvspcSensor::compute_fv(int p, int novar)
{
	int i, j, t;
	gvspcPix last = fifo_pix_flux.last();
	
	std::vector<double> coh;
	std::vector<double> var_coh;
	double x, y, f, e_v, e_f;
	
	for (j=0; j<n_ch; j++)
	{
		if (novar == 0)
		{
			gvspcPix var  =  last.abs() + var_dark;
//			if (fifo_pix_flux.size() > 2) var += fifo_pix_flux.var();
//			if (fifo_pix_flux.size() > 2) var += last;
			gvspcLS p2vm(v2pms[p*n_ch+j].data(), last.data(j,p), var.data(j,p));
//			p2vm.info();
			coh = p2vm.solve();
			var_coh = p2vm.get_var();
		}
		else
		{
			gvspcLS p2vm(v2pms[p*n_ch+j].data(), last.data(j,p));
			coh = p2vm.solve();
			var_coh = p2vm.get_var();
		}
		for (t=0; t<n_tel; t++)
		{
			flux[t] = (j == 0) ? coh[t] : (flux[t] + coh[t]);
			var_flux[t] = (j == 0) ? var_coh[t] : (var_flux[t] + var_coh[t]);
		}
		for (i=0; i<n_bl; i++)
		{
			f = coh[tels[0][i]] + coh[tels[1][i]];
			x = coh[n_tel+0*n_bl+i];
			y = coh[n_tel+1*n_bl+i];
			v_cos_pd[i][j] = x/f;
			v_sin_pd[i][j] = y/f;
			e_f  = var_coh[tels[0][i]] + var_coh[tels[1][i]];
			e_f /= f*f;
			e_v  = var_coh[n_tel+0*n_bl+i];
			e_v /= x*x;
			var_v_cos_pd[i][j]  = v_cos_pd[i][j]*v_cos_pd[i][j];
			var_v_cos_pd[i][j] *= e_v + e_f;
			e_v  = var_coh[n_tel+1*n_bl+i];
			e_v /= y*y;
			var_v_sin_pd[i][j]  = v_sin_pd[i][j]*v_sin_pd[i][j];
			var_v_sin_pd[i][j] *= e_v + e_f;
		}
	}
	return 0;
}

int gvspcSensor::compute_gd()
{
	int i, j;
	std::complex<double> d, m;
	std::vector<std::complex<double> > dd(n_ch-1);
	double e, s;
	for (i=0; i<n_bl; i++)
	{
		// compute average of phasor difference
		for (j=0; j<(n_ch-1); j++)
		{
			d  = std::complex<double>(v_cos_pd[i][j],    v_sin_pd[i][j]);
			d *= std::complex<double>(v_cos_pd[i][j+1], -v_sin_pd[i][j+1]);
			m  = (j == 0) ? d : m+d;
			dd[j] = d;
		}
		gd[i] = arg(m)*(n_ch-1);
		// compute the deviation of
		m = conj(m);
		for (j=0; j<(n_ch-1); j++)
		{
			e  = arg(dd[j]*m);
			e *= e;
			s  = (j == 0) ? e : s+e;
		}
		var_gd[i] = s*(n_ch-1)/(n_ch-2);
	}
	return 0;
}

int gvspcSensor::compute_opl()
{
	int i, j;
	double aij;
//	double min_step=0.0001; // in mm
	gvspcLS opd2opl(n_bl, n_tel);
	
	for (i=0; i<n_bl; i++)
	{
		opd2opl.setb(i, gd[i]);
		for (j=0; j<n_tel; j++)
		{
			aij = (j == tels[0][i]) ? bl_sign[i]*1 : ((j == tels[1][i]) ? -bl_sign[i]*1 : 0);
			opd2opl.setA(i, j, aij);
		}
	}
	
	opl = opd2opl.solve();
	
	return 0;
}

int gvspcSensor::get_tel(int i, int t) const { return tels[t][bl_arr[i]]+1; }
double gvspcSensor::get_flux(int t) const { return flux[t]; }
double gvspcSensor::get_opl(int t) const { return opl[t]; }
double gvspcSensor::get_gd(int i) const
{
	return bl_sign[bl_arr[i]]*gd[bl_arr[i]];
}
double gvspcSensor::get_pd(int i, int j) const
{
	return atan2(bl_sign[bl_arr[i]]*v_sin_pd[bl_arr[i]][j], v_cos_pd[bl_arr[i]][j]);
}
double gvspcSensor::get_v2(int i, int j) const
{
	return v_cos_pd[bl_arr[i]][j]*v_cos_pd[bl_arr[i]][j] + v_sin_pd[bl_arr[i]][j]*v_sin_pd[bl_arr[i]][j];
}
double gvspcSensor::get_cp(int c, int j) const
{
	int i, ii;
	double s;
	std::complex<double> cp;
	for (i=0; i<3; i++)
	{
		// ctri refers to the conventional baseline arrangment
		ii = (ctri[i][c] < 0) ? bl_arr[-ctri[i][c]] : bl_arr[ctri[i][c]];
		s  = (ctri[i][c] < 0) ? -1.0 : 1.0;
		s *= bl_sign[ii];
		cp = (i == 0) ?
			 std::complex<double>(v_cos_pd[ii][j], s*v_sin_pd[ii][j]) :
		cp*std::complex<double>(v_cos_pd[ii][j], s*v_sin_pd[ii][j]);
	}
	
	return std::arg(cp);
}

double gvspcSensor::get_var_flux(int t) const { return var_flux[t]; }
double gvspcSensor::get_var_gd(int i) const { return var_gd[bl_arr[i]]; }
double gvspcSensor::get_var_pd(int i, int j) const
{
	double x = v_cos_pd[bl_arr[i]][j];
	double y = v_sin_pd[bl_arr[i]][j];
	double e;
	e  = x*x*var_v_cos_pd[bl_arr[i]][j];
	e += y*y*var_v_sin_pd[bl_arr[i]][j];
	e *= (x*x + y*y)*(x*x + y*y);
	return e;
}
double gvspcSensor::get_var_v2(int i, int j) const
{
	double e;
	e  = 4*v_cos_pd[bl_arr[i]][j]*v_cos_pd[bl_arr[i]][j]*var_v_cos_pd[bl_arr[i]][j];
	e += 4*v_sin_pd[bl_arr[i]][j]*v_sin_pd[bl_arr[i]][j]*var_v_sin_pd[bl_arr[i]][j];
	return e;
}


///// private /////

void gvspcSensor::init()
{
	n_ch = 0;
	n_pl = 0;
	corner_ch = 0;
	corner_io = 0;
	bw = 0;
	
	indices_ready = false;
	ps_ready = false;
	v2pms_ready = false;
	
	fifo_pix_flux.limit(5);
	fifo_pix_dark.limit(5);
	mean_phot.resize(n_tel);
	
	// Initializations below are suitable for a child class
	// e.g. gvspcSCISensor, gvspcFTSensor, etc.
	
	// index to telescopes (conventional arrangment, i.e. 1, 2, ...)
	// that form baselines
	int ti[] = {2, 0, 0, 1, 1, 0};
	int tj[] = {3, 3, 2, 3, 2, 1};
	tels[0].assign(std::begin(ti), std::end(ti));
	tels[1].assign(std::begin(tj), std::end(tj));
	
	int blsign[] = {1, 1,-1, 1, 1,-1};
	bl_sign.assign(std::begin(blsign), std::end(blsign));
	
	// this maps the internal baseline arrangment to the
	// conventional arrangment, i.e. 1-2, 1-3, 1-4, ...
	// this should only be use when returning baseline related
	// variables to an external calling function
	int blarr[] = {5, 2, 1, 4, 3, 0};
	bl_arr.assign(std::begin(blarr), std::end(blarr));
	
	// index to baselines (conventional arrangement) that form
	// closed triangles
	int bli[] = { 0, 0, 3, 1};
	int blj[] = {-1,-2,-4,-2};
	int blk[] = { 3, 4, 5, 5};
	ctri[0].assign(std::begin(bli), std::end(bli));
	ctri[1].assign(std::begin(blj), std::end(blj));
	ctri[2].assign(std::begin(blk), std::end(blk));
	
}

void gvspcSensor::reinit()
{
	mean_dark.resize(n_ch, n_pl);
	var_dark.resize(n_ch, n_pl);
	v2pms.resize(n_ch*n_pl);
	flux.resize(n_tel); var_flux.resize(n_tel);
	v_cos_pd.resize(n_bl); var_v_cos_pd.resize(n_bl);
	v_sin_pd.resize(n_bl); var_v_sin_pd.resize(n_bl);
	for (int i=0; i<n_bl; i++)
	{
		v_cos_pd[i].resize(n_ch); var_v_cos_pd[i].resize(n_ch);
		v_sin_pd[i].resize(n_ch); var_v_sin_pd[i].resize(n_ch);
	}
	gd.resize(n_bl); var_gd.resize(n_bl);
	opl.resize(n_tel);
}


