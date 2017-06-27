#ifndef HAB_INTERNAL_H
#define HAB_INTERNAL_H
/*===========================================================================*/
/**
    @file    hab_internal.h

    @brief   Constants, macros and types common to internal subsystems

@verbatim
===============================================================================

              Freescale Semiconductor Confidential Proprietary
    (c) Freescale Semiconductor, Inc. 2008, 2009, 2010. All rights reserved.

Presence of a copyright notice is not an acknowledgement of
publication. This software file listing contains information of
Freescale Semiconductor, Inc. that is of a confidential and
proprietary nature and any viewing or use of this file is prohibited
without specific written permission from Freescale Semiconductor, Inc.

===============================================================================
Revision History:

                    Modification Date   Tracking
Author                (dd-mmm-yyyy)      Number    Description of Changes
-------------------   -------------    ----------  -----------------------
David Hartley          25-Feb-2008     ENGR55511   Initial version
Miriam R Boudreaux     23-Jun-2008     ENGR55511   Add hab_state_lib
                                                   and hab_state_cmd
Rod Ziolkowski         14-Sep-2008     ENGR88939   Restore PKCS#1 and SHA-256
Miriam R Boudreaux     10-Oct-2008     ENGR93045   Add logging contexts and 
                                                   HAB_HANDLER_TAG() macro
                                                   Add parameter name in
                                                   hab_handler.check() function
                                                   declaration and parentheses 
                                                   around definition of
                                                   HAB_QUERY_SET_HDL_TAG()
                                                   Macro. Change macro name
                                                   HAB_BASE_ADDRESS_WORDS() to
                                                   HAB_ADDRESS_WORDS()
David Hartley          22-May-2009    ENGR112666   Store default event data
David Hartley          16-Jun-2009    ENGR110871   ROM size reduction
David Hartley          04-Aug-2009    ENGR111476   Byte array conversion
David Hartley          19-Oct-2009    ENGR116111   Add SRTC support
Rod Ziolkowski         20-Jul-2010    ENGR120923   Add CAAM/SNVS support
David Hartley          10-Sep-2010    ENGR120914   Add encrypted boot support

===============================================================================
Portability: 

These definitions are customised for 32 bit cores of either endianness.

===============================================================================
@endverbatim */

