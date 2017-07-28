
#ifndef __FILE_IOS__
#define __FILE_IOS__
//added by gxh
#ifndef _WIN32
#ifdef __cplusplus
#define EXTERNC extern "C"
EXTERNC {
#endif
    __attribute__ ((__visibility__("default"))) char *MAKE_FILE_NAME(char *name);
#ifdef __cplusplus
}
#endif
#endif


#endif /* defined(__FILE_IOS__) */
