#ifndef __MACROS_H__
#define __MACROS_H__

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#define OK 0
#define FAIL -1

#ifndef DPSIZE
#define DPSIZE (2 << 13)
#endif

#ifndef DATAFILE
#define DATAFILE "/scratch/youngmoon01/garbage2.bin"
#endif

#ifndef PORT 
#define PORT 19999
#endif

#ifndef NSERVERS
#define NSERVERS 39
#endif

#ifndef HOST
#define HOST "10.20.12.170"
#endif

#ifndef ALPHA
#define ALPHA 0.03f
#endif

#ifndef LOT
#define LOT 2048
#endif

#endif
