
#ifndef _ELINK_H_
#define _ELINK_H_

// auto datatransfer between modules

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum _el_mod_e_ {
    // common input/output must above 0, for nbds/map defect
    EL_INPUT = 1,
    EL_OUTPUT,
    EL_INOUT, // seems no need to use this
    EL_INNER_BEGIN = 11,
    // video modules
    EL_VIN,  // EL_VFRAME(out)
    EL_VENC, // EL_VFRAME(in)/EL_VSTREAM(out)
    EL_VDEC, // EL_VSTREAM(in)/EL_VFRAME(out)
    EL_VMUL, // EL_VFRAME(in/out)
    EL_VMIX, // EL_VFRAME(in/out)
    EL_VOUT, // EL_VFRAME(in)
    // audio modules
    EL_AIN,  // EL_AFRAME(out)
    EL_AENC, // EL_AFRAME(in)/EL_ASTREAM(out)
    EL_ADEC, // EL_ASTREAM(out)/EL_AFRAME(in)
    EL_AMUL, // EL_AFRAME(in/out)
    EL_AMIX, // EL_AFRAME(in/out)
    EL_AOUT, // EL_AFRAME(in)
    EL_INNER_END = 100,
} EL_MOD_E;

typedef enum _el_type_e {
    EL_VFRAME = 0,  // EL_VFRAME and EL_VSTREAM is EL_NORMAL task
    EL_VSTREAM,
    EL_AFRAME,      // EL_AFRAME and EL_ASTREAM is EL_FAST task
    EL_ASTREAM,
    EL_TYPE_BUTT,
} EL_TYPE_E;

typedef struct _el_chn_s_ {
    EL_MOD_E mod;
    int grp;
    int chn;
} EL_ID_S;

typedef enum _el_speed_e {
    EL_SLOW     = 1,
    EL_NORMAL   = 3,
    EL_FAST     = 5,
    EL_SPEED_BUTT,
} EL_SPEED_E;

typedef void *EL_DATA_T; // generic, cast to user struct
typedef int (*EL_GEN_F)(EL_ID_S, EL_DATA_T);  // generate outdata
typedef int (*EL_PROC_F)(EL_ID_S, EL_DATA_T); // process indata

typedef struct _el_des_s_ {
    EL_MOD_E mod;            // module enum
    char        name[16];
    struct {
        EL_SPEED_E  inspeed;     // indata speed, set by user
        EL_TYPE_E   indatatype;  // indata type
        EL_PROC_F   indatafun;   // (outchn) process indata
    };
    struct {
        EL_SPEED_E  outspeed;    // outdata speed, set by user
        EL_TYPE_E   outdatatype; // outdata type
        EL_GEN_F    outdatafun;  // (inchn) generate outdata
        EL_PROC_F   freedatafun; // free after process
    };
} EL_DES_S;


int el_init(void);
int el_fini(void);
// link channels
int el_link(EL_ID_S from, // src
            EL_ID_S to // dst
           );
int el_unlink(EL_ID_S from, // src
              EL_ID_S to // dst
             );
int el_status(void); // print status
// modules used by el_link/el_unlink
int el_create_chn(EL_DES_S *des);
int el_destroy_chn(int chn);
int el_create_inchn(EL_GEN_F outdata, // inchn generate outdata
                    EL_TYPE_E outtype,
                    EL_PROC_F freedata
                   ); // return id(chn)
int el_create_outchn(EL_PROC_F indata, // outchn process indata
                     EL_TYPE_E intype
                    );  // return id(chn)

#if defined(__cplusplus)
}
#endif

#endif // _ELINK_H_
