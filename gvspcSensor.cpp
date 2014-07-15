//
//  gvspcSensor.cpp
//  cppgvspc
//
//  Created by Yitping Kok on 7/11/14.
//  Copyright (c) 2014 MPE. All rights reserved.
//

#include "gvspcSensor.h"

// TODO: check for memory leaks
// TODO: clean up messages

gvspcSensor::gvspcSensor(int m, int n) : n_tel(m), n_ph(n), n_bl(0.5*m*(m-1))
{
	init();
	std::cout << "1 gvspcSensor created!" << std::endl;
}


gvspcSensor::~gvspcSensor()
{
	std::cout << "1 gvspcSensor destroyed!" << std::endl;
}


int gvspcSensor::has_indices() { return indices_ready; }
int gvspcSensor::has_ps() { return ps_ready; }
int gvspcSensor::has_v2pms() { return v2pms_ready; }

int gvspcSensor::num_baselines() { return n_bl; }
int gvspcSensor::num_polstates() { return n_pl; }
int gvspcSensor::num_telescopes() { return n_tel; }
int gvspcSensor::num_scichannel() { return n_ch; }


// TODO: change to gvspcCsv for csv read/write
int gvspcSensor::load_pixel_indices(char *csv)
{
  int headerinfo[2], csv_n_ch, csv_n_sl, csv_n_pl, csv_corner_ch, csv_corner_io;
	std::vector<int> temp_int;
	std::vector<double> temp_dbl;

  /* get the corner coordinate */
  if (csv2Intarr(csv, (char *) "corner", 1, 2, headerinfo) != 0)
    return GVSPC_ERROR_INPUT_BAD;
  csv_corner_ch = headerinfo[0];
  csv_corner_io = headerinfo[1];
  cpl_msg_debug(cpl_func, "corner_ch = %d", csv_corner_ch);
  cpl_msg_debug(cpl_func, "corner_io = %d", csv_corner_io);

  /* get the image size */
  if (csv2Intarr(csv, (char *) "cropsz", 1, 2, headerinfo) != 0)
    return GVSPC_ERROR_INPUT_BAD;
  csv_n_ch = headerinfo[0];
  csv_n_sl = headerinfo[1];
  cpl_msg_debug(cpl_func, "n_ch = %d", csv_n_ch);
  cpl_msg_debug(cpl_func, "n_sl = %d", csv_n_sl);

  /* check consistency */
  if ((csv_n_sl % NUM_IO_OUTPUT) != 0)
  {
    cpl_msg_error(cpl_func, "unexpected number of polarization states.");
    return GVSPC_ERROR_MODE_UNKNOWN;
  }

  csv_n_pl = csv_n_sl/NUM_IO_OUTPUT;
  cpl_msg_debug(cpl_func, "n_pl = %d", csv_n_pl);

  /* check consistency */
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

  /* update the corner coordinate if 1st spectral channel is not at index 1 */
  temp_int.resize(n_ch,0);
  if (csv2Intarr(csv, (char *) "s_pix", 1, n_ch, &temp_int[0]) != 0)
    return GVSPC_ERROR_INPUT_BAD;
  corner_ch += temp_int[0];
  cpl_msg_debug(cpl_func, "corner_ch = %d", corner_ch);

  temp_dbl.resize(n_ch,0);
  if (csv2Dblarr(csv, (char *) "s_um", 1, n_ch, &temp_dbl[0]) != 0)
    return GVSPC_ERROR_INPUT_BAD;
  bw  = 0.1591549*temp_dbl[0]*temp_dbl[n_ch-1]/(temp_dbl[n_ch-1]-temp_dbl[0]);
  bw *= 0.001; /* mm per radian */
  cpl_msg_debug(cpl_func, "bw (mm per radian) = %.6f", bw);
  
  /* allocate memory for the indices and finally read it*/
	pixel_index.resize(NUM_IO_OUTPUT*n_ch*n_pl,0);
  if (csv2Intarr(csv, (char *) "idx", NUM_IO_OUTPUT*n_pl, n_ch, &pixel_index[0]) != 0)
  {
    cpl_msg_error(cpl_func, "unexpected error, freeing up unused memory.");
    pixel_index.clear();
    return GVSPC_ERROR_FILE_IO;
  }
	
	indices_ready = true;
	reinit();

  return GVSPC_ERROR_NONE;
}


int gvspcSensor::process_image(cpl_image *image, int type)
{
	if (image == NULL) return GVSPC_ERROR_INPUT_BAD;
	
	std::cout << "image size = " <<
		cpl_image_get_size_x(image) << " x " <<
		cpl_image_get_size_y(image) << std::endl;

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
			fifo_pix_flux.add(new_pix);//-mean_dark
			cpl_msg_debug(cpl_func, "adding instantaneous flux into fifo..");
	}
	
	std::cout << "process_image(): leaving" << std::endl;
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
	mean_phot[tel-1] = fifo_pix_flux.mean()-mean_dark;
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
	
	std::cout << "size of v2pms = " << v2pms.size() << std::endl;
	
	for (p=0; p<n_pl; p++)
	{
		for (t=0; t<n_tel; t++) sum[t] = mean_phot[t].sum(p)/n_ch;
		for (j=0; j<n_ch; j++)
		{
			l = p*n_ch + j;
			// TODO: this is not efficient
			for (t=0; t<n_tel; t++) subpix[t] = gvspcPix(mean_phot[t],j,p);
			v2pms[l].set(subpix, sum.data(), tels, ps.data());
			std::cout << "abc" << std::endl;
			std::cout << v2pms[l].nrow() << "x" << v2pms[l].ncol() << std::endl;
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
		csv.write(label.c_str(), v2pms[l].get());
	}
	return GVSPC_ERROR_NONE;
}

