//
//  conf.c
//  testTTY
//
//  Created by qiang on 16/3/31.
//  Copyright © 2016年 Qiangtech. All rights reserved.
//

#include "conf.h"

/*=================================================================
 
 Stinger Configurator Ver. 1.02
 
 Tested on Linux UBUNTU 12.xx / 13.xx and MACOSX
 
 
 ===================================================================*/



/*================================= includes ======================*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>


/*
 This Define is for XCODE debug. If set av[] paramerters will be increased by 1.
 This is used to comply with arguments in debug mode (Product->Scheme->Edit Scheme->Arguments) to be
 passed during debug session.
 */
//#define XCODE

/*
 This Define is for fprint our some DEBUG Strings
 */
//#define DEBUG_1


/*================================= types =========================*/

struct config_args

{
    char in_filename[BUFSIZ];
};

typedef struct config_args conf_args;


/*================================= globals =======================*/

char *progname; /* program name, for error messages */
char version []= "102";

/*================================= prototypes ====================*/

void usage(void);
int main(int ac, char **av);
void process_args(int ac, char **av, conf_args* psa);


/*================================= main()=== =====================*/

//int main(int ac, char **av)
//
//{
//    conf_args sa;
//    /* save name by which program is invoked, for error messages */
//
//    progname = av[0];
//    process_args(ac, av, &sa);
//
//    return 0;
//}

/*=============================== DTR CONTROL ======================
 DTR (0, fd); - Sets the DTR to LOW
 DTR (1, fd); - Sets the DTR to HIGH
 */

int DTR (int on, int fd)
{
    int controlbits = TIOCM_DTR;
    
    if(on)
        
    {
        // SET DTR TO HIGH
#ifdef DEBUG_1
        fprintf(stderr,"Setting DTR\n");
#endif
//        usleep(200000);
        return(ioctl (fd, TIOCMBIC, &controlbits));
    }
    else
    {
        // SET DTR TO LOW
#ifdef DEBUG_1
        fprintf(stderr,"Clearing DTR\n");
#endif
//        usleep(200000);
        return(ioctl (fd, TIOCMBIS, &controlbits));
    }
}


/*=========================== RTS CONTROL ======================
 RTS (0, fd); - Sets the RTS to LOW
 RTS (1, fd); - Sets the RTS to HIGH
 RTS must remain HIGH after DTR goes down.
 */
int RTS (int on, int fd)
{
    int controlbits = TIOCM_RTS;
    
    if(on)
        
    {
        // SET DTR TO HIGH
#ifdef DEBUG_1
        fprintf(stderr,"Setting RTS\n");
#endif
        usleep(200000);
        return(ioctl (fd, TIOCMBIC, &controlbits));
    }
    else
    {
        // SET DTR TO LOW
#ifdef DEBUG_1
        fprintf(stderr,"Clearing RTS\n");
#endif
        usleep(200000);
        return(ioctl (fd, TIOCMBIS, &controlbits));
    }
}

/*======================== Send Command Function =========================*/

void sendcmd (char * command, int byte2write, int fd, char * result )
{
    int res = -1;
    char buffer256 [256];
    
    memset (buffer256, 0, 256);
    
    res = write (fd, command, byte2write);
    
    if (res<0)
    {
        fprintf(stderr,"Write Command Failed!\n");
        strcpy (result, "ERROR");
        return;
    }
    
    // Clear buffer to store answer
    memset (buffer256,0,256);
    
    // Wait 200ms
    usleep (200000);
    
    // Read answer from device
    res = read (fd, &buffer256, 256);
    
    if (res<0)
    {
        fprintf(stderr,"No answer\n");
        memcpy (result, 0, 1);
        return;
    }
    memcpy (result, buffer256, res);
}


/*======================== Config Parser =========================
 This is the command 00 parsing routine which prints out the command 00 results.
 */

void config_parser (char * parsbuffer)

