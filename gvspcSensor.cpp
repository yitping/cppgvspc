#include <vector>
#include <iostream>
#include "gvspcSensor.h"
#include "gvspcCsv.h"
#include "gvspcErrorCode.h"

gvspcSensor::gvspcSensor() { init(); }
gvspcSensor::~gvspcSensor() { std::cout << "1 gvspcSensor destroyed!" << std::endl; }

int gvspcSensor::isNull()
{
	if (pixel_index.empty()) return 1;
	return 0;
}

int gvspcSensor::load_pixel_indices(char *csv)
{
  int headerinfo[2], csv_n_ch, csv_n_sl, csv_n_pl, csv_corner_ch, csv_corner_io;
  //    *temp_int;
  //double *temp_dbl;
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
			fifo_pix_flux.add(new_pix);
			cpl_msg_debug(cpl_func, "adding instantaneous flux into fifo..");
	}
	
	return GVSPC_ERROR_NONE;
}


int gvspcSensor::save_dark()
{
	return GVSPC_ERROR_NONE;
}


///// private /////

void gvspcSensor::init()
{
	n_ch = 0; n_pl = 0; corner_ch = 0; corner_io = 0, bw = 0;
	fifo_pix_dark.limit(5);
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
	
	double *np = new double[fifo_pix_dark.last().size()];;
	for (long i=0; i<fifo_pix_dark.last().size(); i++)
	{
		np[i] = fifo_pix_dark.last()[i];
		std::cout << np[i] << ",";
	}
	std::cout << std::endl;
	dblarr2Csv((char *) "abc.csv", (char *) "flux", fifo_pix_dark.last().size(), 1, np, 0);
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