const std::vector<double>& gvspcSensor::compute_fv(int p)
{
	int i, j, t;
	gvspcPix last = fifo_pix_flux.last() - mean_dark;
	std::vector<double> coh;
	double f;
	std::cout << "prepare to compute cv" << std::endl;
	for (j=0; j<n_ch; j++)
	{
		coh = v2pms[p*n_ch+j].solve(last.data(j,p));
		for (t=0; t<n_tel; t++)
			flux[t] = (j == 0) ? coh[t] : (flux[t] + coh[t]);
		for (i=0; i<n_bl; i++)
		{
			f = coh[tels[0][i]-1] + coh[tels[1][i]-1];
			v_cos_pd[i][j] = coh[n_tel+0*n_bl+i]/f;
			v_sin_pd[i][j] = coh[n_tel+1*n_bl+i]/f;
		}
	}
	return flux;
}

const std::vector<double>& gvspcSensor::compute_gd()
{
	int i, j;
	std::vector<double> re_dd(n_bl,0), im_dd(n_bl,0);
	for (i=0; i<n_bl; i++)
	{
		for (j=0; j<(n_ch-1); j++)
		{
			re_dd[i] +=  v_cos_pd[i][j]*v_cos_pd[i][j+1] + v_sin_pd[i][j]*v_sin_pd[i][j+1];
			im_dd[i] += -v_cos_pd[i][j]*v_sin_pd[i][j+1] + v_sin_pd[i][j]*v_cos_pd[i][j+1];
		}
		gd[i] = atan2(im_dd[i], re_dd[i])*(n_ch-1);
	}
	return gd;
}

const std::vector<double>& gvspcSensor::compute_opl()
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
			aij = (j == tels[0][i]-1) ? sign[i]*1 : ((j == tels[1][i]-1) ? -sign[i]*1 : 0);
			opd2opl.setA(i, j, aij);
		}
	}
	
	opl = opd2opl.solve();
	
	return opl;
}

int gvspcSensor::get_tel(int i, int t) { return tels[t][i]; }
double gvspcSensor::get_flux(int t) { return flux[t]; }
double gvspcSensor::get_opl(int t) { return opl[t]; }
double gvspcSensor::get_gd(int i) { return gd[i]; }
double gvspcSensor::get_pd(int i, int j)
{
	return atan2(v_sin_pd[i][j], v_cos_pd[i][j]);
}
double gvspcSensor::get_v2(int i, int j)
{
	return v_cos_pd[i][j]*v_cos_pd[i][j] + v_sin_pd[i][j]*v_sin_pd[i][j];
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
	
	int ti[] = {3, 1, 1, 2, 2, 1};
	int tj[] = {4, 4, 3, 4, 3, 2};
	int tlsign[] = {1, 1,-1, 1, 1,-1};
	tels[0].assign(std::begin(ti), std::end(ti));
	tels[1].assign(std::begin(tj), std::end(tj));
	sign.assign(std::begin(tlsign), std::end(tlsign));
	
	fifo_pix_flux.limit(MIN_FIFOSIZE);
	fifo_pix_dark.limit(MIN_FIFOSIZE);
	mean_phot.resize(n_tel);
	
}

void gvspcSensor::reinit()
{
	std::cout << "reinit()" << std::endl;
	mean_dark.resize(n_ch, n_pl);
	var_dark.resize(n_ch, n_pl);
	v2pms.resize(n_ch*n_pl);
	flux.resize(n_tel);
	v_cos_pd.resize(n_bl);
	v_sin_pd.resize(n_bl);
	for (int i=0; i<n_bl; i++)
	{
		v_cos_pd[i].resize(n_ch);
		v_sin_pd[i].resize(n_ch);
	}
	gd.resize(n_bl);
	opl.resize(n_tel);
}


/////////////

void gvspcSensor::dump_a_pix()
{
	std::cout << "Size of dark fifo = " << fifo_pix_dark.size() << std::endl;
	if (fifo_pix_dark.size() != 0)
		std::cout << "Size of latest pix in fifo = " << fifo_pix_dark.last().size() << std::endl;
	std::cout << "Size of flux fifo = " << fifo_pix_flux.size() << std::endl;
	if (fifo_pix_flux.size() != 0)
	{
		std::cout << "Size of latest pix in fifo = " << fifo_pix_flux.last().size() << std::endl;
		gvspcPix a = fifo_pix_flux.last();
//		std::vector<double> pix_alle = a.data();
//		std::vector<double> pix_sub0 = a.data(0,0);
//		std::vector<double> pix_sub1 = a.data(2,0);
//		std::cout << "Size of data() in latest pix = " << a.data().size() << std::endl;
//		std::cout << "Size of data(j,p) in latest pix = " << a.data(0,0).size() << std::endl;
		
		std::cout << "abc" << std::endl;
		gvspcCsv sample("abc.csv", 1);
		sample.write((char *) "all", a.data(), 100, 24);
		sample.write((char *) "j0p0", a.data(0,0), 1, 24);
		sample.write((char *) "j2p0", a.data(2,0), 1, 24);
	}
}


void gvspcSensor::dump_an_index()
{
	std::cout << "Pixel index size = " << pixel_index.size() << std::endl;
	intarr2Csv((char *) "index.csv", (char *) "index", pixel_index.size(), 1, &pixel_index[0], 0);
}

