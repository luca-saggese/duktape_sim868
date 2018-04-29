/********************************************************************
 *                Copyright Simcom(shanghai)co. Ltd.                               *
 *---------------------------------------------------------------------
 * FileName      :   eat_gps.h
 * version        :   0.10
 * Description   :   eat gps interface define for customer
 * Authors       :   Heweijaing  
 * Notes         :
 *---------------------------------------------------------------------
 *
 *    HISTORY OF CHANGES
 *---------------------------------------------------------------------
 *0.10  2014-07-30, heweijiang, create originally.
 *
 ********************************************************************/
/********************************************************************
 * Include Files
 ********************************************************************/
#ifndef _EAT_GPS_H_
#define _EAT_GPS_H_
#include "eat_type.h"
/********************************************************************
 * Macros
 ********************************************************************/

/********************************************************************
 * Types
 ********************************************************************/
typedef enum {
    EAT_GPS_RST_COLD,
    EAT_GPS_RST_HOT,
    EAT_GPS_RST_WARM,
}EatGpsRstMode;

typedef enum {
    EAT_GPS_Location_Unknown,
    EAT_GPS_FIX_TYPE_INVALID = 1,
    EAT_GPS_FIX_TYPE_2D = 2,
    EAT_GPS_FIX_TYPE_3D = 3
}EatGpsStatus;

typedef enum
{
    EAT_NMEA_OUTPUT_SIMCOM=0   , /*bit 0=1, enable SIMCOM format data output*/
    EAT_NMEA_OUTPUT_GPGGA =1<<1, /*bit 1=1, enable NMEA $GPGGA data output*/
    EAT_NMEA_OUTPUT_GPGLL =1<<2, /*bit 2=1, enable NMEA $GPGLL data output*/
    EAT_NMEA_OUTPUT_GPGSA =1<<3, /*bit 3=1, enable NMEA $GPGSA data output*/
    EAT_NMEA_OUTPUT_GPGSV =1<<4, /*bit 4=1, enable NMEA $GPGSV data output*/
    EAT_NMEA_OUTPUT_GPRMC =1<<5, /*bit 5=1, enable NMEA $GPRMC data output*/
    EAT_NMEA_OUTPUT_GPVTG =1<<6, /*bit 6=1, enable NMEA $GPVTG data output*/
    EAT_NMEA_OUTPUT_GPZDA =1<<7, /*bit 7=1, enable NMEA $GPZDA data output*/
}EatGpsNmeaOutput;

typedef enum
{
    EAT_GEO_FENCE_DISABLE,
    EAT_GEO_FENCE_ENTER,
    EAT_GEO_FENCE_LEAVE,
    EAT_GEO_FENCE_ENTER_OR_LEAVE,
}EatGpsGeoFenceMode;

typedef enum
{
    EAT_GEO_FENCE_CIRCLE_WITH_RADIUS = 1,
    EAT_GEO_FENCE_CIRCLE_WITH_POINT,
    EAT_GEO_FENCE_TRIANGLE,
    EAT_GEO_FENCE_QUADRANGLE,
}EatGpsGeoFenceShape;

typedef void (* GPS_Msg_Proc_Ind)(char *msg_buf, u8 len);

/********************************************************************
 * External Functions declaration
 ********************************************************************/
/********************************************************************
* Function    :    eat_gps_power_req
* Description :    gps power on or power off request
* Parameters  :   power_flag - EAT_TRUE,power on request
*                                         EAT_FALSE,power off request
* Returns     :    NULL
********************************************************************/
extern void (*const eat_gps_power_req)(eat_bool power_flag);

/********************************************************************
* Function    :    eat_gps_power_status
* Description :    get gps power status
* Parameters  :   
*                      
* Returns     :    EAT_TRUE - Power on
*                     EAT_FALSE - Power off
********************************************************************/
extern eat_bool (* const eat_gps_power_status)(void);


/********************************************************************
* Function    :    eat_gps_start_mode_get
* Description :    get gps reset mode
* Parameters  :   
* Returns     :   EatGpsRstMode - EAT_GPS_RST_COLD
*                                            EAT_GPS_RST_HOT
*                                            EAT_GPS_RST_WARM
********************************************************************/
extern EatGpsRstMode (*const eat_gps_start_mode_get)(void);

/********************************************************************
* Function    :    eat_gps_reset_mode_set
* Description :    set gps reset mode
* Parameters  :   mode - EAT_GPS_RST_COLD
*                                 EAT_GPS_RST_HOT
*                                 EAT_GPS_RST_WARM
* Returns     :    EAT_TRUE     set success
*                     EAT_FALSE    set failed 
********************************************************************/
extern eat_bool (*const eat_gps_start_mode_set)(EatGpsRstMode mode);

