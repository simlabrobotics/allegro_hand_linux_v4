

/*======================*/
/*       Includes       */
/*======================*/
//system headers
#include <stdio.h>
#include <errno.h>
#ifndef _WIN32
#include <inttypes.h>
#include <pthread.h>
#include <syslog.h>
#include <unistd.h>
#else
#include <windows.h>
#endif
#include <malloc.h>
#include <assert.h>

typedef unsigned int DWORD;
typedef unsigned short WORD;
typedef char BYTE;
typedef void* LPSTR;

#include <PCANBasic.h>

#include "canDef.h"
#include "canAPI.h"

CANAPI_BEGIN

/*=====================*/
/*       Defines       */
/*=====================*/
//macros
#define isAlpha(c) ( ((c >= 'A') && (c <= 'Z')) ? 1 : 0 )
#define isSpace(c) ( (c == ' ') ? 1 : 0 )
#define isDigit(c) ( ((c >= '0') && (c <= '9')) ? 1 : 0 )

//constants
#define NUM_OF_FINGERS          4 // number of fingers
#define NUM_OF_TEMP_SENSORS     4 // number of temperature sensors

//structures
typedef struct __attribute__((packed))
{
    unsigned short position;
    unsigned short imu;
    unsigned short temp;
} can_period_msg_t;

typedef struct __attribute__((packed))
{
    unsigned char set;
    unsigned char did;
    unsigned int baudrate;
} can_config_msg_t;



/*=========================================*/
/*       Global file-scope variables       */
/*=========================================*/
unsigned char CAN_ID = 0;
TPCANHandle canDev[MAX_BUS] = {
    PCAN_NONEBUS, // Undefined/default value for a PCAN bus

    PCAN_ISABUS1, // PCAN-ISA interface, channel 1
    PCAN_ISABUS2, // PCAN-ISA interface, channel 2
    PCAN_ISABUS3, // PCAN-ISA interface, channel 3
    PCAN_ISABUS4, // PCAN-ISA interface, channel 4
    PCAN_ISABUS5, // PCAN-ISA interface, channel 5
    PCAN_ISABUS6, // PCAN-ISA interface, channel 6
    PCAN_ISABUS7, // PCAN-ISA interface, channel 7
    PCAN_ISABUS8, // PCAN-ISA interface, channel 8

    PCAN_DNGBUS1, // PCAN-Dongle/LPT interface, channel 1

    PCAN_PCIBUS1, // PCAN-PCI interface, channel 1
    PCAN_PCIBUS2, // PCAN-PCI interface, channel 2
    PCAN_PCIBUS3, // PCAN-PCI interface, channel 3
    PCAN_PCIBUS4, // PCAN-PCI interface, channel 4
    PCAN_PCIBUS5, // PCAN-PCI interface, channel 5
    PCAN_PCIBUS6, // PCAN-PCI interface, channel 6
    PCAN_PCIBUS7, // PCAN-PCI interface, channel 7
    PCAN_PCIBUS8, // PCAN-PCI interface, channel 8

    PCAN_USBBUS1, // PCAN-USB interface, channel 1
    PCAN_USBBUS2, // PCAN-USB interface, channel 2
    PCAN_USBBUS3, // PCAN-USB interface, channel 3
    PCAN_USBBUS4, // PCAN-USB interface, channel 4
    PCAN_USBBUS5, // PCAN-USB interface, channel 5
    PCAN_USBBUS6, // PCAN-USB interface, channel 6
    PCAN_USBBUS7, // PCAN-USB interface, channel 7
    PCAN_USBBUS8, // PCAN-USB interface, channel 8

    PCAN_PCCBUS1, // PCAN-PC Card interface, channel 1
    PCAN_PCCBUS2, // PCAN-PC Card interface, channel 2
};

/*==========================================*/
/*       Private functions prototypes       */
/*==========================================*/
int canReadMsg(int bus, int *id, int *len, unsigned char *data, int blocking);
int canSendMsg(int bus, int id, char len, unsigned char *data, int blocking);

/*========================================*/
/*       Public functions (CAN API)       */
/*========================================*/
int initCAN(int bus){
    TPCANStatus Status = PCAN_ERROR_OK;
    char strMsg[256];
    TPCANBaudrate Baudrate = PCAN_BAUD_1M;
    TPCANType HwType = 0;
    DWORD IOPort = 0;
    WORD Interrupt = 0;

    Status = CAN_Initialize(canDev[bus], Baudrate, HwType, IOPort, Interrupt);
    if (Status != PCAN_ERROR_OK)
    {
        CAN_GetErrorText(Status, 0, strMsg);
        printf("initCAN(): CAN_Initialize() failed with error %u\n", Status);
        printf("%s\n", strMsg);
        return Status;
    }

    Status = CAN_Reset(canDev[bus]);
    if (Status != PCAN_ERROR_OK)
    {
        CAN_GetErrorText(Status, 0, strMsg);
        printf("initCAN(): CAN_Reset() failed with error %u\n", Status);
        printf("%s\n", strMsg);
        return Status;
    }

    return 0; // PCAN_ERROR_OK
}

