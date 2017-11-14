/*
 * def.h
 *
 *  Created on: Sep 12, 2017
 *      Author: shjeong
 */

#ifndef DEF_H_
#define DEF_H_

//data type
#define uint8	unsigned char
#define uint16	unsigned short
#define uint32	unsigned int
#define uint64	unsigned long long
#define int8	char
#define int16	short
#define int32	int
#define int64	long long

//path
#define SYS_PATH				"/exicon"

#define SYS_CFG_PATH			SYS_PATH "/config"
#define SYS_SHA_PATH			SYS_PATH "/share"
#define SYS_TMP_PATH			SYS_PATH "/temp"
#define SYS_SRC_PATH			SYS_PATH "/src"
#define SYS_DATA_PATH 			SYS_PATH "/data"
#define SYS_FIRM_PATH 			SYS_PATH "/firm"
#define SYS_UTIL_PATH 			SYS_PATH "/util"
#define SYS_DIAG_PATH 			SYS_PATH "/diag"
#define SYS_DEBUG_PATH 			SYS_PATH "/debug"

#define SYS_ATH_PATH			SYS_PATH "/athost"
#define SYS_BIN_PATH			SYS_PATH "/bin"
#define SYS_DATA_PATH 			SYS_PATH "/data"
#define SYS_LIB_PATH			SYS_PATH "/lib"
#define SYS_INC_PATH			SYS_PATH "/include"
#define SYS_LOG_PATH			SYS_PATH "/log"
#define SYS_EXEC_PATH			SYS_PATH "/exec"
#define SYS_SCRIPT_PATH			SYS_PATH "/script"
#define SYS_SHE_PATH			SYS_PATH "/shell"
#define SYS_WORK_PATH			SYS_PATH "/work"
#define SYS_UPDATE_PATH			SYS_PATH "/update"

//info file name
#define FILE_TPC_ID             SYS_DATA_PATH "/tpc_id.txt"
#define FILE_BD_CONNECT         SYS_DATA_PATH "/bd_connect.txt"
#define FILE_VERSION            SYS_DATA_PATH "/version.txt"

//log name
#define TEST_LOG_NAME			"port"
#define EVENT_LOG_NAME			"event"
#define MSGQ_LOG_NAME			"msgq"
#define SOCKET_LOG_NAME			"socket"

//EXT name
//#define TEST_SCRIPT_ORI_EXT		"stc"
#define TEST_SCRIPT_ORI_EXT		"uts"
#define	TEST_SCRIPT_RUN_EXT		"c"

//compile option
#define COMPILE_PROG			"/usr/bin/gcc"	//"tcc"
#define COMPILE_INCPATH			"-I"SYS_INC_PATH
#define COMPILE_LIBPATH			"-L"SYS_LIB_PATH
#define COMPILE_LIB				"-ltbase -ltnet -lpthread"

//msgq
#define KEY_TEST_MSGQ			0x1000

//semaphore
#define KEY_BD_LOCK             0x1000
#define KEY_DPS_SHARE_LOCK  	0x1001

//share memory
#define KEY_DPS_SHARE			0x1000

//socket
#define PORT_TDM				5000
#define SOCKET_START_MARK		"<"
#define SOCKET_END_MARK			">"
#define SOCKET_SPLIT_MARK       ","
#define SOCKET_DATA_CNT         7
#define SOCKET_MARK_CNT         (SOCKET_DATA_CNT-1)
#define SOCKET_BUF_SIZE			1024
#define SOCKET_MAX_BUF_SIZE		(SOCKET_BUF_SIZE*1)

//MSG monitoring file
#define MSGBOX_SEND_TO			"in"
#define MSGBOX_RECV_FROM		"out"

//type MSGQ
#define TYPE_MSGQ_SEND			1
#define TYPE_MSGQ_RECV			2

//define value
#define ON						1
#define	OFF						0

#define PATHNAME_SIZE			256