/********************************************************************
* Function    :    eat_gps_nmea_info_output
* Description :    set gps reset mode
* Parameters  :   mode - GPS_NMEA_OUTPUT_SIMCOM=0   , bit 0=1, enable SIMCOM format data output
*                                 GPS_NMEA_OUTPUT_GPGGA =1<<1,bit 1=1, enable NMEA $GPGGA data output
*                                 GPS_NMEA_OUTPUT_GPGLL =1<<2, bit 2=1, enable NMEA $GPGLL data output
*                                 GPS_NMEA_OUTPUT_GPGSA =1<<3, bit 3=1, enable NMEA $GPGSA data output
*                                 GPS_NMEA_OUTPUT_GPGSV =1<<4, bit 4=1, enable NMEA $GPGSV data output
*                                 GPS_NMEA_OUTPUT_GPRMC =1<<5, bit 5=1, enable NMEA $GPRMC data output
*                                 GPS_NMEA_OUTPUT_GPVTG =1<<6, bit 6=1, enable NMEA $GPVTG data output
*                                 GPS_NMEA_OUTPUT_GPZDA =1<<7, bit 7=1, enable NMEA $GPZDA data output
*                     infobuf - NMEA info buff
*                     size - buff size (>=512)
* Returns     :    EAT_TRUE     set success
*                     EAET_FALSE  set failed
********************************************************************/
extern eat_bool (*const eat_gps_nmea_info_output)(EatGpsNmeaOutput mode, char* infobuf, u32 size);

/********************************************************************
* Function    :    eat_gps_sleep_req
* Description :    gps enter sleep mode request
* Parameters  :   sleep_flag - EAT_TRUE,enter sleep
*                                       EAT_FLASE,wake up
* Returns     :    EAT_TRUE     set success
*                     EAET_FALSE  set failed
********************************************************************/
extern eat_bool (*const eat_gps_sleep_set)(eat_bool sleep_flag);

/********************************************************************
* Function    :    eat_gps_sleep_read
* Description :    gps sleep mode read
* Parameters  :   none
* Returns     :    EAT_BOOL - EAT_TRUE,gps is sleep mode
*                                       EAT_FLASE,gps is wake up mode
********************************************************************/
extern eat_bool (*const eat_gps_sleep_read)(void);

/********************************************************************
* Function    :    eat_gps_status_get
* Description :    get gps enter current status
* Parameters  :  
* Returns     :    EatGpsStatus - EAT_GPS_Location_Unknown,
*                                           EAT_GPS_FIX_TYPE_INVALID = 1,
*                                           EAT_GPS_FIX_TYPE_2D = 2,
*                                           EAT_GPS_FIX_TYPE_3D = 3
********************************************************************/
extern EatGpsStatus (*const eat_gps_status_get)(void);

/********************************************************************
* Function    :    eat_gps_geo_fence_set
* Description :    get gps GEO-fence set
* Parameters  : geo_id  - GEO-fence id(0 - 4)
*                    geo_mode - EatGpsGeoFenceMode
*                    geo_shape - EatGpsGeoFenceShape
*                    lat0 - First point The latitude of a point which is defined as the center of the Geo-Fence circular region or first point.
*                    lng0 - The longitude of a point which is defined as the center of the Geo-Fence circular region or first point.
*                    lat1_or_radius -  <latitude1>\<radius>The radius of a circular or latitude of 2nd point.
*                                           radius region(50 - 6000000)
*                    lng1 - The longitude of 2nd point.
*                    lat2 - The latitude of 3rd point.
*                    lng2 - The longitude of 3rd point.
*                    lat3 - The latitude of 4th point.
*                    lng3 - The longitude of 4th point.
* Returns     :    eat_bool - EAT_TRUE or EAT_FALSE
*
* NOTE:      The unit of latitude and latitude is degree
********************************************************************/
extern eat_bool (*const eat_gps_geo_fence_set)(u8 geo_id, EatGpsGeoFenceMode geo_mode, EatGpsGeoFenceShape geo_shape,
    double lat0, double lng0, double lat1_or_radius, double lng1,double lat2, double lng2,double lat3, double lng3);

/********************************************************************
* Function    :    eat_gps_status_read
* Description :    get gps enter current status
* Parameters  :  geo_id  - GEO-fence id(0 - 4)
*                     strbuf - GEO-fence info
*                     size - strbuf size(>= 512)
* Returns     :    eat_bool - EAT_TRUE or EAT_FALSE  
********************************************************************/
extern eat_bool (*const eat_gps_geo_fence_read)(u8 geo_id, char* strbuf, u32 size);

/********************************************************************
* Function    :    eat_gps_register_msg_proc_callback
* Description :    register gps msg callback function 
* Parameters  :  gps_msg_callback - GPS_Msg_Proc_Ind,handle GEO-Fence URC string
*
*GEO-Fence URC:
+UGEOIND: <geo id 0>,<mode>,<shape>,<status>,
            <GNSS run status>,<Fix status>,
            <UTC date&Time>,<Latitude>,<Longitude>,
            <MSL Altitude>,<Speed Over Ground>,
            <Course Over Ground>,<Fix Mode>,<Reserved1>,
            <HDOP>,<PDOP>,<VDOP>,<Reserved2>,
            <GPS Satellites in View>,<GPS Satellites Used>,
            <GLONASS Satellites Used>
*                     
* Returns     :    eat_bool - EAT_TRUE or EAT_FALSE  
********************************************************************/
extern eat_bool (*const eat_gps_register_msg_proc_callback)(GPS_Msg_Proc_Ind gps_msg_callback);

#endif
