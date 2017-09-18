/*
 * def.h
 *
 *  Created on: Sep 12, 2017
 *      Author: shjeong
 */

#ifndef DEF_H_
#define DEF_H_

#define uint8	unsigned char
#define uint16	unsigned short
#define uint32	unsigned int
#define uint64	unsigned long long

#define int8	char
#define int16	short
#define int32	int
#define int64	long long

#define SYS_PATH				"/ssd"
#define SYS_BIN_PATH			SYS_PATH "/bin"
#define SYS_LIB_PATH			SYS_PATH "/lib"
#define SYS_INC_PATH			SYS_PATH "/include"
#define SYS_CFG_PATH			SYS_PATH "/config"
#define SYS_ATH_PATH			SYS_PATH "/athost"
#define SYS_SCR_PATH			SYS_PATH "/script"
#define SYS_SHE_PATH			SYS_PATH "/shell"
#define SYS_SHA_PATH			SYS_PATH "/share"
#define SYS_TMP_PATH			SYS_PATH "/temp"
#define SYS_IMG_PATH			SYS_PATH "/images"
#define SYS_SRC_PATH			SYS_PATH "/src"
#define SYS_DATA_PATH 			SYS_PATH "/data"
#define SYS_FIRM_PATH 			SYS_PATH "/firm"
#define SYS_UTIL_PATH 			SYS_PATH "/util"
#define SYS_DIAG_PATH 			SYS_PATH "/diag"
#define SYS_DEBUG_PATH 			SYS_PATH "/debug"

#define KEY_MSGQ_IN		0x1000
#define KEY_MSGQ_OUT	0x1001

#endif /* DEF_H_ */
