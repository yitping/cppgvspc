#include <vector>
#include <iostream>
#include "gvspcSensor.h"
#include "gvspcCsv.h"
#include "gvspcErrorCode.h"

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
	
	for (t=0; t<n_tel; t++) sum[t] = mean_phot[t].sum()/n_ch;
	for (p=0; p<n_pl; p++) for (j=0; j<n_ch; j++)
	{
		l = p*n_ch + j;
		for (t=0; t<n_tel; t++) subpix[t] = gvspcPix(mean_phot[t],j,p);
		v2pms[l] = gvspcV2PM(subpix, sum.data(), tels, ps.data());
	}
	
	return GVSPC_ERROR_NONE;
}

int gvspcSensor::save_v2pms_to_file(const char *fname)
{
	int p, j, l;
	std::string label;
	for (p=0; p<n_pl; p++) for (j=0; j<n_ch; j++)
	{
		l = p*n_ch + j;
		label = "v2pm" + std::to_string(j) + "_p" + std::to_string(p);
		v2pms[l].save_to_file(fname, label.c_str(), ((p == 0) && (j == 0)) ? 0 : 1);
	}
	return GVSPC_ERROR_NONE;
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
	tels[0].assign(std::begin(ti), std::end(ti));
	tels[1].assign(std::begin(tj), std::end(tj));
	
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
}


/////////////

void gvspcSensor::dump_a_pix()
{
	std::cout << "Size of dark fifo = " << fifo_pix_dark.size() << std::endl;
	if (fifo_pix_dark.size() != 0)
		std::cout << "Size of latest pix in fifo = " << fifo_pix_dark.last().size() << std::endl;
	std::cout << "Size of flux fifo = " << fifo_pix_flux.size() << std::endl;
	if (fifo_pix_flux.size() != 0)
		std::cout << "Size of latest pix in fifo = " << fifo_pix_flux.last().size() << std::endl;
	
	double *np = new double[mean_phot[1].size()];;
	for (long i=0; i<mean_phot[1].size(); i++)
	{
		np[i] = mean_phot[1][i];
		std::cout << np[i] << ",";
	}
	std::cout << std::endl;
	dblarr2Csv((char *) "abc.csv", (char *) "flux", mean_phot[1].size()/24, 24, np, 0);
	std::cout << std::endl;
	std::cout << std::endl;
//	gvspcPix a = (pix_cals + pix_cals)/2;
//	for (int i=0; i<6; i++) for (int k=0; k<4; k++)
//	{
//		std::cout << (((i == 0) && (k == 0)) ? "" : ", ") << a.get(k,i,10,0);
//	}
	std::cout << std::endl;
	delete[] np;
}


void gvspcSensor::dump_an_index()
{
	std::cout << "Pixel index size = " << pixel_index.size() << std::endl;
	intarr2Csv((char *) "index.csv", (char *) "index", pixel_index.size(), 1, &pixel_index[0], 0);
}

