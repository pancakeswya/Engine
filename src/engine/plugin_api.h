#ifndef ENGINE_PLUGIN_API_H_
#define ENGINE_PLUGIN_API_H_

#ifdef ENGINE_SHARED
#   if defined(_WIN32) && !defined(__MINGW32__)
#       ifdef ENGINE_EXPORT
#         define ENGINE_API __declspec(dllexport)
#       else
#         define ENGINE_API __declspec(dllimport)
#       endif
#       define ENGINE_CONV __stdcall
#   else
#     define ENGINE_API __attribute__ ((visibility ("default")))
#     define ENGINE_CONV
#   endif
#else
#   define ENGINE_API
#   define ENGINE_CONV
#endif

#endif //ENGINE_PLUGIN_API_H_
