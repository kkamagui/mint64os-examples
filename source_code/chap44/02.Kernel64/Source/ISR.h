/**
 *  file    ISR.h
 *  date    2009/01/24
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   인터럽트 서비스 루틴(ISR) 관련된 헤더 파일
 */

#ifndef __ISR_H__
#define __ISR_H__

////////////////////////////////////////////////////////////////////////////////
//
//  함수
//
////////////////////////////////////////////////////////////////////////////////
// 예외(Exception) 처리용 ISR
void kISRDivideError( void );
void kISRDebug( void );
void kISRNMI( void );
void kISRBreakPoint( void );
void kISROverflow( void );
void kISRBoundRangeExceeded( void );
void kISRInvalidOpcode();
void kISRDeviceNotAvailable( void );
void kISRDoubleFault( void );
void kISRCoprocessorSegmentOverrun( void );
void kISRInvalidTSS( void );
void kISRSegmentNotPresent( void );
void kISRStackSegmentFault( void );
void kISRGeneralProtection( void );
void kISRPageFault( void );
void kISR15( void );
void kISRFPUError( void );
void kISRAlignmentCheck( void );
void kISRMachineCheck( void );
void kISRSIMDError( void );
void kISRETCException( void );

// 인터럽트(Interrupt) 처리용 ISR
void kISRTimer( void );
void kISRKeyboard( void );
void kISRSlavePIC( void );
void kISRSerial2( void );
void kISRSerial1( void );
void kISRParallel2( void );
void kISRFloppy( void );
void kISRParallel1( void );
void kISRRTC( void );
void kISRReserved( void );
void kISRNotUsed1( void );
void kISRNotUsed2( void );
void kISRMouse( void );
void kISRCoprocessor( void );
void kISRHDD1( void );
void kISRHDD2( void );
void kISRETCInterrupt( void );

#endif /*__ISR_H__*/