{
    int i = 0;
    
    // Serial Number
    fprintf (stderr,"Serial Number:");
    for (i = 6; i<22; i++)
    {
        fprintf (stderr," %02X", parsbuffer[i] & 0xFF);
    }
    
    // Firmware Version
    fprintf(stderr,"\nFirmware Version: %d.%d\n", parsbuffer[30], parsbuffer[31]);
    
    fprintf(stderr,"\nActual Configuration:\n" );
    fprintf(stderr,"---------------------\n" );
    
    // Selected smartcard connector
    fprintf(stderr,"Reader Number             : %02x ", parsbuffer[1]);
    if (parsbuffer[1] == 1)
        fprintf(stderr,"(Upper Slot)\n");
    if (parsbuffer[1] == 2)
        fprintf(stderr,"(Lower Slot)\n");
    
    // Parse Clock Speed
    /*
     345670 3430000 3.43
     3D0900 4000000 4.00
     493E00 4800000 4.80
     5B8D80 6000000 6.00
     7A1200 8000000 8.00
     B71B00 12000000 12.00
     */
    
#ifdef DEBUG_1
    fprintf(stderr,"\nClock Speed in Hex        : 0x%02X%02X%02X\n", parsbuffer[22] & 0xFF, parsbuffer[23] & 0xFF, parsbuffer[24] & 0xFF);
#endif
    
    if ( ((parsbuffer[22] & 0xFF) == 0x34) && ((parsbuffer[23] & 0xFF) ==  0x56) && ((parsbuffer[24] & 0xFF) == 0x70))
        fprintf(stderr,"Clock Speed               : 3.43 Mhz\n");
    
    if ( ((parsbuffer[22] & 0xFF) == 0x3D) && ((parsbuffer[23] & 0xFF) ==  0x09) && ((parsbuffer[24] & 0xFF) == 0x00))
        fprintf(stderr,"Clock Speed               : 4.00 Mhz\n");
    
    if ( ((parsbuffer[22] & 0xFF) == 0x49) && ((parsbuffer[23] & 0xFF) ==  0x3E) && ((parsbuffer[24] & 0xFF) == 0x00))
        fprintf(stderr,"Clock Speed               : 4.80 Mhz\n");
    
    if ( ((parsbuffer[22] & 0xFF) == 0x5B) && ((parsbuffer[23] & 0xFF) ==  0x8D) && ((parsbuffer[24] & 0xFF) == 0x80))
        fprintf(stderr,"Clock Speed               : 6.00 Mhz\n");
    
    if ( ((parsbuffer[22] & 0xFF) == 0x7A) && ((parsbuffer[23] & 0xFF) ==  0x12) && ((parsbuffer[24] & 0xFF) == 0x00))
        fprintf(stderr,"Clock Speed               : 8.00 Mhz\n");
    
    if ( ((parsbuffer[22] & 0xFF) == 0xB7) && ((parsbuffer[23] & 0xFF) ==  0x1B) && ((parsbuffer[24] & 0xFF) == 0x00))
        fprintf(stderr,"Clock Speed               : 12.00 Mhz\n");
    
    
#ifdef DEBUG_1
    fprintf(stderr,"\nBaudrate in Hex           : 0x%02X\n", parsbuffer[25] & 0xFF);
#endif
    
    
    // Parse Smartcard Speed After ATR
    
    if (parsbuffer[25] == 0)
    {
        fprintf(stderr,"Smartcard Speed after ATR : 9600 Baud\n");
    }
    if (parsbuffer[25] == 1)
    {
        fprintf(stderr,"Smartcard Speed after ATR : 38400 Baud\n");
    }
    if (parsbuffer[25] == 2)
    {
        fprintf(stderr,"Smartcard Speed after ATR : 115200 Baud\n");
    }
    
#ifdef DEBUG_1
    fprintf(stderr,"\nParity in Hex             : 0x%02X\n", parsbuffer[26] & 0xFF);
#endif
    
    // Parse Smartcard Parity
    
    if (parsbuffer[26] == 0)
    {
        fprintf(stderr,"Smartcard Parity          : NONE\n");
    }
    if (parsbuffer[26] == 1)
    {
        fprintf(stderr,"Smartcard Parity          : ODD\n");
    }
    if (parsbuffer[26] == 2)
    {
        fprintf(stderr,"Smartcard Parity          : EVEN\n");
    }
}

