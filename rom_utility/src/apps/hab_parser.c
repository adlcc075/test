#include "io.h"
#include "platform.h"
#include "hab_parser.h"
#include "shell/shell.h"

///////////////////// External ////////////////////////////////

extern chip_e chip;

/////////////////// Local var and function /////////////////////

static char *get_hab_reason_desc(uint8_t rsn)
{
    return rsn == HAB_RSN_ANY ? "HAB_RSN_ANY" :
        rsn == HAB_ENG_FAIL ? "HAB_ENG_FAIL" :
        rsn == HAB_INV_ADDRESS ? "HAB_INV_ADDRESS" :
        rsn == HAB_INV_ASSERTION ? "HAB_INV_ASSERTION" :
        rsn == HAB_INV_CALL ? "HAB_INV_CALL" :
        rsn == HAB_INV_CERTIFICATE ? "HAB_INV_CERTIFICATE" :
        rsn == HAB_INV_COMMAND ? "HAB_INV_COMMAND" :
        rsn == HAB_INV_CSF ? "HAB_INV_CSF" :
        rsn == HAB_INV_DCD ? "HAB_INV_DCD" :
        rsn == HAB_INV_INDEX ? "HAB_INV_INDEX" :
        rsn == HAB_INV_IVT ? "HAB_INV_IVT" :
        rsn == HAB_INV_KEY ? "HAB_INV_KEY" :
        rsn == HAB_INV_RETURN ? "HAB_INV_RETURN" :
        rsn == HAB_INV_SIGNATURE ? "HAB_INV_SIGNATURE" :
        rsn == HAB_INV_SIZE ? "HAB_INV_SIZE" :
        rsn == HAB_MEM_FAIL ? "HAB_MEM_FAIL" :
        rsn == HAB_OVR_COUNT ? "HAB_OVR_COUNT" :
        rsn == HAB_OVR_STORAGE ? "HAB_OVR_STORAGE" :
        rsn == HAB_UNS_ALGORITHM ? "HAB_UNS_ALGORITHM" :
        rsn == HAB_UNS_COMMAND ? "HAB_UNS_COMMAND" :
        rsn == HAB_UNS_ENGINE ? "HAB_UNS_ENGINE" :
        rsn == HAB_UNS_ITEM ? "HAB_UNS_ITEM" :
        rsn == HAB_UNS_KEY ? "HAB_UNS_KEY" :
        rsn == HAB_UNS_PROTOCOL ? "HAB_UNS_PROTOCOL" :
        rsn == HAB_UNS_STATE ? "HAB_UNS_STATE" : "Invalid reason";
}

static char *get_hab_status_desc(uint8_t status)
{
    return status == HAB_FAILURE ? "HAB_FAILURE" :
        status == HAB_WARNING ? "HAB_WARNING" :
        status == HAB_SUCCESS ? "HAB_SUCCESS" :
        status == HAB_STS_ANY ? "HAB_STS_ANY" : "Invalid status";
}

static char *get_hab_context_desc(uint8_t context)
{
    return context == HAB_CTX_ANY ? "	HAB_CTX_ANY" :
        context == HAB_CTX_FAB ? "HAB_CTX_FAB" :
        context == HAB_CTX_ENTRY ? "HAB_CTX_ENTRY" :
        context == HAB_CTX_TARGET ? "HAB_CTX_TARGET" :
        context == HAB_CTX_AUTHENTICATE ? "HAB_CTX_AUTHENTICATE" :
        context == HAB_CTX_DCD ? "HAB_CTX_DCD" :
        context == HAB_CTX_CSF ? "HAB_CTX_CSF" :
        context == HAB_CTX_COMMAND ? "HAB_CTX_COMMAND" :
        context == HAB_CTX_AUT_DAT ? "HAB_CTX_AUT_DAT" :
        context == HAB_CTX_ASSERT ? "HAB_CTX_ASSERT" :
        context == HAB_CTX_EXIT ? "HAB_CTX_EXIT" : "Invalid context";
}