#define SC_FILE_DEFAULT         0x0
#define SC_FILE_PASS            0x1
#define SC_FILE_NAME_ERR        0x2
#define SC_FILE_SIZE_ERR        0x3
#define SC_FILE_NONE_ERR        0x4
#define SC_FILE_OTHER_ERR       0x5

#define TPC_ID_DEBUG            200

//port index
enum PORT_INDEX
{
    PORT1       = 0,
    PORT2,
    PORT_CNT,
    PORT_MIN    = PORT1,
    PORT_MAX    = PORT2
};

//dps voltage index
enum DPS_CH_INDEX
{
    DPS_CH1 = 0,        //5V
    DPS_CH2,            //12V
    DPS_CH_CNT,
    DPS_CH_MIN  = DPS_CH1,
    DPS_CH_MAX  = DPS_CH2
};

//dps voltage calibration index
enum DPS_VOLT_CAL_INDEX
{
    //VOLT_CAL_3V3 = 0,
    VOLT_CAL_5V =0,
    VOLT_CAL_12V,
    VOLT_CAL_CNT
};

//dps current calibration index
enum DPS_CUR_CAL_INDEX
{
    CUR_CAL_3V3 = 0,
    CUR_CAL_5V,
    CUR_CAL_12V,
    CUR_CAL_CNT
};

//monitor os status
typedef struct tOsStatus
{
    char sTime[32];
    char sCpuUsage[32];
    char sMemUsage[32];
    char sDiskUsage[32];
    char sMount[32];
    char sBdConnect[32];
}OsStatus;

//monitor dps status
typedef struct tDpsStatus
{
    char sDpsSetVoltage[DPS_CH_CNT][32];
    char sDpsGetVoltage[DPS_CH_CNT][32];
    char sDpsSetCurrent[DPS_CH_CNT][32];
    char sDpsGetCurrent[DPS_CH_CNT][32];
    bool bDpsPower; //on, off
    bool bDpsOcp;
    bool bDpsOvp;
}DpsStatus;

//monitor perf status
typedef struct tPerfStatus
{
    char sWrite[32];
    char sRead[32];
}PerfStatus;

//msg header
typedef struct tMsgHdr
{
	uint16		version;	// Version of this structure.
	uint16		cell;		// Cell from/to.
	uint16		port;		// Port from/to.
	uint16		msg_no;		// Msg.
	uint16		packet;
	uint16		flag;
}__attribute__ ((packed)) MsgHdr;

#define MSG_HEADER_SIZE			sizeof(MsgHdr)
#define MSG_PACKET_SIZE			32
#define MSG_STRING_LENGTH		(MSG_PACKET_SIZE - MSG_HEADER_SIZE)

//msg packet
typedef struct tMsgPack
{
	MsgHdr		hdr;
    char		string[MSG_STRING_LENGTH];
}__attribute__ ((packed)) MsgPack;

//msgq of msg packet
typedef struct tMsgPackQ
{
	long		type;
	MsgPack		msg;
}__attribute__ ((packed)) MsgPackQ;

//socket packet
typedef struct tSockPack
{
	MsgHdr		hdr;
    char		*pstring;
}__attribute__ ((packed)) SockPack;

//msg version
enum MSG_VERSION
{
    MSGVER_NONE = 0,
    MSGVER_BD_INFO,
    MSGVER_BD_DIAG,
    MSGVER_BD_UPDATE,
    MSGVER_PORT_DPS,
    MSGVER_PORT_DIAG,
    MSGVER_PORT_TEST
};

//msg number
enum MSG_NUMBER
{
	MSG_NONE = 0,
	MSG_TEST_START,     //  1:Host ---> Target     - Request Target to start the test
	MSG_TEST_STOP,      //  2:Host ---> Target     - Request Target to stop the test
	MSG_INIT,           //  3:Host <--> Target     - Request Target for init state (thru this, host/target will know current state if the host/target)
	MSG_INITACK,        //  4:Host <--> Target     - Acknoledge receive of INIT msg

