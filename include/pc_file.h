#ifndef PC_FILE_H
#define PC_FILE_H

#include "pc_component_data.h"

/* Cross-platform export/import macros for Windows DLL vs Linux static/shared compatibility */
#if defined(_WIN32) || defined(_WIN64)
#ifdef PC_FILE_EXPORTS
#define PC_API __declspec(dllexport)
#else
#define PC_API __declspec(dllimport)
#endif
#else
#define PC_API
#endif

/* Sử dụng struct tường minh, không dùng typedef */
PC_API void file_load_components(struct component_manager *mgr_p);
PC_API void file_save_components(struct component_manager *mgr_p);

#endif /* PC_FILE_H */