static char *get_hab_eng_desc(uint8_t eng)
{
    return eng == HAB_ENG_ANY ? "HAB_ENG_ANY" :
        eng == HAB_ENG_SCC ? "HAB_ENG_SCC" :
        eng == HAB_ENG_RTIC ? "HAB_ENG_RTIC" :
        eng == HAB_ENG_SAHARA ? "HAB_ENG_SAHARA" :
        eng == HAB_ENG_CSU ? "HAB_ENG_CSU" :
        eng == HAB_ENG_SRTC ? "HAB_ENG_SRTC" :
        eng == HAB_ENG_DCP ? "HAB_ENG_DCP" :
        eng == HAB_ENG_CAAM ? "HAB_ENG_CAAM" :
        eng == HAB_ENG_SNVS ? "HAB_ENG_SNVS" :
        eng == HAB_ENG_OCOTP ? "HAB_ENG_OCOTP" :
        eng == HAB_ENG_DTCP ? "HAB_ENG_DTCP" :
        eng == HAB_ENG_ROM ? "HAB_ENG_ROM" :
        eng == HAB_ENG_HDCP ? "HAB_ENG_HDCP" :
        eng == HAB_ENG_RTL ? "HAB_ENG_RTL" : eng == HAB_ENG_SW ? "HAB_ENG_SW" : "Invalid eng";
}

static char *get_hab_state_cmd_desc(uint8_t cmd)
{
    return cmd == HAB_STATE_CMD_UNAUTH ? "HAB_STATE_CMD_UNAUTH" :
        cmd == HAB_STATE_CMD_PREAUTH ? "HAB_STATE_CMD_PREAUTH" :
        cmd == HAB_STATE_CMD_AUTH ? "HAB_STATE_CMD_AUTH" : "Invalid CMD state";
}

static char *get_hab_state_lib_desc(uint8_t lib)
{
    return lib == HAB_STATE_LIB_RESET ? "HAB_STATE_LIB_RESET" :
        lib == HAB_STATE_LIB_INITIAL ? "HAB_STATE_LIB_INITIAL" :
        lib == HAB_STATE_LIB_INNER ? "HAB_STATE_LIB_INNER" :
        lib == HAB_STATE_LIB_OUTER ? "HAB_STATE_LIB_OUTER" : "Invalid LIB state";
}

static char *get_hab_key_entry_type(uint8_t type)
{
    return type == HAB_KEY_PUBLIC ? "HAB_KEY_PUBLIC" :
        type == HAB_KEY_SECRET ? "HAB_KEY_SECRET" :
        type == HAB_KEY_MASTER ? "HAB_KEY_MASTER" :
        type == HAB_KEY_HASH ? "HAB_KEY_HASH" : "Unkonw key type";
}

static char *get_hab_key_entry_alg(uint8_t alg)
{

    return alg == HAB_ALG_PKCS1 ? "HAB_ALG_PKCS1" :
        alg == HAB_ALG_AES ? "HAB_ALG_AES" :
        alg == HAB_ALG_BLOB ? "HAB_ALG_BLOB" :
        alg == HAB_ALG_HASH ? "HAB_ALG_HASH" :
        alg == HAB_ALG_SHA1 ? "HAB_ALG_SHA1" :
        alg == HAB_ALG_SHA256 ? "HAB_ALG_SHA256" : "Un-supported alg";
}

int hab_evt_def_parser(hab_hdr_t * hdr)
{
    evt_def_t *evt_p = (evt_def_t *) hdr;

    printf("\t status: %s\t, context: %s\t, *data: 0x%x\n", get_hab_status_desc(evt_p->sts),
           get_hab_context_desc(evt_p->ctx), evt_p->data);

    return 0;
}

int hab_eng_dat_parser(hab_hdr_t * hdr)
{
    /* 1 Word is enough for the eng_dat's data field */
    printf("\tplugin flag: 0x%x\n", *((uint32_t *) hdr + 1));

    return 0;
}

int hab_key_parser(hab_hdr_t * hdr)
{
    //int len = HAB_HDR_LEN(*hdr);
    int i = 0;
    hab_eng_keystore_record_t *keystore_p = (hab_eng_keystore_record_t *) hdr;

    printf("\teng: %s, count: %d, keys: ", get_hab_eng_desc(keystore_p->eng), keystore_p->slots);
    for (i = 0; i < keystore_p->slots; i++) {
        printf("0x%x ", keystore_p->key[i]);
    }
    printf("\n");

    return 0;
}

