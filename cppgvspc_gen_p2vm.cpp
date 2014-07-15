#include <iostream>
#include <cpl.h>
#include <unistd.h>
#include <string.h>
#include "gvspcSensor.h"


int main(int argc, char **argv)
{
	int retval=EXIT_SUCCESS, errflag=0, optflag[5]={0}, msgflag=0, opt, i;
  char **files_dark, **files_tel1, **files_tel2, **files_tel3, **files_tel4,
	*pfile, *file_index, *file_p2vm;
  int num_dark=0, num_tel1=0, num_tel2=0, num_tel3=0, num_tel4=0;
  cpl_image *image;
	
  cpl_init(CPL_INIT_DEFAULT);
	
  file_p2vm  = NULL;
  file_index = NULL;
  files_dark = NULL;
  files_tel1 = NULL;
  files_tel2 = NULL;
  files_tel3 = NULL;
  files_tel4 = NULL;
	
  while ((opt = getopt(argc, argv, ":i:d:1:2:3:4:p:m")) != -1)
  {
		switch (opt)
		{
      case 'i':
				file_index = optarg;
				break;
      case 'p':
				file_p2vm = optarg;
				break;
      case 'm':
				msgflag = 1;
				break;
      case 'd':
      case '1':
      case '2':
      case '3':
      case '4':
				i = optind-1;
				/* just one pass for each */
				if (opt == 'd')
				{
					files_dark = &argv[i];
					if (optflag[0]++) { errflag++; break; }
				}
				if (opt == '1')
				{
					files_tel1 = &argv[i];
					if (optflag[1]++) { errflag++; break; }
				}
				if (opt == '2')
				{
					files_tel2 = &argv[i];
					if (optflag[2]++) { errflag++; break; }
				}
				if (opt == '3')
				{
					files_tel3 = &argv[i];
					if (optflag[3]++) { errflag++; break; }
				}
				if (opt == '4')
				{
					files_tel4 = &argv[i];
					if (optflag[4]++) { errflag++; break; }
				}
				while (i < argc)
				{
					pfile = argv[i++];
					if (pfile[0] != '-')
					{
						if (opt == 'd') num_dark++;
						if (opt == '1') num_tel1++;
						if (opt == '2') num_tel2++;
						if (opt == '3') num_tel3++;
						if (opt == '4') num_tel4++;
					}
					else break;
				}
				optind = i-1;
				break;
      case ':':
				std::cerr << "option -" << optopt << " requires an operand" << std::endl;
				errflag++;
				break;
      case '?':
				std::cerr << "unrecognized option: -" << optopt << std::endl;
				errflag++;
				break;
    }
  }
	
	cpl_msg_set_level((msgflag) ? CPL_MSG_DEBUG : CPL_MSG_INFO);
	
	if (errflag || (file_index == NULL) || (num_dark == 0) ||
      (num_tel1 == 0) || (num_tel2 == 0) || (num_tel3 == 0) || (num_tel4 == 0))
  {
		std::cerr << "Usage: " << argv[0] << " -i index [-p p2vm -m]\n\t-d dark_fits1 [dark_fits2 ...]\n\t-1 tel1_fits1 [tel1_fits2 ...]\n\t-2 tel2_fits1 [tel2_fits2 ...]\n\t-3 tel3_fits1 [tel3_fits2 ...]\n\t-4 tel4_fits1 [tel4_fits2 ...]\n" << std::endl;
    retval = EXIT_FAILURE;
  }
  else
  {
		if (file_p2vm == NULL) file_p2vm = (char *) "p2vm.csv";
    for (i=0; i<num_dark; i++) cpl_msg_info(cpl_func, "dark file: %s", files_dark[i]);
    for (i=0; i<num_tel1; i++) cpl_msg_info(cpl_func, "tel1 file: %s", files_tel1[i]);
    for (i=0; i<num_tel2; i++) cpl_msg_info(cpl_func, "tel2 file: %s", files_tel2[i]);
    for (i=0; i<num_tel3; i++) cpl_msg_info(cpl_func, "tel3 file: %s", files_tel3[i]);
    for (i=0; i<num_tel4; i++) cpl_msg_info(cpl_func, "tel4 file: %s", files_tel4[i]);
    cpl_msg_info(cpl_func, "p2vm file: %s", file_p2vm);
		
		// instantiate and init sensor
		gvspcSensor sensor;
		sensor.load_pixel_indices(file_index);
		
		if (sensor.has_indices())
		{
			cpl_msg_info(cpl_func, "reading dark fits...");
			for (i=0; i<num_dark; i++)
			{
				cpl_msg_info(cpl_func, "loading %s", files_dark[i]);
				image = cpl_image_load(files_dark[i], CPL_TYPE_DOUBLE, 0, 0);
				if (image == NULL)
				{
					std::cerr << "image is null" << std::endl;
					continue;
				}
				sensor.process_image(image, 0);
				cpl_image_delete(image);
			}
			sensor.save_dark();
			
			cpl_msg_info(cpl_func, "reading tel1 fits...");
			for (i=0; i<num_tel1; i++)
			{
				cpl_msg_info(cpl_func, "loading %s", files_tel1[i]);
				image = cpl_image_load(files_tel1[i], CPL_TYPE_DOUBLE, 0, 0);
				if (image == NULL)
				{
					std::cerr << "image is null" << std::endl;
					continue;
				}
				sensor.process_image(image, 1);
				cpl_image_delete(image);
			}
			sensor.save_phot(1);
			
			cpl_msg_info(cpl_func, "reading tel2 fits...");
			for (i=0; i<num_tel2; i++)
			{
				cpl_msg_info(cpl_func, "loading %s", files_tel2[i]);
				image = cpl_image_load(files_tel2[i], CPL_TYPE_DOUBLE, 0, 0);
				if (image == NULL)
				{
					std::cerr << "image is null" << std::endl;
					continue;
				}
				sensor.process_image(image, 1);
				cpl_image_delete(image);
			}
			sensor.save_phot(2);
			
			cpl_msg_info(cpl_func, "reading tel3 fits...");
			for (i=0; i<num_tel3; i++)
			{
				cpl_msg_info(cpl_func, "loading %s", files_tel3[i]);
				image = cpl_image_load(files_tel3[i], CPL_TYPE_DOUBLE, 0, 0);
				if (image == NULL)
				{
					std::cerr << "image is null" << std::endl;
					continue;
				}
				sensor.process_image(image, 1);
				cpl_image_delete(image);
			}
			sensor.save_phot(3);
			
			cpl_msg_info(cpl_func, "reading tel4 fits...");
			for (i=0; i<num_tel4; i++)
			{
				cpl_msg_info(cpl_func, "loading %s", files_tel4[i]);
				image = cpl_image_load(files_tel4[i], CPL_TYPE_DOUBLE, 0, 0);
				if (image == NULL)
				{
					std::cerr << "image is null" << std::endl;
					continue;
				}
				sensor.process_image(image, 1);
				cpl_image_delete(image);
			}
			sensor.dump_a_pix();
			sensor.save_phot(4);
			
			long x;
			int k=4, j=1800, p=2;
			i = 6;
			x = k*i*j*p;
			
			std::cout << "Hello, world!" << std::endl;
			std::cout << "tell me more!" << std::endl;
			std::cout << "x = " << x << std::endl;

			sensor.set_default_ps();
//			sensor.load_v2pms("v2pms.csv");
			sensor.compute_v2pms();
			sensor.save_v2pms(file_p2vm);
//			sensor.dump_an_index();
			
		}
		
		gvspcFifo<int> bint(5);
		gvspcFifo<double> bdbl(10);
		
		bint.add(6); bint.add(1); bint.add(2); bint.add(6); bint.add(7); bint.add(10); bint.add(3);
		bdbl.add(6.3); bdbl.add(2.4); bdbl.add(5.1); bdbl.add(3.9); bdbl.add(4.8);
		
		i = 2; std::cout << "bint[" << i << "] = " << bint[i] << std::endl;
		i = 0; std::cout << "bdbl[" << i << "] = " << bdbl[i] << std::endl;
		std::cout << "computing mean..." << std::endl;
		std::cout << "mean bint = " << bint.mean() << "; var bint = " << bint.var() << std::endl;
		std::cout << "done mean..." << std::endl;
		std::cout << "computing mean..." << std::endl;
		std::cout << "mean bdbl = " << bdbl.mean() << "; var bint = " << bdbl.var() << std::endl;
		std::cout << "done mean..." << std::endl;
		
		for (i=0; i<bint.size(); i++)
		{
			std::cout << ((i == 0) ? "" : ", ") << bint[i];
		}
		std::cout << std::endl;
		for (i=0; i<bdbl.size(); i++)
		{
			std::cout << ((i == 0) ? "" : ", ") << bdbl[i];
		}
		std::cout << std::endl;
		
		gvspcFifo<gvspcPix> fifo_phot(4);
		for (i=0; i<10; i++)
		{
			std::cout << "creating..." << std::endl;
			gvspcPix new_pix(10,2);
			std::cout << "done..." << std::endl;
			fifo_phot.add(new_pix);
			std::cout << "leaving..." << std::endl;
		}
		std::cout << "computing mean..." << std::endl;
		gvspcPix mu = fifo_phot.mean();
		std::cout << "done mean..." << std::endl;
		
//		gvspcCsv sample("v2pms.csv");
//		sample.info();
//		std::vector<std::vector<double> > dd;
//		std::vector<std::vector<int> > ii;
//		std::cout << "Reading variable #"<< sample.read_as_dbl("v2pm2_p0", dd) << std::endl;
//		for (i=0; i<dd.size(); i++)
//		{
//			for (int j=0; j<dd[i].size(); j++)
//				std::cout << ((j==0) ? "" : ",") << dd[i][j];
//			std::cout << std::endl;
//		}
//		sample.read_as_dbl("v2pm49_p0", dd);
//		std::cout << std::endl;
//		std::cout << "Reading variable #"<< sample.read_as_int("v2pm0_p0", ii) << std::endl;
//		for (i=0; i<ii.size(); i++)
//		{
//			for (int j=0; j<ii[i].size(); j++)
//				std::cout << ((j==0) ? "" : ",") << ii[i][j];
//			std::cout << std::endl;
//		}
//		std::cout << std::endl;
//		std::cout << "Reading variable #"<< sample.read_as_dbl("v2pm0_p0", dd) << std::endl;
//		for (i=0; i<dd.size(); i++)
//		{
//			for (int j=0; j<dd[i].size(); j++)
//				std::cout << ((j==0) ? "" : ",") << dd[i][j];
//			std::cout << std::endl;
//		}
		
	}
	
	if (!cpl_memory_is_empty()) cpl_msg_error(cpl_func, "memory is not empty.");
	cpl_end();
	return retval;
}
