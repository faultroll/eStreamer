
## intro

dlink treat all modules as a box with in-channel and out-channel. it uses the `link` operation to link modules. once two modules are linked, data will be transfered between them, from src out-chn to dst in-chn. 

eg. vdec has 1 out-chn, vmul has 1 in-chn and multi out-chn, vout has 1 in-chn. if we want to decode from 1 source and display them in two screen, we create three `link`s vdec0_outchn_0-vmul0_inchn0, vmul0_outchn0-vout0_inchn0, vmul0_outchn1-vout1_inchn0. thus data will be transfered from vdec through vmul to vout. 

## depend

a MT-safe map(we use nbds_map here)

c11 atomics(cas/...) and threads