/*======================== A5_LOOPBACK =========================
 This Loop function is needed to heck if the Stinger is still connected to the USB.
 mode = 0 - Waits for the Stinger to be disconnected
 mode = 1 - Waits for the Stinger to be connected and returns the new file descriptor n_fd
 */
void A5_loopback (char * result, int fd, int mode, char **av, int * n_fd)

{
    char temp [256];
    int res = -1;
    char buffer256 [256];
    
    memset (buffer256, 0, 256);
    
    temp [0] = 0xA5;
    temp [1] = 0;
    
    if (mode == 0)
    {
        while (1)
        {
            memset (buffer256, 0, 256);
            res = write (fd, temp, 1);
            
            if (res<0)
                fprintf(stderr,"Write A5 ERROR!\n");
            
            fcntl(fd, F_SETFL, FNDELAY);
            sleep (1);
            
            res = read (fd, &buffer256, 32);
            
            if (res>0)
            {
                fprintf(stderr,"\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\\b\b\b\b\b\bWaiting for Stinger disconnection. Please remove the USB Cable!...");
            }
            else
            {
                break;
            }
        }
    }
    
    // Wait for Stinger to be connected again and return n-fd
    
    if (mode == 1)
    {
        // Keeps on looping until /dev/ttyxxxx open function returns a value > -1
        while (1)
        {
            memset (buffer256, 0, 256);
            
#ifdef XCODE
            fd = open (av[2], O_RDWR | O_NOCTTY | O_NDELAY);
#else
            fd = open (av[1], O_RDWR | O_NOCTTY | O_NDELAY);
#endif
            
            if (fd<0)
            {
#ifdef XCODE
                fprintf(stderr,"\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\bPlease connect the Stinger with USB Cable! Waiting for Stinger connection...",av[2]);
#else
                fprintf(stderr,"\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\bPlease connect the Stinger with USB Cable! Waiting for Stinger connection...",av[1]);
#endif
            }
            
            // open /dev/ttyxxxx return a value > -1 return the new file descriptor
            else
            {
                *n_fd = fd;
                break;
            }
        }
    }
}


/*======================== Configure Port =========================*/

int configure_port(int fd)
{
    struct termios port_settings;                       // structure to store the port settings in
    fcntl (fd, F_SETFL, 0);                             // clear all flags
    tcgetattr(fd, &port_settings);                      // Read Options
    cfsetspeed(&port_settings, B115200);
    port_settings.c_cflag |= (CLOCAL | CREAD | CS8);    // set no parity, stop bits, data bits
    port_settings.c_cflag &= ~(PARENB | CSTOPB);        // set no parity, stop bits
    tcsetattr(fd, TCSANOW, &port_settings);             // Write new settings
    
    return 0;
}

