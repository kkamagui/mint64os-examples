/**
 *  file    RTC.c
 *  date    2009/02/14
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   RTC 컨트롤러에 관련된 소스 파일
 */

#include "RTC.h"

/**
 *  CMOS 메모리에서 RTC 컨트롤러가 저장한 현재 시간을 읽음
 */
void kReadRTCTime( BYTE* pbHour, BYTE* pbMinute, BYTE* pbSecond )
{
    BYTE bData;
    
    // CMOS 메모리 어드레스 레지스터(포트 0x70)에 시간을 저장하는 레지스터 지정
    kOutPortByte( RTC_CMOSADDRESS, RTC_ADDRESS_HOUR );
    // CMOS 데이터 레지스터(포트 0x71)에서 시간을 읽음
    bData = kInPortByte( RTC_CMOSDATA );
    *pbHour = RTC_BCDTOBINARY( bData );
    
    // CMOS 메모리 어드레스 레지스터(포트 0x70)에 분을 저장하는 레지스터 지정
    kOutPortByte( RTC_CMOSADDRESS, RTC_ADDRESS_MINUTE );
    // CMOS 데이터 레지스터(포트 0x71)에서 분을 읽음
    bData = kInPortByte( RTC_CMOSDATA );
    *pbMinute = RTC_BCDTOBINARY( bData );
    
    // CMOS 메모리 어드레스 레지스터(포트 0x70)에 초를 저장하는 레지스터 지정
    kOutPortByte( RTC_CMOSADDRESS, RTC_ADDRESS_SECOND );
    // CMOS 데이터 레지스터(포트 0x71)에서 초를 읽음
    bData = kInPortByte( RTC_CMOSDATA );
    *pbSecond = RTC_BCDTOBINARY( bData );
}

/**
 *  CMOS 메모리에서 RTC 컨트롤러가 저장한 현재 일자를 읽음
 */
void kReadRTCDate( WORD* pwYear, BYTE* pbMonth, BYTE* pbDayOfMonth, 
                   BYTE* pbDayOfWeek )
{
    BYTE bData;
    
    // CMOS 메모리 어드레스 레지스터(포트 0x70)에 연도를 저장하는 레지스터 지정
    kOutPortByte( RTC_CMOSADDRESS, RTC_ADDRESS_YEAR );
    // CMOS 데이터 레지스터(포트 0x71)에서 연도를 읽음
    bData = kInPortByte( RTC_CMOSDATA );
    *pwYear = RTC_BCDTOBINARY( bData ) + 2000;
    
    // CMOS 메모리 어드레스 레지스터(포트 0x70)에 월을 저장하는 레지스터 지정
    kOutPortByte( RTC_CMOSADDRESS, RTC_ADDRESS_MONTH );
    // CMOS 데이터 레지스터(포트 0x71)에서 월을 읽음
    bData = kInPortByte( RTC_CMOSDATA );
    *pbMonth = RTC_BCDTOBINARY( bData );
    
    // CMOS 메모리 어드레스 레지스터(포트 0x70)에 일을 저장하는 레지스터 지정
    kOutPortByte( RTC_CMOSADDRESS, RTC_ADDRESS_DAYOFMONTH );
    // CMOS 데이터 레지스터(포트 0x71)에서 일을 읽음
    bData = kInPortByte( RTC_CMOSDATA );
    *pbDayOfMonth = RTC_BCDTOBINARY( bData );
    
    // CMOS 메모리 어드레스 레지스터(포트 0x70)에 요일을 저장하는 레지스터 지정
    kOutPortByte( RTC_CMOSADDRESS, RTC_ADDRESS_DAYOFWEEK );
    // CMOS 데이터 레지스터(포트 0x71)에서 요일을 읽음
    bData = kInPortByte( RTC_CMOSDATA );
    *pbDayOfWeek = RTC_BCDTOBINARY( bData );
}

/**
 *  요일 값을 이용해서 해당 요일의 문자열을 반환
 */
char* kConvertDayOfWeekToString( BYTE bDayOfWeek )
{
    static char* vpcDayOfWeekString[ 8 ] = { "Error", "Sunday", "Monday", 
            "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
    
    // 요일 범위가 넘어가면 에러를 반환
    if( bDayOfWeek >= 8 )
    {
        return vpcDayOfWeekString[ 0 ];
    }
    
    // 요일을 반환
    return vpcDayOfWeekString[ bDayOfWeek ];
}