int hab_state_parser(hab_hdr_t * hdr)
{
    state_rec_t *state_p = (state_rec_t *) hdr;

    printf("\tlib state: %s\t cmd state: %s\n", get_hab_state_lib_desc(state_p->lib),
           get_hab_state_cmd_desc(state_p->cmd));

    return 0;
}

int hab_bnd_dat_parser(hab_hdr_t * hdr)
{
    bind_storage_t *bind = (bind_storage_t *) hdr;

    printf("\t MID fuse(bank,row,bit,fuse):(%d, %d, %d, %d)", bind->mid.bank, bind->mid.row,
           bind->mid.bit, bind->mid.fuses);
    printf("\n\t data type: %s, CID: 0x%x\n", bind->data_type == HAB_DAT_CSF ? "HAB_DAT_CSF" :
           bind->data_type == HAB_DAT_IMG ? "HAB_DAT_IMG" : "Unknow data type", bind->cid);

    return 0;
}

int hab_evt_parser(hab_hdr_t * hdr)
{
    event_rec_t *event_p = (event_rec_t *) hdr;
    hab_eng_caam_evt_data_t *caam_evt_dat_p = NULL;
    hab_eng_snvs_evt_data_t *snvs_evt_dat_p = NULL;
    int len = HAB_HDR_LEN(*hdr), i;

    printf("\t status: %s, reason: %s, context: %s, engine: %s", get_hab_status_desc(event_p->sts),
           get_hab_reason_desc(event_p->rsn), get_hab_context_desc(event_p->ctx),
           get_hab_eng_desc(event_p->eng));
    printf("\n\t ");

    if (HAB_ENG_CAAM == event_p->eng) {
        caam_evt_dat_p = (hab_eng_caam_evt_data_t *) event_p->data;
        printf("\t\tsec_mem_status: 0x%x\n", BYTES_2_WORD(caam_evt_dat_p->sec_mem_status));
        printf("\t\tjob_ring_out_status: 0x%x\n",
               BYTES_2_WORD(caam_evt_dat_p->job_ring_out_status));
        printf("\t\tsec_mem_part_own: 0x%x\n", BYTES_2_WORD(caam_evt_dat_p->sec_mem_part_own));
        printf("\t\tfault_addr: 0x%x\n", BYTES_2_WORD(caam_evt_dat_p->fault_addr));
        printf("\t\tfault_addr_mid: 0x%x\n", BYTES_2_WORD(caam_evt_dat_p->fault_addr_mid));
        printf("\t\tfault_addr_detail: 0x%x\n", BYTES_2_WORD(caam_evt_dat_p->fault_addr_detail));
        printf("\t\tstatus: 0x%x\n", BYTES_2_WORD(caam_evt_dat_p->status));
    } else if (HAB_ENG_SNVS == event_p->eng) {
        snvs_evt_dat_p = (hab_eng_snvs_evt_data_t *) event_p->data;
        printf("\t\thp_vio_ctl: 0x%x\n", BYTES_2_WORD(snvs_evt_dat_p->hp_vio_ctl));
        printf("\t\thp_stat: 0x%x\n", BYTES_2_WORD(snvs_evt_dat_p->hp_stat));
        printf("\t\thp_vio_stat: 0x%x\n", BYTES_2_WORD(snvs_evt_dat_p->hp_vio_stat));
        printf("\t\tlp_ctl: 0x%x\n", BYTES_2_WORD(snvs_evt_dat_p->lp_ctl));
        printf("\t\tlp_mk_ctl: 0x%x\n", BYTES_2_WORD(snvs_evt_dat_p->lp_mk_ctl));
        printf("\t\tlp_vio_ctl: 0x%x\n", BYTES_2_WORD(snvs_evt_dat_p->lp_vio_ctl));
        printf("\t\tlp_stat: 0x%x\n", BYTES_2_WORD(snvs_evt_dat_p->lp_stat));
        printf("\t\tlp_rt_cnt_ms: 0x%x\n", BYTES_2_WORD(snvs_evt_dat_p->lp_rt_cnt_ms));
        printf("\t\tlp_rt_cnt_ls: 0x%x\n", BYTES_2_WORD(snvs_evt_dat_p->lp_rt_cnt_ls));
    } else if (HAB_ENG_OCOTP == event_p->eng) {

    } else if (HAB_ENG_SW == event_p->eng) {

    } else {
		if(len > 8){
			printf("\tload:");
			for(i = 0; i < len -8; i += 4){
				printf("\t0x%x ", BYTES_2_WORD(&event_p->data[i]));
			}
			printf("\n");
		}

    }

    return 0;
}

