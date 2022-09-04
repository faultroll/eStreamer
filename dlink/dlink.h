
#ifndef _DLINK_H
#define _DLINK_H

// auto datatransfer between modules

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum _dl_mod_e_ {
    // common input/output must above 0, for nbds/map defect
    DL_USER = 1,
    DL_INPUT,
    DL_OUTPUT,
    DL_INOUT, // seems no need to use this
    DL_INNER_BEGIN = 11,
    // video modules
    DL_VIN,  // DL_VFRAME(out)
    DL_VENC, // DL_VFRAME(in)/DL_VSTREAM(out)
    DL_VDEC, // DL_VSTREAM(in)/DL_VFRAME(out)
    DL_VMUL, // DL_VFRAME(in/out)
    DL_VMIX, // DL_VFRAME(in/out)
    DL_VOUT, // DL_VFRAME(in)
    // audio modules
    DL_AIN,  // DL_AFRAME(out)
    DL_AENC, // DL_AFRAME(in)/DL_ASTREAM(out)
    DL_ADEC, // DL_ASTREAM(out)/DL_AFRAME(in)
    DL_AMUL, // DL_AFRAME(in/out)
    DL_AMIX, // DL_AFRAME(in/out)
    DL_AOUT, // DL_AFRAME(in)
    DL_INNER_END = 100,
} DL_MOD_E;

typedef enum _dl_type_e {
    DL_VFRAME = 0,  // DL_VFRAME and DL_VSTREAM is DL_NORMAL task
    DL_VSTREAM,
    DL_AFRAME,      // DL_AFRAME and DL_ASTREAM is DL_FAST task
    DL_ASTREAM,
    DL_TYPE_BUTT,
} DL_TYPE_E;

typedef struct _dl_chn_s_ {
    DL_MOD_E mod;
    int grp;
    int chn;
} DL_ID_S;
#define DL_ID_EMPTY    { .mod = -1, .grp = -1, .chn = -1, }

typedef enum _dl_speed_e {
    DL_SLOW     = 1,
    DL_NORMAL   = 3,
    DL_FAST     = 5,
    DL_SPEED_BUTT,
} DL_SPEED_E;

typedef void *DL_DATA_T; // generic, cast to user struct
typedef int (*DL_GEN_F)(DL_ID_S, DL_DATA_T);  // generate outdata
typedef int (*DL_PROC_F)(DL_ID_S, /* const */ DL_DATA_T); // process indata

typedef struct _dl_des_s_ {
    DL_MOD_E mod;            // module enum
    char     name[16];
    struct {
        DL_SPEED_E  inspeed;     // indata speed, set by user
        DL_TYPE_E   indatatype;  // indata type
        DL_PROC_F   indatafun;   // (outchn) process indata
    };
    struct {
        DL_SPEED_E  outspeed;    // outdata speed, set by user
        DL_TYPE_E   outdatatype; // outdata type
        DL_GEN_F    outdatafun;  // (inchn) generate outdata
        DL_PROC_F   freedatafun; // free after process
    };
} DL_DES_S;


// TODO(lgY): add drain-mode
int dl_init(void);
int dl_fini(void);
// link channels
int dl_link(DL_ID_S from, // src
            DL_ID_S to // dst
           );
int dl_unlink(DL_ID_S from, // src
              DL_ID_S to // dst
             );
int dl_status(void); // print status
// modules used by dl_link/dl_unlink
int dl_create_chn(DL_DES_S *des);
int dl_destroy_chn(int chn);
// for convenient
int dl_create_inchn(DL_GEN_F outdata, // inchn generate outdata
                    DL_TYPE_E outtype,
                    DL_PROC_F freedata
                   ); // return id(chn)
int dl_create_outchn(DL_PROC_F indata, // outchn process indata
                     DL_TYPE_E intype
                    );  // return id(chn)

#if defined(__cplusplus)
}
#endif

#endif // _DLINK_H