///*================================= process_args() ================
// Main routine for argument processing and configuration
// */
//void process_args(int ac, char **av, conf_args* psa)
//{
//
//char s1[256]; /* temp str */
//char s2[256]; /* temp str */
//char s3[256]; /* temp str */
//
//char * str;
//
//char temp [256];
//char buffer256 [256];
//char cmd00 [256];
//
//int fd = -1;
//int n_fd = 0;
//
//int parameter_ok = 0;
//
//int len = 0;
//
//fprintf(stderr, "\nStinger Configurator %.01s.%02s \nFor Linux/MACOSX - Command Line Version\n\n", &version[0],&version[1]);
//
/////////////////////////////////////////////////////////////////
////                                                          //
//// First check if the  INPUT  is in a valid format         //
////                                                        //
/////////////////////////////////////////////////////////////
//
//
//// CHECK NUMBER OF * MINIMUM PARAMETERS *
//
//#ifdef XCODE
//
//// Check if we have at least 3 parameters in XCODE commnand line.
//if (ac<4)
//{
//    fprintf(stderr, "** ERROR - %s: not enough arguments\n", progname);
//    usage();
//    exit(1);
//}
//
//#else
//// Check if we have at least 2 parameters in Console commnand line.
//if (ac<3)
//{
//    fprintf(stderr, "** ERROR - %s: not enough arguments\n", progname);
//    usage();
//    exit(1);
//}
//#endif
//
//
//// CHECK IF A "?" IS PRESENT FOR INFORMATION REQUEST
//
//#ifdef XCODE      // Check if we have at least 3 parameters in XCODE Debug mode.
//if (ac==4)
//{
//    str = strstr (av[3], "?");
//#else
//    if (ac==3)  // Check if we have at least 2 parameters in Console commnand line.
//    {
//        str = strstr (av[2], "?");
//#endif
//        if (str == NULL)
//        {
//            fprintf(stderr, "** ERROR - %s: command not recognized\n", progname);
//            usage();
//            exit(1);
//        }
//        else
//        {
//#ifdef XCODE
//            strcpy(s3, av[3]);
//#else
//            strcpy(s3, av[2]);
//#endif
//        }
//    }
//    
//    
//    
//    // CHECK IF MAXIMUM PARAMETERS ARE EXCEEDED
//    
//#ifdef XCODE      // Check if we have at least 3 parameters in XCODE Debug mode.
//    if (ac>6)
//#else
//        if (ac>5)  // Check if we have at least 2 parameters in Console commnand line.
//#endif
//        {
//            fprintf(stderr, "** ERROR - %s: too many arguments\n", progname);
//            usage();
//            exit(1);
//        }
//    
//    
//    
//    // Check if /dev/ttyUSB sting is specified
//    
//#ifdef XCODE
//    strcpy(s1, av[2]);
//#else
//    strcpy(s1, av[1]);
//#endif
//    
//    str = strstr (s1, "/dev/");
//    
//    // Check if /dev/ is present in argument line
//    if (str == NULL)
//    {
//#ifdef XCODE
//        fprintf(stderr, "** ERROR - Device %s not found! Try again\n", av[2]);
//#else
//        fprintf(stderr, "** ERROR - Device %s not found! Try again\n", av[1]);
//#endif
//        usage();
//        exit(1);
//    }
//    
//    
//    // COPY second argument //
//    
//#ifdef XCODE
//    strcpy(s1, av[3]);
//#else
//    strcpy(s1, av[2]);
//#endif
//    
//    
//    // Open COM PORT
//    
//#ifdef XCODE
//    fd = open (av[2], O_RDWR | O_NOCTTY | O_NDELAY);
//#else
//    fd = open (av[1], O_RDWR | O_NOCTTY | O_NDELAY);
//#endif
//    
//    // Check if COM port is opened successfully
//    if (fd<0)
//    {
//#ifdef XCODE
//        fprintf(stderr,"** ERROR - Couldn't open %s\nPlease check connection and try again.\nREMEMBER TO RUN IN SUPER USER MODE (sudo)!\n Exiting...\n",av[2]);
//#else
//        fprintf(stderr,"** ERROR - Couldn't open %s\nPlease check connection and try again.\nREMEMBER TO RUN IN SUPER USER MODE (sudo)!\n Exiting...\n",av[1]);
//#endif
//        exit(1);
//    }
//    
//    // Com port is opened successfully!
//#ifdef XCODE
//    fprintf(stderr,"Port open %s opened.\n",av[2]);
//#else
//    fprintf(stderr,"Port open %s opened.\n",av[1]);
//#endif
//    
//    // Configure Port
//    configure_port(fd);
//    
//    // SET DTR LOW
//    DTR (0, fd);
//    
//    // SET RTS HIGH - IMPORTANT TO KEEP THE RTS HIGH BEFORE STARTING
//    RTS (1, fd);
//    
//    fprintf (stderr, "\nRetreiving informations. Please wait...\n");
//    
//    // WAIT 3 SECONDS UNTIL RTS HIGH PROCEDURE ENDS
//    usleep (3000000);
//    
//    
//    ////////////////////////////////
//    // ? option = Info only      //
//    //////////////////////////////
//    
//    str = strstr (s3, "?");
//    if (str != NULL)
//    {
//        fprintf (stderr, "\nStinger Information:\n");
//        fprintf (stderr, "--------------------\n");
//        memset (cmd00, 0, 256);
//        temp [0] = 0x00;
//        sendcmd (temp, 1, fd, cmd00);
//        config_parser (cmd00);
//        fprintf (stderr, "\nDone!\n\n");
//        close (fd);
//        // If it is just configuration request exit.
//        exit (1);
//    }
//    
//    
//    ////////////////////////////////
//    // Send Information request  //
//    //////////////////////////////
//    
//    
//    fprintf (stderr, "\nStinger Information:\n");
//    fprintf (stderr, "--------------------\n");
//    memset (cmd00, 0, 256);
//    
//    // Set Command to send to 00
//    temp [0] = 0x00;
//    
//    // Send 02 command lenght 1 bytes and store answer to cmd00
//    sendcmd (temp, 1, fd, cmd00);
//    
//    // Parse command 00 answer
//    config_parser (cmd00);
//    
//    fprintf(stderr,"\nNew Configuration for reader number: %02d ", cmd00[1]);
//    if (cmd00[1] == 1)
//        fprintf(stderr,"(Upper Slot):\n");
//    if (cmd00[1] == 2)
//        fprintf(stderr,"(Lower Slot):\n");
//    fprintf(stderr,"-----------------------------------------------------\n");
//    
//    
//    memset (temp, 0, 256);
//    
//    // Check if the argument is valid if so parameter_ok = 1
//    str = strstr (s1, "343");
//    if (str != NULL)
//    {
//        temp [2] = 0x01;
//        parameter_ok = 1;
//    }
//    
//    str = strstr (s1, "400");
//    if (str != NULL)
//    {
//        temp [2] = 0x02;
//        parameter_ok = 1;
//    }
//    
//    str = strstr (s1, "480");
//    if (str != NULL)
//    {
//        temp [2] = 0x03;
//        parameter_ok = 1;
//    }
//    
//    str = strstr (s1, "600");
//    if (str != NULL)
//    {
//        temp [2] = 0x04;
//        parameter_ok = 1;
//    }
//    
//    str = strstr (s1, "800");
//    if (str != NULL)
//    {
//        temp [2] = 0x05;
//        parameter_ok = 1;
//    }
//    
//    str = strstr (s1, "1200");
//    if (str != NULL)
//    {
//        temp [2] = 0x06;
//        parameter_ok = 1;
//    }
//    
//    // Check if parameter is OK
//    if (parameter_ok == 0)
//    {
//        fprintf(stderr,"**ERROR: Unknow Clock Speed parameter!\n");
//        usage();
//        exit (1);
//    }
//    else
//    {
//        parameter_ok = 0;  // Reset Parameter
//    }
//    
//    
//    /////////////////////////////////////////////
//    // Send 02 Command - Set Clock Speed       //
//    ////////////////////////////////////////////
//    
//    // Prepare command 02 to be send to opened dev
//    temp [0] = 0x02;
//    temp [1] = cmd00[1]-1;
//    
//    memset (buffer256,0,256);
//    
//    // Send 02 command lenght 3 bytes and store answer to buffer256
//    sendcmd (temp, 3, fd, buffer256);
//    
//    // If in DEBUG mode check is the Stinger return the write successful
//#ifdef DEBUG_1
//    if ((buffer256[0] == temp[0]) && (buffer256[1] == 0x00))
//        fprintf(stderr,"Command %02d successful\n", temp[0]);
//    else
//        fprintf(stderr,"Command %02d ERROR\n", temp[0]);
//#endif
//    
//    fprintf(stderr,"Clock Speed               : Value = %02d ", temp[2]);
//    len = strlen (s1);
//    
//    if (len ==3)
//        fprintf(stderr,"set at %.01s.%.01s%.01s Mhz\n", &s1[0], &s1[1], &s1[2]);
//    else
//        fprintf(stderr,"(Upper Slot) set at %.01s%.01s.%.01s%.01s Mhz\n", &s1[0], &s1[1], &s1[2], &s1[3]);
//    
//    
//    // Parse third and forth argument
//    /*
//     
//     Send from PC: 03 ss yy xx (nn)
//     Baudrate
//     yy ->
//     00 == 9600,
//     01 == 38400,
//     02 == 115200
//     
//     */
//    
//    // Copy 3rd argument to s1
//#ifdef XCODE
//    strcpy(s1, av[4]);
//#else
//    strcpy(s1, av[3]);
//#endif
//    
//    // Copy 4th argument to s2
//#ifdef XCODE
//    strcpy(s2, av[5]);
//#else
//    strcpy(s2, av[4]);
//#endif
//    
//    
//    memset (temp, 0, 256);
//    
//    // Check if the argument is valid if so parameter_ok = 1
//    str = strstr (s1, "9600");
//    if (str != NULL)
//    {
//        temp [2] = 0x00;
//        parameter_ok = 1;
//    }
//    
//    str = strstr (s1, "38400");
//    if (str != NULL)
//    {
//        temp [2] = 0x01;
//        parameter_ok = 1;
//    }
//    str = strstr (s1, "115200");
//    if (str != NULL)
//    {
//        temp [2] = 0x02;
//        parameter_ok = 1;
//    }
//    
//    // Check if parameter is OK
//    if (parameter_ok == 0)
//    {
//        fprintf(stderr,"**ERROR: Unknow Baudrate parameter!\n");
//        usage();
//        exit (1);
//    }
//    else
//    {
//        parameter_ok = 0;  // Reset Parameter
//    }
//    
//    
//    // Check if the argument is valid if so parameter_ok = 1
//    str = strstr (s2, "NONE");
//    if (str != NULL)
//    {
//        temp [3] = 0x00;
//        parameter_ok = 1;
//    }
//    
//    str = strstr (s2, "ODD");
//    if (str != NULL)
//    {
//        temp [3] = 0x01;
//        parameter_ok = 1;
//    }
//    str = strstr (s2, "EVEN");
//    if (str != NULL)
//    {
//        temp [3] = 0x02;
//        parameter_ok = 1;
//    }
//    
//    // Check if parameter is OK
//    if (parameter_ok == 0)
//    {
//        fprintf(stderr,"**ERROR: Unknow Parity parameter!\n");
//        usage();
//        exit (1);
//    }
//    else
//    {
//        parameter_ok = 0;  // Reset Parameter
//    }
//    
//    
//    // Prepare 03 Command
//    temp [0] = 0x03;
//    temp [1] = cmd00[1]-1;
//    
//    memset (buffer256,0,256);
//    
//    ////////////////////////////////////////////////////////////////
//    // Send 03 Command - Set baudrate after ATR and parity       //
//    //////////////////////////////////////////////////////////////
//    
//    sendcmd (temp, 4, fd, buffer256);
//    
//    // If in DEBUG mode check is the Stinger return the write successful
//#ifdef DEBUG_1
//    if ((buffer256[0] == temp[0]) && (buffer256[1] == 0x00))
//        fprintf(stderr,"Command %02d successful\n", temp[0]);
//    else
//        fprintf(stderr,"Command %02d ERROR\n", temp[0]);
//    
//#endif
//    
//    fprintf(stderr,"Smartcard Speed after ATR : Value = %02d set at %s baud.\n", temp[2], s1);
//    fprintf(stderr,"Smartcard Parity          : Value = %02d set at %s \n", temp[3], s2);
//    
//    // Wait 500ms before saving configuration
//    usleep (500000);
//    
//    /////////////////////////////////////////////////
//    // Send 05 Command - Save Configuration       //
//    ///////////////////////////////////////////////
//    
//    memset (temp, 0, 256);
//    
//    temp [0] = 0x05;
//    memset (buffer256,0,256);
//    
//    // Send 05 command lenght 1 bytes and store answer to buffer256
//    sendcmd (temp, 1, fd, buffer256);
//    
//    // If in DEBUG mode check is the Stinger return the write successful
//#ifdef DEBUG_1
//    if ((buffer256[0] == temp[0]) && (buffer256[1] == 0x00))
//        fprintf(stderr,"Command %02d successful\n", temp[0]);
//    else
//        fprintf(stderr,"Command %02d ERROR\n", temp[0]);
//#endif
//    
//    // Check if configuration is saved successfully
//    if (buffer256[0] == 0x05)
//    {
//        fprintf(stderr,"\n\n ** Configuration Saved SUCCESSFULLY **\n\nPlease un-plug and plug in the Stinger again!");
//    }
//    else
//    {
//        fprintf(stderr,"\n** ERROR - Save Configuration FAILED. Please un-plug and plug in the Stinger and try again! ");
//    }
//    
//    
//    memset (buffer256,0,256);
//    
//    //////////////////////////////////////////////////////////////////////////////
//    // Call A5_loopback function to check when programmer is disconneted       //
//    ////////////////////////////////////////////////////////////////////////////
//    
//#ifdef XCODE
//    A5_loopback (buffer256, fd, 0, av[2], n_fd);
//#else
//    A5_loopback (buffer256, fd, 0, av[1], n_fd);
//#endif
//    
//    fprintf(stderr,"\nReady to GO!\n\n");
//    
//    // Close COM port
//    close (fd);
//    
//    exit (1);
//    
//}

    
/*================================= usage() =======================*/
void usage(void)

