/* Disassembler error definitions */

#define DDL_VIO 1    /* DDLIM violation */
#define ADL_VIO 2    /* ADLIM violation */
#define AIL_VIO 3    /* AILIM violation */
#define PIL_VIO 4    /* PILIM violation */
#define PDL_VIO 5    /* PDLIM violation */
#define IDL_VIO 6    /* IDLIM violation */
#define IIL_VIO 7    /* IIDLIM violation */
#define ASL_VIO 8    /* ASLIM violation */
#define ALL_VIO 9    /* ALLIM violation */
#define PCL_VIO 10   /* PCLIM violation */
#define PCI_VIO 11   /* PCILIM violation */
#define IML_VIO 12   /* IMLIM violation */

#define MD7_ILL 13   /* illegal mode 7 continuation */
#define ISZ_ILL 14   /* illegal size specification */
#define RLC_OVR 15   /* relocation overrun */

#define OOM_ERR 30   /* out of memory error */
#define FSK_ERR 31   /* File seek error     */
#define RLC_ERR 32   /* Relocation too big  */
#define FRD_ERR 33   /* File read error     */
#define FOP_ERR 34   /* File open error     */
#define NPG_ERR 35   /* No program error    */

#define ABORT 1
#define CONTINUE 0