#ifdef __cplusplus
extern "C" {
#endif

#define ptrdiff_t uint32_t 

/*=============================================================================
                            INCLUDE FILES
=============================================================================*/

#include "hab_types.h"          /* for external types */

/*=============================================================================
                              CONSTANTS
=============================================================================*/

/** @name Data structure tags
 *
 * Extends the list of @ref struct tags for internal use data structures.
 */
/*@{*/
#define HAB_TAG_ANY        0x00 /**< Wildcard - match any structure */
#define HAB_TAG_INVALID    0x01 /**< Invalid tag - breaks Hamming distance */
#define HAB_TAG_KEY        0x03 /**< Key record */
#define HAB_TAG_STATE      0x05 /**< State record */
#define HAB_TAG_ALG_DEF    0x06 /**< Algorithm defaults */
#define HAB_TAG_ENG_DAT    0x09 /**< Engine data */
#define HAB_TAG_BND_DAT    0x0a /**< Binding data */
#define HAB_TAG_EVT_DEF    0x0c /**< Event defaults */
#define HAB_TAG_GENERIC    0x0f /**< Generic data */
#define HAB_TAG_INIT       0x11 /**< Initialization record */
#define HAB_TAG_UNLK       0x12 /**< Unlock record */
#define HAB_TAG_PLG        0xde /**< Plugin */

/* Values 00 ... 7e reserved for internal use.
 *
 * Available values: 14, 17, 18, 1b, 1d, 1e, 21, 22, 24, 27,
 * 28, 2b, 2d, 2e, 30, 33, 35, 36, 39, 3a, 3c, 3f, 41, 42, 44, 47, 48, 4b, 4d,
 * 4e, 50, 53, 55, 56, 59, 5a, 5c, 5f, 60, 63, 65, 66, 69, 6a, 6c, 6f, 71, 72,
 * 74, 77, 78, 7b, 7d, 7e
 */
/*@}*/

/** @name Plugin types */
/*@{*/
#define HAB_PLG_TYPE_ANY 0x00   /**< Wildcard - match any plugin type */
#define HAB_PLG_TYPE_CMD 0xcc   /**< Command plugins */
#define HAB_PLG_TYPE_PCL 0xdd   /**< Protocol plugins */
#define HAB_PLG_TYPE_ENG 0xee   /**< Engine plugins */
/*@}*/

/** @name Handler types
 *
 * Only the wildcard handler type is defined here; other handler types are
 * defined in the command, protocol and engine layer interfaces.  To avoid
 * clashes, the following inclusive value ranges are reserved:
 * - command layer:  0x10 .. 0x2f
 * - protocol layer: 0x30 .. 0x4f
 * - engine layer:   0x50 .. 0x7f
 * - external use:   0x80 .. 0xff
 */
/*@{*/
#define HAB_HDL_TYPE_ANY 0x00   /**< Wildcard - match any handler type */
/*@}*/

/** @name Finite field algorithms */
/*@{*/
#define HAB_ALG_FP       0x30   /**< Odd prime finite fields  */
#define HAB_ALG_F2M      0x33   /**< Binary finite fields */
/* Available values: 35, 36, 39, 3a, 3c, 3f
 */
/*@}*/

#ifdef HAB_FUTURE
/** @name Elliptic curve algorithms
 * @anchor ec_alg
 */
/*@{*/
#define HAB_ALG_ECFP     0x41   /**< Elliptic curves over prime fields */
#define HAB_ALG_ECF2M    0x42   /**< Elliptic curves over binary fields */
/* Available values: 44, 47, 48, 4b, 4d, 4e
 */
/*@}*/

/** @name Cipher algorithms */
/*@{*/
#define HAB_ALG_NULL     0x50   /**< NULL cipher algorithm ID */
#define HAB_ALG_DES      0x53   /**< DES cipher algorithm ID */
/* Available values: 56, 59, 5a, 5c, 5f
 */
/*@}*/
#endif

/** @name Logging contexts
 *
 * Extends the list of #hab_context_t for internal logging use.
 */
/*@{*/
#define HAB_CTX_API   0x41      /**< API layer internal logging */
#define HAB_CTX_CORE  0x42      /**< Core subsystem internal logging */
#define HAB_CTX_OBJ   0x44      /**< Object layer internal logging */
#define HAB_CTX_CMD   0x47      /**< Command layer internal logging */
#define HAB_CTX_PCL   0x48      /**< Protocol layer internal logging */
#define HAB_CTX_ENG   0x4b      /**< Engine layer internal logging */
/* Values 40 ... 5f reserved for internal use.
 *
 * Available values: 4d, 4e, 50, 53, 55, 56, 59, 5a, 5c, 5f
 */
/*@}*/

/** @name Architecture-independent constants
 *
 * For the sake of clarity, HAB uses the following on all architectures:
 *
 * - "byte" means 8 bits and is usually contained in a uint8_t variable
 * - "word" means 32 bits and is usually contained in a uint32_t variable
 *
 * Note that these must be literal constants (not sizeof() expressions) since
 * they may be used in preprocessor expressions.
 */
/*@{*/

/** Conversion power between bytes and bits */
#define HAB_BYTE_SHIFT           3

/** Byte size in bits: 1 << HAB_BYTE_SHIFT */
#define HAB_BITS_IN_BYTE         8

/** Conversion power between words and bits */
#define HAB_WORD_SHIFT           5

/** Words size in bits: 1 << HAB_WORD_SHIFT */
#define HAB_BITS_IN_WORD         32

/** Word size in bytes */
#define HAB_BYTES_IN_WORD        4

/*@}*/

/** @name Architecture-specific constants
 *
 * Note that these must be literal constants (not sizeof() expressions) since
 * they are used in preprocessor expressions.
 *
 * The values given here are specific to 32-bit ARM processors with 32-bit
 * addresses.
 *
 * When different architectures are encountered, these constants should be
 * moved to an architecture-specific include file so that the correct values
 * are obtained by adjusting the include path.
 */
/*@{*/

/**  Size of an address field - sizeof(void*) */
#define HAB_ADDRESS_BYTES    4
    
/** Size of an offset field - sizeof(ptrdiff_t) */
#define HAB_OFFSET_BYTES     4
    
/** Size of a size field - sizeof(size_t) */
#define HAB_SIZE_BYTES       4
    
/*@}*/

/*=============================================================================
                                MACROS
=============================================================================*/

/** @name Plugin macros */
/*@{*/

/** Initialise plugin
 *@hideinitializer
 *
 * This macro is intended for use as part of an initialiser for extended
 * plugins.
 *
 * @param [in] plg_type  plugin type
 *
 * @param [in] table  handler table
 *
 * @pre None.
 *
 * @post None.
 *
 * @returns base plugin initialiser
 */
#define HAB_PLUGIN_INIT(plg_type, table)                               \
    {HAB_HDR(HAB_TAG_PLG, sizeof(hab_plugin_t), HAB_VERSION),        \
            (plg_type), HAB_ENTRIES_IN(table), (table)}

/*@}*/

/** @name Handler macros */
/*@{*/

/** Extend base handler
 *@hideinitializer
 *
 * Extend a base handler to an extended handler type.
 *
 * This macro is intended to be used to define similar specific macros for
 * each plugin layer.  The specific macros would be used by other components
 * within or using that layer.
 *
 * @param [in] tgt The target handler type (a C type)
 * @param [in] hdl_base  The base handler location
 *
 * @pre The base handler must be the first entry in the extended handler.
 *
 * @pre The base handler #hab_handler_t.hdl_type field must match the target
 * handler type.
 *
 * @post None.
 *
 * @returns The extended handler location
 *
 * @see HAB_HANDLER_BASE()
 */
#define HAB_HANDLER_EXTEND(tgt, hdl_base)       \
    ((tgt*)(hdl_base))

/**  Get base handler
 *@hideinitializer
 *
 * Locate the base handler within an extended handler.
 *
 * @param [in] hdl_ext  The extended handler location
 *
 * @pre The base handler must be the first entry in the extended handler.
 *
 * @post None.
 *
 * @returns The base handler location
 *
 * @see HAB_HANDLER_EXTEND()
 */
#define HAB_HANDLER_BASE(hdl_ext)       \
    ((hab_handler_t*)(hdl_ext))

/**  Get handler tag
 *@hideinitializer
 *
 * Locate the handler tag within an extended handler.
 *
 * @param [in] hdl_ext  The extended handler location
 *
 * @pre The base handler must be the first entry in the extended handler.
 *
 * @post None.
 *
 * @returns The base handler tag
 *
 * @see HAB_HANDLER_EXTEND()
 */
#define HAB_HANDLER_TAG(hdl_ext) \
      (HAB_HANDLER_BASE(hdl_ext)->hdl_tag)

/*@}*/

/** @name Query macros */
/*@{*/

/** Initialise query
 *@hideinitializer
 *
 * Constructs initialiser for standard query with wildcard handler tag.
 *
 * @param [in] plg_type  plugin type
 *
 * @param [in] hdl_type  handler type
 *
 * @pre None.
 *
 * @post None.
 *
 * @remark Only a @a standard query is initialised by this macro: the extended
 * query fields are set to NULL and zero as appropriate.  To create an
 * extended query, initialise a standard query and then use the macro
 * HAB_QUERY_SET_DATA().
 *
 * @remark Use HAB_QUERY_SET_HDL_TAG() to change handler tag.
 *
 * @see Specialised query initialisers for each plugin type.
 *
 * @returns query initialiser
 */
#define HAB_QUERY_INIT(plg_type, hdl_type)              \
    {(plg_type), (hdl_type), HAB_TAG_ANY, NULL, 0}

/** Set query plugin type
 *@hideinitializer
 *
 * @param [in,out] qry_ptr  Query location
 *
 * @param [in] type  plugin type
 *
 * @pre @a qry_ptr locates a writable #hab_query_t variable.
 *
 * @post @a qry_ptr fields are updated.
 */
#define HAB_QUERY_SET_PLG_TYPE(qry_ptr, type)   \
    ((qry_ptr)->plg_type = (type))

/** Set query handler type
 *@hideinitializer
 *
 * @param [in,out] qry_ptr  Query location
 *
 * @param [in] type  handler type
 *
 * @pre @a qry_ptr locates a writable #hab_query_t variable.
 *
 * @post @a qry_ptr fields are updated.
 */
#define HAB_QUERY_SET_HDL_TYPE(qry_ptr, type)   \
    ((qry_ptr)->hdl_type = (type))

/** Set query handler tag
 *@hideinitializer
 *
 * @param [in,out] qry_ptr  Query location
 *
 * @param [in] tag  handler tag
 *
 * @pre @a qry_ptr locates a writable #hab_query_t variable.
 *
 * @post @a qry_ptr fields are updated.
 */
#define HAB_QUERY_SET_HDL_TAG(qry_ptr, tag)     \
    ((qry_ptr)->hdl_tag = (tag))

/** Set query data
 *@hideinitializer
 *
 * @param [in,out] qry_ptr  Query location
 *
 * @param [in] data_ptr Additional data location 
 *
 * @param [in] bytes Additional data size
 *
 * @pre @a qry_ptr locates a writable #hab_query_t variable.
 *
 * @post @a qry_ptr fields are updated.
 */
#define HAB_QUERY_SET_DATA(qry_ptr, data_ptr, bytes)                    \
    ((qry_ptr)->data = (data_ptr), (qry_ptr)->data_bytes = (bytes))

/*@}*/

/** @name Macros dealing with bitfields in uint32_t
 *
 * These macros rely on constants named in a certain way:
 * - LBL_WIDTH gives the number of bits in the field
 * - LBL_SHIFT gives the number of bits to the right of the field.
 *
 * where "LBL" is a freely-chosen label used as in input to the macros. For
 * example, two defines
 *
@verbatim
    #define XYZ_WIDTH 3
    #define XYZ_SHIFT 4
@endverbatim
 *
 * can then be used to manipulate a bit field called XYZ occupying bits [6:4]
 * in a word as in
 *
@verbatim
    HAB_INSERT_BITS(5, XYZ)
    HAB_CLEAR_BITS(1, XYZ)
@endverbatim
 */
/*@{*/

/** Create a mask for a bitfield label
 *@hideinitializer
 *
 * This pre-processor macro generates a bit mask for the labelled bitfield, in
 * which bits within the bitfield are set to 1 while other bits are cleared to
 * zero.
 *
 * @param[in] LBL   Label for the bitfield constants
 *
 * @pre Constants LBL_WIDTH and LBL_SHIFT are defined, with 0 < LBL_WIDTH < 32
 * and 0 <= LBL_SHIFT < 32.
 *
 * @return Bit mask
 */
#define HAB_MASK(LBL)                                           \
    ((((uint32_t)1 << (LBL##_WIDTH)) - 1) << (LBL##_SHIFT))

/** Position a value in a bitfield
 *@hideinitializer
 *
 * This pre-processor macro shifts and masks the given value into the correct
 * position for the labelled bitfield.
 *
 * @param[in] val   Value to write to the bitfield
 *
 * @param[in] LBL   Label for the bitfield constants
 *
 * @pre Constants LBL_WIDTH and LBL_SHIFT are defined, with 0 < LBL_WIDTH < 32
 * and 0 <= LBL_SHIFT < 32.
 *
 * @return Shifted and masked value
 */
#define HAB_INSERT_BITS(val, LBL)               \
    (((uint32_t)(val) << LBL##_SHIFT) & HAB_MASK(LBL))

/** Select a bitfield within a variable
 *@hideinitializer
 *
 * This pre-processor masks out all bits within the given variable other than
 * the labelled bitfield. The input variable is not affected.
 *
 * @param[in] var   Variable containing the bitfield
 *
 * @param[in] LBL   Label for the bitfield constants
 *
 * @pre Constants LBL_WIDTH and LBL_SHIFT are defined, with 0 < LBL_WIDTH < 32
 * and 0 <= LBL_SHIFT < 32.
 *
 * @return Masked value
 */
#define HAB_SELECT_BITS(var, LBL)               \
    ((var) & HAB_MASK(LBL))

/** Clear a bitfield within a variable
 *@hideinitializer
 *
 * This pre-processor clears the labelled bitfield in the given variable,
 * leaving other bits unchanged. The input variable is not affected.
 *
 * @param[in] var   Variable containing the bitfield
 *
 * @param[in] LBL   Label for the bitfield constants
 *
 * @pre Constants LBL_WIDTH and LBL_SHIFT are defined, with 0 < LBL_WIDTH < 32
 * and 0 <= LBL_SHIFT < 32.
 *
 * @return Masked value
 */
#define HAB_CLEAR_BITS(var, LBL)                \
    ((var) & ~HAB_MASK(LBL))

/** Extract a bitfield from a variable
 *@hideinitializer
 *
 * This pre-processor masks and shifts the labelled bitfield from the given
 * variable. The input variable is not affected.
 *
 * @param[in] var   Variable containing the bitfield
 *
 * @param[in] LBL   Label for the bitfield constants
 *
 * @pre Constants LBL_WIDTH and LBL_SHIFT are defined, with 0 < LBL_WIDTH < 32
 * and 0 <= LBL_SHIFT < 32.
 *
 * @return Bitfield value shifted to least-significant position
 */
#define HAB_EXTRACT_BITS(var, LBL)                      \
    (HAB_SELECT_BITS((var), LBL) >> LBL##_SHIFT)

/** Match a bitfield from a variable with a value
 *@hideinitializer
 *
 * This pre-processor macro compares the given value with the labelled
 * bitfield within the given variable.
 *
 * @param[in] var   Variable containing the bitfield
 *
 * @param[in] val   Value to match
 *
 * @param[in] LBL   Label for the bitfield constants
 *
 * @pre Constants LBL_WIDTH and LBL_SHIFT are defined, with 0 < LBL_WIDTH < 32
 * and 0 <= LBL_SHIFT < 32.
 *
 * @return Boolean result of comparison
 */
#define HAB_MATCH_BITS(var, val, LBL)                                   \
    (HAB_SELECT_BITS((var), LBL) == HAB_INSERT_BITS((val), LBL))

/** Replace bits
 *
 * Replaces labelled bitfield in a given variable with a given value.  The
 * input variable is unchanged.
 *
 * @param[in] var   Variable containing the bitfield
 *
 * @param[in] val   Value to write over the bitfield
 *
 * @param[in] LBL   Label for the bitfield constants
 *
 * @remark No parentheses on @a LBL in macro definition because it must be
 * used with ## preprocessor operator by the underlying macros called.
 *
 * @returns Value from @a var with bits in field @a LBL replaced by value from
 * @a val.
 */
#define HAB_REPLACE_BITS(var, val, LBL)                                 \
    ( HAB_CLEAR_BITS((var), LBL) | HAB_INSERT_BITS((val), LBL) )

/*@}*/

/** @name Header field macros */
/*@{*/

/** Construct Header field
 *@hideinitializer
 *
 * @param[in] tag   Tag field value
 *
 * @param[in] len   Length field value
 *
 * @param[in] par   Parameter field value
 *
 * @return @ref hdr
 */
#define HAB_HDR(tag, len, par)                          \
    {(tag), {(len)>>HAB_BITS_IN_BYTE, (len)}, (par)}

/** Write header field
 *@hideinitializer
 *
 * @param[out] ptr   Address to write header
 *
 * @param[in] tag   Tag field value
 *
 * @param[in] len   Length field value
 *
 * @param[in] par   Parameter field value
 *
 * @return @ref hdr
 */
#define HAB_SET_HDR(ptr, tag, len, par)         \
    hab_set_hdr((ptr), (tag), (len), (par))

/** Get Header tag
 *@hideinitializer
 *
 * @param[in] hdr   @ref hdr value
 *
 * @return Tag field
 */
#define HAB_HDR_TAG(hdr)                        \
    (((const hab_hdr_t*)&(hdr))->tag)

/** Get Header length
 *@hideinitializer
 *
 * Combines the length bytes to give the length value
 *
 * @param[in] hdr   @ref hdr value
 *
 * @return Length value as a size_t
 */
#define HAB_HDR_LEN(hdr)                                              \
    ((size_t)(((const hab_hdr_t*)&(hdr))->len[0] << HAB_BITS_IN_BYTE) \
     + (size_t)((const hab_hdr_t*)&(hdr))->len[1])

/** Get Header parameter field
 *@hideinitializer
 *
 * @param[in] hdr   @ref hdr value
 *
 * @return Parameter field
 */
#define HAB_HDR_PAR(hdr)                        \
    (((const hab_hdr_t*)&(hdr))->par)

/*@}*/

/** @name HAB version macros */
/*@{*/

/** Construct full version 
 *@hideinitializer
 *
 * @param[in] maj   Major version
 *
 * @param[in] min   Minor version
 *
 * @return Full version
 */
#define HAB_VER(maj, min)                       \
    ( HAB_INSERT_BITS(maj, HAB_VER_MAJ)         \
      | HAB_INSERT_BITS(min, HAB_VER_MIN) )

/** Get major version
 *@hideinitializer
 *
 * @param[in] ver   full version value
 *
 * @return Major version field (right-shifted)
 */
#define HAB_VER_MAJ(ver)                        \
    (HAB_EXTRACT_BITS(ver, HAB_VER_MAJ))

/** Get minor version
 *@hideinitializer
 *
 * @param[in] ver   full version value
 *
 * @return Minor version field (right-shifted)
 */
#define HAB_VER_MIN(ver)                        \
    (HAB_EXTRACT_BITS(ver, HAB_VER_MIN))

/*@}*/

/** @name Address/offset/size field macros */
/*@{*/

/** Convert a big-endian address field to a processor specific pointer
 *@hideinitializer
 *
 * @param [in] field   Location of big-endian address field
 *
 * @pre @a field is not NULL
 *
 * @post None
 *
 * @returns pointer value
 */
#if HAB_ADDRESS_BYTES == 4
#define HAB_UNPACK_ADDRESS(field)               \
    (void*)HAB_UNPACK_32_FROM_8(field)
#else
#define HAB_UNPACK_ADDRESS(field)               \
    hab_unpack_address(field)
#endif

/** Convert a big-endian offset field to a processor specific offset variable
 *@hideinitializer
 *
 * @param [in] field   Location of big-endian offset field
 *
 * @pre @a field is not NULL
 *
 * @post None
 *
 * @returns offset value
 */
#if HAB_ADDRESS_BYTES == 4
#define HAB_UNPACK_OFFSET(field)                \
    (ptrdiff_t)HAB_UNPACK_32_FROM_8(field)
#else
#define HAB_UNPACK_OFFSET(field)                \
    hab_unpack_offset(field)
#endif

/** Convert a big-endian size field to a processor specific size variable
 *@hideinitializer
 *
 * @param [in] field   Location of big-endian size field
 *
 * @pre @a field is not NULL
 *
 * @post None
 */
#if HAB_ADDRESS_BYTES == 4
#define HAB_UNPACK_SIZE(field)                  \
    (size_t)HAB_UNPACK_32_FROM_8(field)
#else
#define HAB_UNPACK_SIZE(field)                  \
    hab_unpack_size(field)
#endif

/** Convert a processor specific pointer to a big-endian address field
 *@hideinitializer
 *
 * @param [out] field   Location of big-endian address field
 *
 * @param [in] ptr     Input pointer
 *
 * @pre @a field is not NULL
 *
 * @post None
 */
#if HAB_ADDRESS_BYTES == 4
#define HAB_PACK_ADDRESS(field, ptr)                            \
    HAB_PACK_32_TO_8((uint8_t*)(field), (uint32_t)(ptr))
#else
#define HAB_PACK_ADDRESS(field, ptr)            \
    hab_pack_address(field, ptr)
#endif

/** Convert a processor specific offset variable to a big-endian offset field
 *@hideinitializer
 *
 * @param [out] field   Location of big-endian offset field
 *
 * @param [in] off     Input offset variable
 *
 * @pre @a field is not NULL
 *
 * @post None
 */
#if HAB_ADDRESS_BYTES == 4
#define HAB_PACK_OFFSET(field, off)                                     \
    HAB_PACK_32_TO_8((uint8_t*)(field), (uint32_t)(off))
#else
#define HAB_PACK_OFFSET(field, off)                                     \
    hab_pack_offset(field, off)
#endif

/** Convert a processor specific size variable to a big-endian size field
 *@hideinitializer
 *
 * @param [out] field   Location of big-endian size field
 *
 * @param [in] bytes     Input size variable
 *
 * @pre @a field is not NULL
 *
 * @post None
 */
#if HAB_ADDRESS_BYTES == 4
#define HAB_PACK_SIZE(field, bytes)                                     \
    HAB_PACK_32_TO_8((uint8_t*)(field), (uint32_t)(bytes))
#else
#define HAB_PACK_SIZE(field, bytes)                                     \
    hab_pack_size(field, bytes)
#endif

/*@}*/

/** @name Miscellaneous macros */
/*@{*/

/** Unpack single unsigned 32-bit word from big-endian byte array
 *
 * @param[in] p Start of byte array
 *
 * @returns unsigned word value
 */
#define HAB_UNPACK_32_FROM_8(p)                                         \
    ((uint32_t)( (((uint8_t*)(p))[0] << 3*HAB_BITS_IN_BYTE)             \
                 | (((uint8_t*)(p))[1] << 2*HAB_BITS_IN_BYTE)           \
                 | (((uint8_t*)(p))[2] << HAB_BITS_IN_BYTE)             \
                 |  ((uint8_t*)(p))[3] ) )
    
/** Pack single unsigned 32-bit word to big-endian byte array
 *
 * @param[in] b Start of byte array
 *
 * @param[in] w unsigned word value
 */
#define HAB_PACK_32_TO_8(b, w)                                          \
    (((uint8_t*)(b))[0] = (uint8_t)((w) >> 3*HAB_BITS_IN_BYTE),         \
     ((uint8_t*)(b))[1] = (uint8_t)((w) >> 2*HAB_BITS_IN_BYTE),         \
     ((uint8_t*)(b))[2] = (uint8_t)((w) >> HAB_BITS_IN_BYTE),           \
     ((uint8_t*)(b))[3] = (uint8_t)(w))

/** Pack single unsigned 16-bit value to big-endian byte array
 *
 * @param[in] b Start of byte array
 *
 * @param[in] w unsigned 16-bit value
 */
#define HAB_PACK_16_TO_8(b, w)                                          \
    (((uint8_t*)(b))[0] = (uint8_t)((w) >> HAB_BITS_IN_BYTE),           \
     ((uint8_t*)(b))[1] = (uint8_t)(w))

/** Compute space required for bit string
 *
 * @param[in] bits Number of bits to store
 *
 * @returns Number of bytes required
 */
#define HAB_BYTES_FOR_BITS(bits)                        \
    (((bits) + HAB_BITS_IN_BYTE - 1) >> HAB_BYTE_SHIFT)

/** Compute space required for bit string
 *
 * @param[in] bits Number of bits to store
 *
 * @returns Number of words required
 */
#define HAB_WORDS_FOR_BITS(bits)                        \
    (((bits) + HAB_BITS_IN_WORD - 1) >> HAB_WORD_SHIFT)

/** Compute space required for byte string
 *
 * @param[in] bytes Number of bytes to store
 *
 * @returns Number of words required
 */
#define HAB_WORDS_FOR_BYTES(bytes)                                      \
    (((bytes) + HAB_BYTES_IN_WORD - 1) >> (HAB_WORD_SHIFT - HAB_BYTE_SHIFT))

/** Compute space required for word string
 *
 * @param[in] words Number of words to store
 *
 * @pre Assumes 4-byte words
 *
 * @returns Number of bytes required
 */
#define HAB_BYTES_FOR_WORDS(words)                      \
    ((words) << (HAB_WORD_SHIFT - HAB_BYTE_SHIFT))

/** Break on condition
 *
 * @param [in] c   boolean condition
 *
 * @pre None.
 *
 * @post None.
 *
 * @remark This macro may be used only within a loop where a statement is
 * expected.
 */
#define HAB_BREAK_IF(c)                         \
    if (c) { break; }

/** Break unless condition met
 *
 * @param [in] c   boolean condition
 *
 * @pre None.
 *
 * @post None.
 *
 * @remark This macro may be used only within a loop where a statement is
 * expected.
 */
#define HAB_BREAK_UNLESS(c)                     \
    if (!(c)) { break; }

/** Continue unless condition met
 *
 * @param [in] c   boolean condition
 *
 * @pre None.
 *
 * @post None.
 *
 * @remark This macro may be used only within a loop where a statement is
 * expected.
 */
#define HAB_CONTINUE_UNLESS(c)                  \
    if (!(c)) { continue; }

/** Continue on condition
 *
 * @param [in] c   boolean condition
 *
 * @pre None.
 *
 * @post None.
 *
 * @remark This macro may be used only within a loop where a statement is
 * expected.
 */
#define HAB_CONTINUE_IF(c)                      \
    if (c) { continue; }

/** Suppress compiler warning about unused parameter
 *
 * @param [in] p   unused parameter
 *
 * @remark This macro convinces many compilers that a parameter is used, yet
 * is optimised away to have no effect on the final code size.
 */
#define HAB_SUPPRESS_UNUSED_WARNING(p)          \
    ((p) = (p))

/*@}*/

/*=============================================================================
                                ENUMS
=============================================================================*/

/** HAB Library State
 */
typedef enum hab_state_lib {
        HAB_STATE_LIB_RESET = 0x0,   /**< HAB state after reset. Self tests
                                        called by rvt.entry() have not been
                                        run, the self tests need only be run
                                        once. */
        HAB_STATE_LIB_INITIAL = 0x1, /**< rvt.entry() has been called for the
                                        first time after reset and before 
                                        rvt.exit() has been called. HAB APIs
                                        will check the state condition prior
                                        to executing. */
        HAB_STATE_LIB_INNER = 0x2,   /**< rvt.entry() has been called after 
                                        rvt.exit() has been previously run.
                                        HAB APIs will check the state condition
                                        prior to executing. */
        HAB_STATE_LIB_OUTER = 0x4    /**< rvt.exit() has been called, on
                                        subsequent calls to rvt.entry() the
                                        self tests will not be executed. */
} hab_state_lib_t;

/** HAB Command State
 */
typedef enum hab_state_cmd {
        HAB_STATE_CMD_UNAUTH  = 0x0, /**< Unauth - Processing DCD cmds */
        HAB_STATE_CMD_PREAUTH = 0x2, /**< PreAuth - Processing CSF cmds and
                                                prior to CSF authentication */
        HAB_STATE_CMD_AUTH    = 0x4  /**< Auth- Processing CSF cmds after 
                                                CSF authentication */
} hab_state_cmd_t;

/*=============================================================================
                    STRUCTURES AND OTHER TYPEDEFS
=============================================================================*/

/** Tag */
typedef uint32_t hab_tag_t;

/** Plugin type */
typedef uint32_t hab_plugin_type_t;

/** Handler type */
typedef uint32_t hab_handler_type_t;

/** Read-write memory buffer
 *
 * @see #hab_block and #hab_hal_region for alternative representations.
 */
typedef struct hab_buffer {
    /** Base address. */
    uint8_t* base;
    /** Size. */
    size_t bytes;
} hab_buffer_t;

/** Read-only memory buffer
 *
 * @see #hab_buffer.
 */
typedef struct hab_const_buffer {
    /** Base address. */
    const uint8_t* base;
    /** Size. */
    size_t bytes;
} hab_const_buffer_t;


/** Memory block
 *
 * @see #hab_hal_region for an alternative representation.
 */
typedef struct hab_block {
    /** Base address. */
    const void* base;
    /** Size. */
    size_t bytes;
} hab_block_t;

/** Memory block list */
typedef struct hab_block_list {
    /** Block count. */
    uint32_t count;
    /** Block array. */
    hab_block_t* blocks;
} hab_block_list_t;

/** Query structure
 *
 * This structure encapsulates information required by the plugin manager to
 * find a handler.  Queries are divided into two kinds:
 *
 * - @a standard queries in which only the plugin type, handler type and
 * handler tag are used
 * - @a extended queries in which additional query-specific data is checked by
 * any candidate plugin to verify its capability to handle the operations
 * which follow
 *
 * @remark The values #HAB_PLG_TYPE_ANY, #HAB_HDL_TYPE_ANY, and #HAB_TAG_ANY
 * may be used as wildcards in the query.
 *
 * @warning If @a data is not NULL, then its interpretation is completely
 * determined by the information contained in @a query.
 *
 * @see #hab_core_plugin_find.
 */
typedef struct hab_query {
    /** Required plugin type */
    hab_plugin_type_t plg_type;
    /** Required handler type */
    hab_handler_type_t hdl_type;
    /** Required handler tag */
    hab_tag_t hdl_tag;
    /** Additional query-specific data (NULL for a standard query) */
    const void* data;
    /** Size of @a data, if present (zero for a standard query) */
    size_t data_bytes;
} hab_query_t;

/** Handler base structure
 *
 * A handler structure is the interface used to execute a particular command,
 * protocol or engine operation.  Handlers are grouped within command,
 * protocol and engine plugins, and may be discovered by querying the plugin
 * manager.  Different handler types are defined for each plugin type.
 *
 * The handler base structure contains the fields common to all handler types.
 *
 * @see Command handler types in hab_cmd.h, protocol handler types in
 * hab_pcl.h, engine handler types in hab_eng.h, and hab_core_plugin_find().
 */
typedef struct hab_handler {
    /** Handler type */
    hab_handler_type_t hdl_type;
    /** Handler tag */
    hab_tag_t hdl_tag;
    /** Handler check function (optional) 
     *
     * Perform additional handler-specific checks to confirm that a handler
     * will be able to support an extended query. 
     *
     * @param [in] query Specification of task requiring a handler
     *
     * @pre @a query is not NULL
     *
     * @post None
     *
     * @remark This function does not assume that @a query is an extended
     * query.
     *
     * @retval HAB_SUCCESS if @a query is a standard query.
     *
     * @retval HAB_SUCCESS if @a query is an extended query and the handler is
     * compatible with the query-specific data.
     *
     * @retval HAB_FAILURE otherwise
     */
    hab_status_t (*check)(const hab_query_t* query);
} hab_handler_t;

/** ROM address of handler */
typedef const hab_handler_t * hab_handler_ptr_t;

/** Plugin structure
 *
 * A plugin is a collection of handlers which are provided as a group for
 * possible inclusion in an IC ROM build.  Typically, a plugin relates to a
 * particular HW peripheral, protocol or group of commands supported on the
 * IC.  Since the set of plugins available on a given IC depends on
 * integration choices, plugins and their handlers are not accessed directly.
 * Instead, they may be discovered by querying the plugin manager for required
 * capabilities.
 *
 * @remark The @a check function is required only for extended queries.  If
 * all handlers provided by the plugin can be discovered using standard
 * queries, then @a check should be set to NULL.
 *
 * @see hab_core_plugin_find(), #hab_handler, #hab_query.
 */
typedef struct hab_plugin {
    /** @ref hdr with tag #HAB_TAG_PLG, length and HAB version fields
     *  (see @ref data)
     */
    hab_hdr_t hdr;
    /** Plugin type */
    hab_plugin_type_t plg_type;
    /** Handler table size */
    uint32_t hdl_tbl_entries;
    /** Handler table */
    const hab_handler_ptr_t* hdl_tbl;
} hab_plugin_t;

/*=============================================================================
                     GLOBAL VARIABLE DECLARATIONS
=============================================================================*/

/*=============================================================================
                         FUNCTION PROTOTYPES
=============================================================================*/

/*=============================================================================
                           INLINE FUNCTIONS
=============================================================================*/

/** Write header field
 *
 * @param[out] ptr   Address to write header
 *
 * @param[in] tag   Tag field value
 *
 * @param[in] len   Length field value
 *
 * @param[in] par   Parameter field value
 *
 * @return @ref hdr
 */
static inline void
hab_set_hdr(void *ptr, uint32_t tag, size_t len, uint32_t par)
{
    hab_hdr_t hdr = HAB_HDR(tag, len, par);
    *(hab_hdr_t*)ptr = hdr;
    return;
}

/** Copy between big-endian and processor-endian buffers
 *
 * Copies source to destination, reversing byte order on little-endian
 * processors.
 *
 * @param [out] dst    Destination buffer
 * @param [in]  src    Source buffer
 * @param [in]  bytes  Buffer size
 *
 * @pre @a src and @a dst are not NULL
 * @pre @a src and @a dst are same size
 * @pre @a src and @a dst buffers do not overlap
 * @pre 0 < @a bytes < 256
 */
static inline void
hab_endian_copy(void *dst, const void *src, size_t bytes)
{
    /* Output step - dereference gives non-zero on little-endian processors */
    ptrdiff_t step = (*(uint8_t*)&bytes == 0)? 1: -1;
    /* Pointer into source */
    register const uint8_t* p = src;
    /* Pointer into destination */
    register uint8_t* q = dst;

    /* Start at end of destination on little-endian processors */
    q += (step == 1)? 0: bytes - 1;

    /* Copy source to destination, byte at a time */
    while (bytes--)
    {
        *q = *p++;
        q += step;
    }
    return;
}

/** Generic version of HAB_UNPACK_ADDRESS()
 *
 * @copydoc HAB_UNPACK_ADDRESS()
 */
static inline void*
hab_unpack_address(const void *field)
{
    uint8_t* ptr;
    hab_endian_copy(&ptr, field, HAB_ADDRESS_BYTES);
    return ptr;
}

/** Generic version of HAB_UNPACK_OFFSET()
 *
 * @copydoc HAB_UNPACK_OFFSET()
 */
static inline ptrdiff_t
hab_unpack_offset(const void *field)
{
    ptrdiff_t off;
    hab_endian_copy(&off, field, HAB_OFFSET_BYTES);
    return off;
}

/** Generic version of HAB_UNPACK_SIZE()
 *
 * @copydoc HAB_UNPACK_SIZE()
 */
static inline size_t
hab_unpack_size(const void *field)
{
    size_t bytes;
    hab_endian_copy(&bytes, field, HAB_SIZE_BYTES);
    return bytes;
}

/** Generic version of HAB_PACK_ADDRESS()
 *
 * @copydoc HAB_PACK_ADDRESS()
 */
static inline void
hab_pack_address(void *field, const void * ptr)
{
    hab_endian_copy(field, &ptr, HAB_ADDRESS_BYTES);
}

/** Generic version of HAB_PACK_OFFSET()
 *
 * @copydoc HAB_PACK_OFFSET()
 */
static inline void
hab_pack_offset(void *field, ptrdiff_t off)
{
    hab_endian_copy(field, &off, HAB_OFFSET_BYTES);
}

/** Generic version of HAB_PACK_SIZE()
 *
 * @copydoc HAB_PACK_SIZE()
 */
static inline void
hab_pack_size(void *field, size_t bytes)
{
    hab_endian_copy(field, &bytes, HAB_SIZE_BYTES);
}

#ifdef __cplusplus
}
#endif

#endif /* HAB_INTERNAL_H */