{
    fprintf(stderr,"\nTo configure your Stinger:\n--------------------------\n");
    
    fprintf(stderr,"\nUSAGE: %s device_id clock_speed baudrate parity\n", progname);
    fprintf(stderr," clock_speed values: 343, 400, 480, 600, 800, 1200\n");
    fprintf(stderr," baudrate values: 9600, 38400, 115200\n");
    fprintf(stderr," parity values: ODD, EVEN, NONE\n");
    fprintf(stderr,"\nEXAMPLE: %s /dev/tty.usbserial-000012FDA 400 115200 NONE\n\n", progname);
    
    fprintf(stderr,"\nTo check your configuration:\n----------------------------\n");
    fprintf(stderr,"\nUSAGE: %s device_id ?\n", progname);
    fprintf(stderr,"\nEXAMPLE: %s /dev/tty.usbserial-000012FDA ?\n", progname);
    
    fprintf(stderr,"\nPlease try again...\n");
    
}
/*================================= EOF ===========================*/



int myConfigure_port(int fd)
{
    struct termios port_settings;                       // structure to store the port settings in
    fcntl (fd, F_SETFL, 0);                             // clear all flags
    tcgetattr(fd, &port_settings);                      // Read Options
    cfsetspeed(&port_settings, B9600);
    port_settings.c_cflag |= (CLOCAL | CREAD | CS8);    // set no parity, stop bits, data bits
    port_settings.c_cflag &= ~(PARENB | CSTOPB);        // set no parity, stop bits
    port_settings.c_oflag |= IXON|IXOFF;
    tcsetattr(fd, TCSANOW, &port_settings);             // Write new settings
    
    return 0;
}
void mysendcmd(char * command, int byte2write, int fd, char * result )
{
    int res = -1;
    char buffer256 [256];
    
    memset (buffer256, 0, 256);
    
    res = write (fd, command, byte2write);
    
    if (res<0)
    {
        fprintf(stderr,"Write Command Failed!\n");
        strcpy (result, "ERROR");
        return;
    }
    
    // Clear buffer to store answer
    memset (buffer256,0,256);
    
    // Wait 200ms
    usleep (200000);
    
    // Read answer from device
    //res = read (fd, &buffer256, 1);
    res = -1;
    
    if (res<0)
    {
        fprintf(stderr,"No answer\n");
        //memcpy (result, 0, 1);
        return;
    }
    memcpy (result, buffer256, res);
}
    
