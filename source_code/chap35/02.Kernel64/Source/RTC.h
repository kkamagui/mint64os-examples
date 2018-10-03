/**
 *  file    RTC.c
 *  date    2009/02/14
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   RTC 컨트롤러에 관련된 헤더 파일
 */

#ifndef __RTC_H__
#define __RTC_H__

#include "Types.h"

////////////////////////////////////////////////////////////////////////////////
//
// 매크로
//
////////////////////////////////////////////////////////////////////////////////
// I/O 포트
#define RTC_CMOSADDRESS         0x70
#define RTC_CMOSDATA            0x71

// CMOS 메모리 어드레스
#define RTC_ADDRESS_SECOND      0x00
#define RTC_ADDRESS_MINUTE      0x02
#define RTC_ADDRESS_HOUR        0x04
#define RTC_ADDRESS_DAYOFWEEK   0x06
#define RTC_ADDRESS_DAYOFMONTH  0x07
#define RTC_ADDRESS_MONTH       0x08
#define RTC_ADDRESS_YEAR        0x09

// BCD 포맷을 Binary로 변환하는 매크로
#define RTC_BCDTOBINARY( x )    ( ( ( ( x ) >> 4 ) * 10 ) + ( ( x ) & 0x0F ) )

////////////////////////////////////////////////////////////////////////////////
//
//  함수
//
////////////////////////////////////////////////////////////////////////////////
void kReadRTCTime( BYTE* pbHour, BYTE* pbMinute, BYTE* pbSecond );
void kReadRTCDate( WORD* pwYear, BYTE* pbMonth, BYTE* pbDayOfMonth, 
                   BYTE* pbDayOfWeek );
char* kConvertDayOfWeekToString( BYTE bDayOfWeek );

#endif /*__RTC_H__*/