int freeCAN(int bus){
    TPCANStatus Status = PCAN_ERROR_OK;
    char strMsg[256];

    Status = CAN_Uninitialize(canDev[bus]);
    if (Status != PCAN_ERROR_OK)
    {
        CAN_GetErrorText(Status, 0, strMsg);
        printf("freeCAN(): CAN_Uninitialize() failed with error %u\n", Status);
        printf("%s\n", strMsg);
        return Status;
    }

    return 0; // PCAN_ERROR_OK
}

int canReadMsg(int bus, int *id, int *len, unsigned char *data, int blocking){
    TPCANMsg CANMsg;
    TPCANTimestamp CANTimeStamp;
    TPCANStatus Status = PCAN_ERROR_OK;
    char strMsg[256];
    int i;

    // We execute the "Read" function of the PCANBasic
    Status = CAN_Read(canDev[bus], &CANMsg, &CANTimeStamp);
    if (Status != PCAN_ERROR_OK)
    {
        if (Status != PCAN_ERROR_QRCVEMPTY)
        {
            CAN_GetErrorText(Status, 0, strMsg);
            printf("canReadMsg(): CAN_Read() failed with error %u\n", Status);
            printf("%s\n", strMsg);
        }

        return Status;
    }

    *id = (CANMsg.ID & 0xfffffffc) >> 2;
    *len = CANMsg.LEN;
    for(i = 0; i < CANMsg.LEN; i++)
        data[i] = CANMsg.DATA[i];

    return 0;
}

int canSendMsg(int bus, int id, char len, unsigned char *data, int blocking){
    TPCANMsg CANMsg;
    TPCANStatus Status = PCAN_ERROR_OK;
    char strMsg[256];
    int i;

    CANMsg.ID = (id << 2) | CAN_ID;
    CANMsg.LEN = len & 0x0F;
    for(i = 0; i < len; i++)
        CANMsg.DATA[i] = data[i];
    CANMsg.MSGTYPE = PCAN_MESSAGE_STANDARD;
    Status = CAN_Write(canDev[bus], &CANMsg);
    if (Status != PCAN_ERROR_OK)
    {
        CAN_GetErrorText(Status, 0, strMsg);
        printf("canSendMsg(): CAN_Write() failed with error %u\n", Status);
        printf("%s\n", strMsg);
        return Status;
    }

    return 0; //PCAN_ERROR_OK;
}

int canSentRTR(int bus, int id, int blocking){
    TPCANMsg CANMsg;
    TPCANStatus Status = PCAN_ERROR_OK;
    char strMsg[256];

    CANMsg.ID = (id << 2) | CAN_ID;
    CANMsg.LEN = 0;
    CANMsg.MSGTYPE = PCAN_MESSAGE_RTR; // Remote Transmission Request
    Status = CAN_Write(canDev[bus], &CANMsg);
    if (Status != PCAN_ERROR_OK)
    {
        CAN_GetErrorText(Status, 0, strMsg);
        printf("canSendMsg(): CAN_Write() failed with error %u\n", Status);
        printf("%s\n", strMsg);
        return Status;
    }

    return 0; //PCAN_ERROR_OK;
}

/*========================================*/
/*       CAN API                          */
/*========================================*/
int command_can_open(int ch)
{
    assert(ch >= 0 && ch < MAX_BUS);

    DWORD ret;

    printf("<< CAN: Open Channel...\n");
    ret = initCAN(ch);
    if (ret != 0) return ret;
    printf("\t- Ch.%2d (OK)\n", ch);
    printf("\t- Done\n");

    return ret;
}

int command_can_open_ex(int ch, int type, int index)
{
    return command_can_open(ch);
}

int command_can_reset(int ch)
{
    return -1;
}

int command_can_close(int ch)
{
    assert(ch >= 0 && ch < MAX_BUS);

    TPCANStatus Status = PCAN_ERROR_OK;
    char strMsg[256];
    printf("<< CAN: Close...\n");

    Status = CAN_Uninitialize(canDev[ch]);
    if (Status != PCAN_ERROR_OK)
    {
        CAN_GetErrorText(Status, 0, strMsg);
        printf("freeCAN(): CAN_Uninitialize() failed with error %u\n", Status);
        printf("%s\n", strMsg);
        return Status;
    }

    printf("\t- Done\n");
    return 0; //PCAN_ERROR_OK;
}

int command_can_set_id(int ch, unsigned char can_id)
{
    CAN_ID = can_id;
    return 0; //PCAN_ERROR_OK;
}