	MSG_INITCOLOR,      //  5:Host <--- Target     - Sets port color to ACTIVE color (WHITE)
	MSG_PASS,           //  6:Host <--- Target     - Sets port color to GREEN
	MSG_FAIL,           //  7:Host <--- Target     - Sets port color to RED
	MSG_TEST,     	    //  8:Host <--- Target     - Sets port color to YELLOW  - port is in TEST mode
	MSG_DONE,	    	//  9:Host <--- Target     - Target sending informative message.
	MSG_TEXT1,          // 10:Host <--- Target     - Target sending Text msg to host for host to display on text box 1
	MSG_TEXT2,          // 11:Host <--- Target     - Target sending text msg to host for host to display on text box 2
	MSG_TEXT3,          // 12:Host <--- Target     - Target sending text msg to host for host to display on text box 3
	MSG_TEXT4,	    	// 13:Host <--- Target	   - Target sending test msg( ErrorMsg ) to host

	MSG_TIMER_START,	// 14:Host <--- Target     - Target requesting host to start time timer - target will sent initial start value
	MSG_TIMER_STOP,		// 15:Host <--- Target     - Target requesting host to display the active timer value

	MSG_INFOR,			// 16:Host <--- Target     - Target sending informative message.
	MSG_DIAGSTART,		// 17:Host <----Target     - Target sending Diagnostic Start message.
	MSG_DIAGPASS,		// 18:Host <--- Target     - Target sending Diagnostic Pass message.
	MSG_DIAGFAIL,		// 19:Host <--- Target     - Target sending Diagnostic Fail message.
	MSG_SETBADPORT,		// 20:Host ---> Target	   - Set Port as Bad Port
	MSG_RELEASEBADPORT,	// 21:Host ---> Target	   - Release Port as Bad Port
	MSG_LEDCTRL,		// 22:Host ---> Target	   - Release Port as Bad Port

	MSG_MOUNT_CHECK,	// 23:Host ---> Target     - mount check
	MSG_COM_OPEN,		// 24:Host ---> Target     -
	MSG_COM_CLOSE,		// 25:Host ---> Target     -
	MSG_COM_TEST,		// 26:Host ---> Target     -

	MSG_SERIAL_NUMBER,	// 27:Host ---> Target	   - send serial number
	MSG_CONFIGURATION,	// 28:Host ---> Target     - configuration
	DATE_TIME_SET,		// 29:Host ---> Target     - date & time set

	MSG_REBOOT,			// 30:Host ---> Target     - reboot linux system

	MSG_RESPONSE,		// 31:Host <--- Target     - Target Response
	MSG_DISCON,			// 32:Host ---> Host       - Host Disconnect Self

	MSG_SBL_FAIL,		// 33:Host <--- Target     - Sets port color to RED
	MSG_DUT_FAIL,		// 34:Host <--- Target     - Check DUT color (WHITE)

	MSG_SYSCODE,		// 35:Host ---> Target     - Check System Code BoardName
	MSG_TESTFAIL,		// 36:Host <--- Target     - Test Start Fail Reply Msg

	MSG_DPS_CHK_START,
	MSG_DPS_CHK_STOP,
	MSG_DPS_RESET,

	MSG_DPS_SET_VOLT,
	MSG_DPS_GET_VOLT,
	MSG_DPS_GET_CURRENT,

	MSG_EX_TEST_CONFIG,
	MSG_EX_TEST_MODE,
	MSG_EX_USER_COMMAND,
	MSG_EX_SYS_CMD,

	MSG_ENGINE_STOP,

	MSG_SYS_TEST,
	MSG_SYS_FAIL,
	MSG_SYS_PASS,
	MSG_SYS_STOP,
	MSG_SYS_TEXT1,
	MSG_SYS_TEXT2,
	MSG_SYS_TEXT3,
	MSG_SYS_TEXT4,
	MSG_SYS_TEXT5,
	MSG_SYS_TEXT6,
	MSG_SYS_TEXT7,
	MSG_SYS_TEXT8,

    MSG_ENDMARKER = 0xFFFF
};

#endif /* DEF_H_ */
