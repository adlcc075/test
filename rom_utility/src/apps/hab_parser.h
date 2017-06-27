#ifndef _HAB_PARSER_H_
#define _HAB_PARSER_H_
#include "platform.h"
#include "hab_internal.h"
#include "hab_types.h"

#define BYTES_2_WORD(x)	(*(x)) | (*((x)+1) << 8) | (*((x)+2) << 16) | (*((x)+3) << 24)

#define ALIGNMENT_MASK (HAB_BYTES_IN_WORD - 1)

#define IS_EMPTY(hdr)                           \
	    ((HAB_HDR_TAG(hdr) ==  HAB_TAG_ANY)          \
		      && ((HAB_HDR_PAR(hdr) == 0x42) || \
				  (HAB_HDR_PAR(hdr) == 0x41)))

#define WORD_MULTIPLE(bytes)                            \
	    (((bytes) + ALIGNMENT_MASK) & ~ALIGNMENT_MASK)

#define ADVANCE_RECORD(rec)                                             \
	    ((hab_hdr_t*)((uint8_t*)(rec) + WORD_MULTIPLE(HAB_HDR_LEN(*(rec)))))

#define HAB_TAG_KEY_PUBLIC	HAB_KEY_PUBLIC
#define HAB_TAG_KEY_SECRET	HAB_KEY_SECRET
#define HAB_TAG_KEY_MASTER	HAB_KEY_MASTER
#define HAB_TAG_KEY_HASH	HAB_KEY_HASH

typedef struct {
    uint8_t tag;
    char *description;
    int (*parse) (hab_hdr_t * hdr);
} hab_tag_desc_t;

/** Structure used for saving event defaults */
typedef struct evt_def {
    hab_hdr_t hdr;                      /**< Variable header @see #Header */
    uint32_t sts;                       /**< default status */
    uint32_t ctx;                       /**< default context */
    const uint8_t *data;                /**< default data location */
    size_t bytes;                       /**< size of default data */
} evt_def_t;

typedef struct hab_core_key {
    hab_hdr_t hdr;         /**< Header with key type in @a tag field and
                            *   algorithm in @a par field */
    uint8_t sbz[HAB_BYTES_IN_WORD - 1];/**< Reserved - should be zero */
    uint8_t flg;           /**< Key flags */
} hab_core_key_t;

typedef struct hab_eng_keystore_record {
    hab_hdr_t hdr;               /**< Header with @a tag #HAB_TAG_KEY
                                  *   and @a par giving key type */
    hab_tag_t eng;               /**< Associated engine (for logging) */
    uint32_t slots;              /**< Number of key record slots */
    const hab_core_key_t *key[]; /**< Key record pointers */
} hab_eng_keystore_record_t;

typedef struct state_rec {
    hab_hdr_t hdr;              /**< Record header */
    uint32_t lib;               /**< Library state variable */
    uint32_t cmd;                  /**< Command state variable */
} state_rec_t;

    /** MID fuse field location */
typedef struct hab_core_bind_mid {
    uint8_t bank;                                  /**< Fuse bank */
    uint8_t row;                                 /**< Starting row */
    uint8_t bit;                                     /**< Starting bit */
    uint8_t fuses;                                       /**< Fuse count */
} hab_core_bind_mid_t;

typedef struct bind_storage {
    hab_hdr_t hdr;                             /**< Record header */
    hab_core_bind_mid_t mid;                   /**< MID fuse location */
    uint8_t data_type;                         /**< Bound data type */
    uint8_t cid;                               /**< Bound CID */
} bind_storage_t;

            /** Event record */
typedef struct event_rec {
    hab_hdr_t hdr;                         /**< Variable header @see #Header */
    uint8_t sts;                               /**< Status */
    uint8_t rsn;                                   /**< Reason */
    uint8_t ctx;                                       /**< Context */
    uint8_t eng;                                           /**< Engine */
    uint8_t data[];                                            /**< context dependent data */
} event_rec_t;

/** CAAM event data */
typedef struct hab_eng_caam_evt_data {
    uint8_t sec_mem_status[HAB_BYTES_IN_WORD];            /**< Sec. Mem. Status */
    uint8_t job_ring_out_status[HAB_BYTES_IN_WORD];             /**< JR Output Status */
    uint8_t sec_mem_part_own[HAB_BYTES_IN_WORD];                  /**< Sec Mem. Part Owners */
    uint8_t fault_addr[HAB_BYTES_IN_WORD];                            /**< Fault Address */
    uint8_t fault_addr_mid[HAB_BYTES_IN_WORD];                            /**< Fault Address MID */
    uint8_t fault_addr_detail[HAB_BYTES_IN_WORD];                             /**< Fault Address Detail */
    uint8_t status[HAB_BYTES_IN_WORD];                                            /**< CAAM status */
} hab_eng_caam_evt_data_t;

    /** SNVS event data */
typedef struct hab_eng_snvs_evt_data {
    uint8_t hp_vio_ctl[HAB_BYTES_IN_WORD];           /**< HP Security Violation Ctrl */
    uint8_t hp_stat[HAB_BYTES_IN_WORD];                  /**< HP Status */
    uint8_t hp_vio_stat[HAB_BYTES_IN_WORD];                  /**< HP Security Violation Stat */
    uint8_t lp_ctl[HAB_BYTES_IN_WORD];                           /**< LP Control */
    uint8_t lp_mk_ctl[HAB_BYTES_IN_WORD];                            /**< LP Master Key Ctrl */
    uint8_t lp_vio_ctl[HAB_BYTES_IN_WORD];                               /**< LP Security Violation Ctrl */
    uint8_t lp_stat[HAB_BYTES_IN_WORD];                                      /**< LP Status */
    uint8_t lp_rt_cnt_ms[HAB_BYTES_IN_WORD];                                     /**< LP Real Time Counter MSB */
    uint8_t lp_rt_cnt_ls[HAB_BYTES_IN_WORD];                                         /**< LP Real Time Counter LSB */
} hab_eng_snvs_evt_data_t;

int hab_parser(chip_e chip);

#endif