//void myProcess()
//{
//    int fd = -1;
//    fd = open ("/dev/tty.usbmodem1421", O_RDWR | O_NOCTTY | O_NDELAY);
//    printf("fd=%i\n",fd);
//    myConfigure_port(fd);
//    
////    RTS (0, fd);
////    usleep (10000);
////    DTR (0, fd);
////    usleep (10000);
////    DTR (1, fd);
////    usleep (10000);
////    DTR (0, fd);
//    
//    // 测试成功，YES!
////    RTS (1, fd);
////    sleep (0.01);
//    DTR (1, fd);
//    sleep (0.01);
//    DTR (0, fd);
//    sleep (0.01);
//    DTR (1, fd);
//    sleep (0.1);// 100ms
//
//    printf("step5 发送1EAF\n");
//    char temp [256];
//    char cmd00 [256];
//    memset (cmd00, 0, 256);
//    char t[4]="1EAF";
//    mysendcmd (t, 4, fd, cmd00);
//    printf("step4\n");
//    
//    config_parser (cmd00);
//    printf("step5\n");
//    
//    close(fd);
//    printf("关闭\n");
//}

void myProcess2(char *devName)
{
    int fd = -1;
    fd = open (devName, O_RDWR | O_NOCTTY | O_NDELAY);
    printf("fd=%i\n",fd);
    myConfigure_port(fd);
    
    DTR (1, fd);
    usleep (500000);  //DTR stay high for 500mS at the beginning
    DTR (0, fd);
    usleep (50000);  //low pulse for 50mS
    DTR (1, fd);
    usleep (5000); // set high for 5mS
    DTR (0, fd);
    usleep (50000);   //low pulse for 50mS again
    DTR (1, fd);    //  set high
    usleep (50000);//  wait for 50ms
    
    
//    DTR (1, fd);
//    usleep(500000); //DTR stay high for 500mS at the beginning
//    DTR (0, fd);
//    usleep (50000);   //low pulse for 50mS again
//    DTR (1, fd);    //  set high
//    usleep (50000);//  wait for 50ms
    
    printf("step5 发送1EAF\n");
    
    char cmd00 [256];
    memset (cmd00, 0, 256);
    char t[4]="1EAF";
    mysendcmd (t, 4, fd, cmd00);
    printf("step4\n");
    sleep(1);
    
    config_parser (cmd00);
    close(fd);
}

//void myProcess3(char *devName)
//{
//    int fd = -1;
//    fd = open (devName, O_RDWR | O_NOCTTY | O_NDELAY);
//    printf("fd=%i\n",fd);
//    myConfigure_port(fd);
//    
//    //    RTS (0, fd);
//    //    usleep (10000);
//    //    DTR (0, fd);
//    //    usleep (10000);
//    //    DTR (1, fd);
//    //    usleep (10000);
//    //    DTR (0, fd);
//    
//    // 测试成功，YES!
//    //RTS (1, fd);
//    //sleep (0.01);
//    
//    sleep (0.1);
//    DTR (1, fd);
//    sleep (0.01);
//    DTR (0, fd);
//    sleep (0.05);
//    DTR (1, fd);
//    sleep (0.2);
//    
//    printf("step5 发送1EAF\n");
//    char temp [256];
//    char cmd00 [256];
//    memset (cmd00, 0, 256);
//    char t[4]="1EAF";
//    mysendcmd (t, 4, fd, cmd00);
//    printf("step4\n");
//    sleep (0.5);
//    
////    config_parser (cmd00);
//    printf("step5\n");
//    
//    close(fd);
//    printf("关闭\n");
//}