int hab_key_entry_parser(hab_hdr_t * hdr)
{
    hab_core_key_t *key = (hab_core_key_t *) hdr;
    uint8_t type = hdr->tag;
    uint8_t alg = hdr->par;
    uint8_t flg = key->flg;
    int len = HAB_HDR_LEN(*hdr) - sizeof(hab_core_key_t), i;
    uint32_t *val = (uint32_t *) (hdr + 1);

    printf("\ttype: %s, algorithm: %s, flag: 0x%x\n", get_hab_key_entry_type(type),
           get_hab_key_entry_alg(alg), flg);

    if ((HAB_TAG_KEY_PUBLIC == type) || (HAB_TAG_KEY_SECRET == type)) {
        printf("\tKey/cert:");
        for (i = 0; i < len / 4; i++, val++) {
            printf("\t0x%x", *val);
            if (i % 8 == 7)
                printf("\n");
        }
    }
    printf("\n");

    return 0;
}

#define HAB_TAG_END	0xFF
hab_tag_desc_t tag_desc[] = {
    {HAB_TAG_ANY, "HAB_TAG_ANY", NULL}
    ,
    {HAB_TAG_INVALID, "HAB_TAG_INVALID", NULL}
    ,
    {HAB_TAG_KEY, "HAB_TAG_KEY", hab_key_parser}
    ,
    {HAB_TAG_STATE, "HAB_TAG_STATE", hab_state_parser}
    ,
    {HAB_TAG_ALG_DEF, "HAB_TAG_ALG_DEF", NULL}
    ,
    {HAB_TAG_ENG_DAT, "HAB_TAG_ENG_DAT", hab_eng_dat_parser}
    ,
    {HAB_TAG_BND_DAT, "HAB_TAG_BND_DAT", hab_bnd_dat_parser}
    ,
    {HAB_TAG_EVT_DEF, "HAB_TAG_EVT_DEF", hab_evt_def_parser}
    ,
    {HAB_TAG_GENERIC, "HAB_TAG_GENERIC", NULL}
    ,
    {HAB_TAG_INIT, "HAB_TAG_INIT", NULL}
    ,
    {HAB_TAG_UNLK, "HAB_TAG_UNLK", NULL}
    ,
    {HAB_TAG_PLG, "HAB_TAG_PLG", NULL}
    ,

    {HAB_TAG_IVT, "HAB_TAG_IVT", NULL}
    ,
    {HAB_TAG_DCD, "HAB_TAG_DCD", NULL}
    ,
    {HAB_TAG_CSF, "HAB_TAG_CSF", NULL}
    ,
    {HAB_TAG_CRT, "HAB_TAG_CRT", NULL}
    ,
    {HAB_TAG_SIG, "HAB_TAG_SIG", NULL}
    ,
    {HAB_TAG_EVT, "HAB_TAG_EVT", hab_evt_parser}
    ,
    {HAB_TAG_RVT, "HAB_TAG_RVT", NULL}
    ,
    {HAB_TAG_WRP, "HAB_TAG_WRP", NULL}
    ,
    {HAB_TAG_MAC, "HAB_TAG_MAC", NULL}
    ,

    {HAB_TAG_KEY_PUBLIC, "HAB_TAG_KEY_PUBLIC", hab_key_entry_parser}
    ,
    {HAB_TAG_KEY_SECRET, "HAB_TAG_KEY_SECRET", hab_key_entry_parser}
    ,
    {HAB_TAG_KEY_MASTER, "HAB_TAG_KEY_MASTER", hab_key_entry_parser}
    ,
    {HAB_TAG_KEY_HASH, "HAB_TAG_KEY_HASH", hab_key_entry_parser}
    ,

    {HAB_TAG_END, NULL}
};

