#ifndef BASE_DEF_H_
#define BASE_DEF_H_

typedef unsigned char			uint8;
typedef unsigned short			uint16;
typedef unsigned int			uint32;
typedef unsigned long long		uint64;

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

#ifdef __linux__
#define	SYS_COMP_PATH			SYS_PATH "/cross/linux"
#else
#define SYS_COMP_PATH 			SYS_PATH "/cross/win"
#endif

#define SYS_ATH_RACK_PATH		SYS_ATH_PATH "/rack_001"
#define SYS_ATH_UPDATE_PATH		SYS_ATH_PATH "/update"
#define SYS_SHA_UPDATE_PATH		SYS_SHA_PATH "/update"

#define SYS_MMC_PATH			"/mmc"

#define SYS_IN_FILENAME			"in"
#define SYS_OUT_FILENAME		"out"
#define SYS_CAL_FILENAME		"cal"
#define SYS_STATE_FILENAME		"state"

#define TMON_MAIN_PORT(x)		(4*x+100)
#define TMON_DEBUG_PORT(x)		(4*x+101)

#define TMON_MSG_PORT(x)		(4*x+102)
#define TMON_MSG_D_PORT(x)		(4*x+103)

#define TMON_MSG_Q_KEY(x)		(4*x+102)
#define TMON_MSG_D_Q_KEY(x)		(4*x+103)

#define CMON_MAIN_PORT			(4322)

#define SOCK_MSG_PORT(x,y)		(x*MAX_TEST_PORT+1000+y)

#define NORMAL_END				(0)
#define ABNORMAL_END			(-1)

#define DEFAULT_IP				"192.168.100."
#define SERVER_IP				"192.168.100.250"
#define DEBUGGER_IP				"192.168.100.240"

#define SERVER_ID				"admin"
#define SERVER_PASS				"Exicon777"
#define SERVER_HOST				"athost"
#define SERVER_SHARE			"share"
#define SERVER_DEBUG			"debug"

#define IsSetBit(bits, n)		((((unsigned int *)(bits))[(n)>>5] &  (1 << ((n)&0x1F))) != 0)
#define SetBit(bits, n)			(((unsigned int *)(bits))[(n)>>5] |=  (1 << ((n)&0x1F)))
#define ClearBit(bits, n)		(((unsigned int *)(bits))[(n)>>5] &= ~(1 << ((n)&0x1F)))

#define MAX_TEST_PORT			(30)
#define MAX_TEST_FLAG			(16)

enum
{
	PROJ_BIST = 0,
	PROJ_SST12K,
	PROJ_OBOS,
};

#define BIST_MAX_PORT			(12)
#define SST12K_MAX_PORT			(2)
#define OBOS_MAX_PORT			(2)

typedef struct
{
	char     opname[32];
	int      opcode;
} __attribute__((packed)) opcode_t;


typedef struct
{
	uint16 msg_cmd;
    char     msg_name[32];
} __attribute__((packed)) msg_code_t;



typedef struct tagMsgPackHdr
{
	uint16			version;	// Version of this structure.
	uint16			cell;		// Cell from/to.
	uint16			port;		// Port from/to.
	uint16			msg;		// Msg.
	uint16			packet;		// This is running counter (every packet will have unique id
	uint16			flag;
} __attribute__ (( packed )) ExMsgPackHdr;

#define MSG_PACKET_VERSION			0x0002
#define MSG_HEADER_VERSION			(13330)
#define MSG_PACKET_SIZE				32
#define MSG_PACKET_SIZE_2			64
#define MAX_MSG_STRING_LENGTH		(MSG_PACKET_SIZE_2-sizeof(ExMsgPackHdr))

typedef struct tagMsgPack
{
	ExMsgPackHdr	hdr;
	char			msgstr[MAX_MSG_STRING_LENGTH];
} __attribute__ (( packed )) ExMsgPack;

typedef struct tagCmdPack
{
	char			cmdstr[256];
} __attribute__ (( packed )) ExCmdPack;

/*
#define MSG_PACKET_VERSION_2			0x0002
#define MSG_HEADER_VERSION_2			(13330)
#define MSG_PACKET_SIZE_2				128
#define MAX_MSG_STRING_LENGTH_2			(MSG_PACKET_SIZE2-sizeof(ExMsgPackHdr))

typedef struct tagMsgPack2
{
	ExMsgPackHdr	hdr;
	char			msgstr[MAX_MSG_STRING_LENGTH];
} __attribute__ (( packed )) ExMsgPack2;
*/

typedef struct tagSysMsgQ
{
	long		lMsgType;
	ExMsgPack 	pack;
} __attribute__ (( packed )) ExSysMsg;

/*
typedef struct tagSysMsgQ2
{
	long		lMsgType;
	ExMsgPack2	pack;
} __attribute__ (( packed )) ExSysMsg2;
*/

typedef struct
{
    char     opn[32];
	uint32   opc;
} __attribute__((packed)) ExOpName_t;

enum
{
	YELLOW = 0,
	RED = 1,
	GREEN = 2,
	BLUE = 3,
};

enum
{
	MSG_NONE = 0,
	MSG_TEST_START,          //  1:Host ---> Target     - Request Target to start the test
	MSG_TEST_STOP,           //  2:Host ---> Target     - Request Target to stop the test
	MSG_INIT,           //  3:Host <--> Target     - Request Target for init state (thru this, host/target will know current state if the host/target)
	MSG_INITACK,        //  4:Host <--> Target     - Acknoledge receive of INIT msg

	MSG_INITCOLOR,      //  5:Host <--- Target     - Sets port color to ACTIVE color (WHITE)
	MSG_PASS,           //  6:Host <--- Target     - Sets port color to GREEN
	MSG_FAIL,           //  7:Host <--- Target     - Sets port color to RED
	MSG_TEST,           //  8:Host <--- Target     - Sets port color to YELLOW  - port is in TEST mode
	MSG_DONE,	    //  9:Host <--- Target     - Target sending informative message.
	MSG_TEXT1,          // 10:Host <--- Target     - Target sending Text msg to host for host to display on text box 1
	MSG_TEXT2,          // 11:Host <--- Target     - Target sending text msg to host for host to display on text box 2
	MSG_TEXT3,          // 12:Host <--- Target     - Target sending text msg to host for host to display on text box 3
	MSG_TEXT4,	    // 13:Host <--- Target	   - Target sending test msg( ErrorMsg ) to host

	MSG_TIMER_START,	// 14:Host <--- Target     - Target requesting host to start time timer - target will sent initial start value
	MSG_TIMER_STOP,		// 15:Host <--- Target     - Target requesting host to display the active timer value

	MSG_INFOR,		// 16:Host <--- Target     - Target sending informative message.
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

	MSG_REBOOT,		// 30:Host ---> Target     - reboot linux system

	MSG_RESPONSE,		// 31:Host <--- Target     - Target Response
	MSG_DISCON,		// 32:Host ---> Host       - Host Disconnect Self

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

	MSG_ENDMARKER = 0xFFFF,
};


#endif /* BASE_DEF_H_ */
