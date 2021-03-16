#ifndef DEF_WIN_H
#define DEF_WIN_H
#include "log.h"

#define SAFE_RELEASE(punk) if ((punk) != nullptr) { (punk)->Release(); (punk) = nullptr; }
#define EXIT_ON_ERROR(hres) if (FAILED(hres)) { FileDebug() << "HRESULT FAILED " << Qt::hex << HRESULT_CODE(hres) << HResultToString(hres); goto Exit; }

#endif // DEF_WIN_H
