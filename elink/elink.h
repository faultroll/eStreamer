
#ifndef _ELINK_H_
#define _ELINK_H_

// auto datatransfer between modules

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum _elink_mod_e_ {
    // common input/output must above 0, for nbds/map defect
    ELINK_INPUT = 1,
    ELINK_OUTPUT,
    ELINK_INOUT, // seems no need to use this
    ELINK_INNER_BEGIN = 10,
    // video modules
    ELINK_VIN,  // ELINK_VFRAME(out)
    ELINK_VENC, // ELINK_VFRAME(in)/ELINK_VSTREAM(out)
    ELINK_VDEC, // ELINK_VSTREAM(in)/ELINK_VFRAME(out)
    ELINK_VMUL, // ELINK_VFRAME(in/out)
    ELINK_VMIX, // ELINK_VFRAME(in/out)
    ELINK_VOUT, // ELINK_VFRAME(in)
    // audio modules
    ELINK_AIN,  // ELINK_AFRAME(out)
    ELINK_AENC, // ELINK_AFRAME(in)/ELINK_ASTREAM(out)
    ELINK_ADEC, // ELINK_ASTREAM(out)/ELINK_AFRAME(in)
    ELINK_AMUL, // ELINK_AFRAME(in/out)
    ELINK_AMIX, // ELINK_AFRAME(in/out)
    ELINK_AOUT, // ELINK_AFRAME(in)
    ELINK_INNER_END = 100,
} ELINK_MOD_E;

typedef enum _elink_type_e {
    ELINK_VFRAME = 0,  // ELINK_VFRAME and ELINK_VSTREAM is ELINK_NORMAL task
    ELINK_VSTREAM,
    ELINK_AFRAME,      // ELINK_AFRAME and ELINK_ASTREAM is ELINK_FAST task
    ELINK_ASTREAM,
    ELINK_TYPE_BUTT,
} ELINK_TYPE_E;

typedef struct _elink_chn_s_ {
    ELINK_MOD_E mod;
    int grp;
    int chn;
} ELINK_ID_S;

typedef enum _elink_speed_e {
    ELINK_SLOW     = 1,
    ELINK_NORMAL   = 3,
    ELINK_FAST     = 5,
    ELINK_SPEED_BUTT,
} ELINK_SPEED_E;

typedef void *ELINK_DATA_T; // generic, cast to user struct
typedef int (*ELINK_GEN_F)(ELINK_ID_S, ELINK_DATA_T);  // generate outdata
typedef int (*ELINK_PROC_F)(ELINK_ID_S, ELINK_DATA_T); // process indata

typedef struct _elink_des_s_ {
    ELINK_MOD_E mod;            // module enum
    char        name[16];
    struct {
        ELINK_SPEED_E  inspeed;     // indata speed, set by user
        ELINK_TYPE_E   indatatype;  // indata type
        ELINK_PROC_F   indatafun;   // (outchn) process indata
    };
    struct {
        ELINK_SPEED_E  outspeed;    // outdata speed, set by user
        ELINK_TYPE_E   outdatatype; // outdata type
        ELINK_GEN_F    outdatafun;  // (inchn) generate outdata
        ELINK_PROC_F   freedatafun; // free after process
    };
} ELINK_DES_S;


int elink_init(void);
int elink_fini(void);
// link channels
int elink_link(ELINK_ID_S from, // src
               ELINK_ID_S to // dst
              );
int elink_unlink(ELINK_ID_S from, // src
                 ELINK_ID_S to // dst
                );
int elink_status(void); // print status
// modules used by elink_link/elink_unlink
int elink_create_chn(ELINK_DES_S *des);
int elink_destroy_chn(int chn);
int elink_create_inchn(ELINK_GEN_F outdata, // inchn generate outdata
                       ELINK_TYPE_E outtype,
                       ELINK_PROC_F freedata
                      ); // return id(chn)
int elink_create_outchn(ELINK_PROC_F indata, // outchn process indata
                        ELINK_TYPE_E intype
                       );  // return id(chn)

#if defined(__cplusplus)
}
#endif

#endif // _ELINK_H_