uint32_t get_hab_log_end(chip_e chip)
{
#define MX7D_HAB_LOG_END	0x0090553f
#define MX6_HAB_LOG_END		0x00904B7F
    if (CHIP_MX7D_TO1_0 == chip) {
        return MX7D_HAB_LOG_END;
    }else if((chip >= CHIP_MX6DQ_TO1_0) && (chip < CHIP_MX6SX_MAX)){
		return MX6_HAB_LOG_END; 
	}

    return 0;
}

hab_tag_desc_t *get_hab_tag_desc_blk(hab_hdr_t * hdr)
{
    hab_tag_desc_t *tag_desc_p = NULL;

    if (HAB_TAG_ANY != hdr->tag) {

        for (tag_desc_p = tag_desc; HAB_TAG_END != tag_desc_p->tag; tag_desc_p++) {
            if (hdr->tag == tag_desc_p->tag) {
                break;
            }
        }

        if (HAB_TAG_END == tag_desc_p->tag) {
            tag_desc_p = NULL;
        }
    }

    return tag_desc_p;
}

int hab_log_parser(uint8_t * log_addr)
{
    hab_hdr_t *hdr = NULL, *hdr_inner = NULL;
    hab_tag_desc_t *tag_desc_p = NULL;
    int len = 0;

    for (hdr = (hab_hdr_t *) log_addr;
         (uint32_t) hdr < get_hab_log_end(chip); hdr = ADVANCE_RECORD(hdr)) {
#ifdef DEBUG
        printf("hdr: 0x%x\n", (uint32_t) hdr);
#endif
        if (!IS_EMPTY(*hdr)) {
            if (NULL != (tag_desc_p = get_hab_tag_desc_blk(hdr))) {
                printf("\nRecord HDR(at 0x%x): 0x%x type: %s\n", (uint32_t) hdr,
                       *((uint32_t *) hdr),
                       tag_desc_p->description !=
                       NULL ? tag_desc_p->description : "Invalid Record");
                if (NULL != tag_desc_p->parse) {
                    tag_desc_p->parse(hdr);
                }
            }
        } else {                // Even for "empty" record, may just been released by HAB(set empty flag in HDR), 
            // but useful information may still included. 
            len = HAB_HDR_LEN(*hdr) / 4;
            for (hdr_inner = hdr + 1; hdr_inner < hdr + len;) {
                if (((HAB_HDR_PAR(*hdr_inner) == 0x42) || (HAB_HDR_PAR(*hdr_inner) == 0x41))
                    && (NULL != (tag_desc_p = get_hab_tag_desc_blk(hdr_inner)))) {

                    printf("\nRecord HDR(at 0x%x): 0x%x type: %s\n", (uint32_t) hdr_inner,
                           *((uint32_t *) hdr_inner),
                           tag_desc_p->description !=
                           NULL ? tag_desc_p->description : "Invalid Record");
                    if (NULL != tag_desc_p->parse) {
                        tag_desc_p->parse(hdr_inner);
                    }
                    hdr_inner = ADVANCE_RECORD(hdr_inner);
                } else {
                    hdr_inner++;
                }
            }
        }
    }

    return 0;
}

int do_hab_parse(unsigned int argc, char *argvs[])
{
    uint8_t *hab_log_addr = NULL;
    if (CHIP_MX7D_TO1_0 == chip) {
        hab_log_addr = (uint8_t *) 0x9049C0;
    }else if((chip >= CHIP_MX6DQ_TO1_0) && (chip < CHIP_MX6SX_MAX)){
		hab_log_addr = (uint8_t *) 0x904000;
	}

    if (NULL != hab_log_addr) {
        printf("\nParse HAB log buffer...\n\n");
        hab_log_parser(hab_log_addr);
    }

    return 0;
}

SHELL_CMD(hab_parse, do_hab_parse, "hab_parse\t\tParse Hab events\tUsage: hab_parse")