int command_servo_on(int ch)
{
    assert(ch >= 0 && ch < MAX_BUS);

    long Txid;
    unsigned char data[8];
    int ret;

    Txid = ID_CMD_SYSTEM_ON;
    ret = canSendMsg(ch, Txid, 0, data, TRUE);

    return ret;
}

int command_servo_off(int ch)
{
    assert(ch >= 0 && ch < MAX_BUS);

    long Txid;
    unsigned char data[8];
    int ret;

    Txid = ID_CMD_SYSTEM_OFF;
    ret = canSendMsg(ch, Txid, 0, data, TRUE);

    return ret;
}

int command_set_torque(int ch, int findex, short* pwm)
{
    assert(ch >= 0 && ch < MAX_BUS);
    assert(findex >= 0 && findex < NUM_OF_FINGERS);

    long Txid;
    short duty[4];
    int ret;

    if (findex >= 0 && findex < NUM_OF_FINGERS)
    {
        duty[0] = pwm[0];
        duty[1] = pwm[1];
        duty[2] = pwm[2];
        duty[3] = pwm[3];

        Txid = ID_CMD_SET_TORQUE_1 + findex;

        ret = canSendMsg(ch, Txid, 8, (unsigned char *)duty, TRUE);
    }
    else
        return -1;

    return ret;
}

int command_set_pose(int ch, int findex, short* jposition)
{
    assert(ch >= 0 && ch < MAX_BUS);
    assert(findex >= 0 && findex < NUM_OF_FINGERS);

    long Txid;
    short pose[4];
    int ret;

    if (findex >= 0 && findex < NUM_OF_FINGERS)
    {
        pose[0] = jposition[0];
        pose[1] = jposition[1];
        pose[2] = jposition[2];
        pose[3] = jposition[3];

        Txid = ID_CMD_SET_POSE_1 + findex;

        ret = canSendMsg(ch, Txid, 8, (unsigned char *)pose, TRUE);
    }
    else
        return -1;

    return ret;
}

int command_set_period(int ch, short* period)
{
    assert(ch >= 0 && ch < MAX_BUS);

    long Txid;
    can_period_msg_t msg;
    int ret;

    Txid = ID_CMD_SET_PERIOD;
    if (period != 0)
    {
        msg.position = period[0];
        msg.imu = period[1];
        msg.temp = period[2];
    }
    else
    {
        msg.position = 0;
        msg.imu = 0;
        msg.temp = 0;
    }
    ret = canSendMsg(ch, Txid, 6, (unsigned char *)&msg, TRUE);

    return ret;
}

int command_set_device_id(int ch, unsigned char did)
{
    assert(ch >= 0 && ch < MAX_BUS);

    long Txid;
    int ret;
    can_config_msg_t msg;

    Txid = ID_CMD_CONFIG;
    msg.set = 0x01;
    msg.did = did;
    msg.baudrate = 0;
    ret = canSendMsg(ch, Txid, 6, (unsigned char *)&msg, TRUE);

    return ret;
}

int command_set_rs485_baudrate(int ch, unsigned int baudrate)
{
    assert(ch >= 0 && ch < MAX_BUS);

    long Txid;
    int ret;
    can_config_msg_t msg;

    Txid = ID_CMD_CONFIG;

    msg.set = 0x02;
    msg.did = 0;
    msg.baudrate = baudrate;
    ret = canSendMsg(ch, Txid, 6, (unsigned char *)&msg, TRUE);

    return ret;
}

int request_hand_information(int ch)
{
    assert(ch >= 0 && ch < MAX_BUS);

    long Txid = ID_RTR_HAND_INFO;
    int ret = canSentRTR(ch, Txid, TRUE);

    return ret;
}

int request_hand_serial(int ch)
{
    assert(ch >= 0 && ch < MAX_BUS);

    long Txid = ID_RTR_SERIAL;
    int ret = canSentRTR(ch, Txid, TRUE);

    return ret;
}

int request_finger_pose(int ch, int findex)
{
    assert(ch >= 0 && ch < MAX_BUS);
    assert(findex >= 0 && findex < NUM_OF_FINGERS);

    long Txid = ID_RTR_FINGER_POSE + findex;
    int ret = canSentRTR(ch, Txid, TRUE);

    return ret;
}

int request_imu_data(int ch)
{
    assert(ch >= 0 && ch < MAX_BUS);

    long Txid = ID_RTR_IMU_DATA;
    int ret = canSentRTR(ch, Txid, TRUE);

    return ret;
}

int request_temperature(int ch, int sindex)
{
    assert(ch >= 0 && ch < MAX_BUS);
    assert(sindex >= 0 && sindex < NUM_OF_TEMP_SENSORS);

    long Txid = ID_RTR_TEMPERATURE + sindex;
    int ret = canSentRTR(ch, Txid, TRUE);

    return ret;
}

int get_message(int ch, int* id, int* len, unsigned char* data, int blocking)
{
    int err;
    err = canReadMsg(ch, id, len, data, blocking);
    return err;
}


CANAPI_END
