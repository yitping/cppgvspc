/*
 */

#ifndef GVSPC_ERROR_CODE_H
#define GVSPC_ERROR_CODE_H

typedef enum GVSPC_ERROR
{
  GVSPC_ERROR_FATAL = -1,
  GVSPC_ERROR_NONE  = 0,
  GVSPC_ERROR_SUBR_FAILED,
  GVSPC_ERROR_FILE_IO,
  GVSPC_ERROR_HEAP_EMPTY,
  GVSPC_ERROR_MODE_UNKNOWN,
  GVSPC_ERROR_INPUT_BAD,
  GVSPC_ERROR_INIT_BAD
} gvspc_error_code;

#endif /* GVSPC_ERROR_CODE_H */