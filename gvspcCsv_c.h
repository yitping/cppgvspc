#ifndef GVSPC_IMAGE_H
#define GVSPC_IMAGE_H 

#include <stdio.h>
#include <string.h>
#include <cpl.h>

#define CSV_DBL_FORMAT1 "%.4f"
#define CSV_DBL_FORMAT2 ",%.4f"
#define CSV_INT_FORMAT1 "%d"
#define CSV_INT_FORMAT2 ",%d"

void csv2Array_split(char *line, void *val, char *sep, cpl_type t);
int csv2Array(char *csv, char *var, int nrow, int ncol, void *arr, cpl_type t);
int csv2Dblarr(char *csv, char *var, int nrow, int ncol, double *arr);
int csv2Intarr(char *csv, char *var, int nrow, int ncol, int *arr);
int array2Csv(char *csv, char *var, int nrow, int ncol, void *arr, int append, cpl_type t);
int dblarr2Csv(char *csv, char *var, int nrow, int ncol, double *arr, int append);
int intarr2Csv(char *csv, char *var, int nrow, int ncol, int *arr, int append);

#endif /* GVSPC_IMAGE_H */
