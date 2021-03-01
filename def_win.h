#ifndef DEF_WIN_H
#define DEF_WIN_H
#include "log.h"

#define SAFE_RELEASE(punk) if ((punk) != NULL) { (punk)->Release(); (punk) = NULL; }
#define EXIT_ON_ERROR(hres) if (FAILED(hres)) { FileDebug() << "HRESULT FAILED " << hres; goto Exit; }

#endif // DEF_WIN_H
