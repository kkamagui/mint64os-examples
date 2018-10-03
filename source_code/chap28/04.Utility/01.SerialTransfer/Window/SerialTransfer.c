/**
 *  file    SerialTransferForWindow.c
 *  date    2009/06/06
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui 
 *  brief   시리얼 포트로 데이터를 전송하는데 프로그램의 소스 파일
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <windows.h>

// 기타 매크로
#define DWORD               unsigned long
#define BYTE                unsigned char
#define MIN( x, y )         ( ( ( x ) < ( y ) ) ? ( x ) : ( y ) )

// 시리얼 포트 FIFO의 최대 크기
#define SERIAL_FIFOMAXSIZE  16

/**
 *  main 함수
 */
int main( int argc, char** argv )
{
    char vcFileName[ 256 ];
    char vcDataBuffer[ SERIAL_FIFOMAXSIZE ];
    HANDLE hSerialPort;
    DCB stDCB;
    BYTE bAck;
    DWORD dwDataLength;
    DWORD dwSentSize;
    DWORD dwTemp;
	DWORD dwProcessedSize;
    FILE* fp;
    
    //--------------------------------------------------------------------------
    // 파일 열기
    //--------------------------------------------------------------------------
    // 파일 이름을 입력하지 않았으면 파일 이름을 입력 받음
    if( argc < 2 )
    {
        fprintf( stderr, "Input File Name: " );
        gets( vcFileName );
    }
    // 파일 이름을 실행 시에 입력했다면 복사함
    else
    {
        strcpy( vcFileName, argv[ 1 ] );
    }

    // 파일 열기 시도
    fp = fopen( vcFileName, "rb" );
    if( fp == NULL )
    {
        fprintf( stderr, "%s File Open Error\n", vcFileName );
        return 0;
    }
    
    // fseek로 파일 끝으로 이동하여 파일의 길이를 측정한 뒤, 다시 파일의 처음으로 이동
    fseek( fp, 0, SEEK_END );
    dwDataLength = ftell( fp );
    fseek( fp, 0, SEEK_SET );
    fprintf( stderr, "File Name %s, Data Length %d Byte\n", vcFileName, 
            dwDataLength );
    
    //--------------------------------------------------------------------------
    // 시리얼 포트 접속
    //--------------------------------------------------------------------------
    // 시리얼 포트 열기
    hSerialPort = CreateFile( "COM1", GENERIC_READ|GENERIC_WRITE, 0, 0, 
                              OPEN_EXISTING, 0, 0 );
    if( hSerialPort == INVALID_HANDLE_VALUE )
    {
        fprintf( stderr, "COM1 Open Error\n" );
        return 0;
    }
    else
    {
        fprintf( stderr, "COM1 Open Success\n" );
    }
    
    // 통신 속도, 패리티, 정지 비트 설정
    memset( &stDCB, 0, sizeof( stDCB ) );
    stDCB.fBinary = TRUE;           // Binary Transfer
    stDCB.BaudRate = CBR_115200 ;   // 115200 bps
    stDCB.ByteSize = 8;             // 8bit Data
    stDCB.Parity = 0;               // No Parity
    stDCB.StopBits = ONESTOPBIT;    // 1 Stop Bit
    if( SetCommState( hSerialPort, &stDCB ) == FALSE )
    {
        fprintf( stderr, "COM1 Set Value Fail\n" );
        return 0;
    }
    
    //--------------------------------------------------------------------------
    // 데이터 전송
    //--------------------------------------------------------------------------
    // 데이터 길이를 전송
    if( ( WriteFile( hSerialPort, &dwDataLength, 4, &dwProcessedSize, NULL ) == FALSE ) ||
		( dwProcessedSize != 4 ) )
    {
        fprintf( stderr, "Data Length Send Fail, [%d] Byte\n", dwDataLength );
        return 0;
    }
    else
    {
        fprintf( stderr, "Data Length Send Success, [%d] Byte\n", dwDataLength );
    }
    // Ack가 수신될 때까지 대기
    if( ( ReadFile( hSerialPort, &bAck, 1, &dwProcessedSize, 0 ) == FALSE ) ||
        ( dwProcessedSize != 1 ) )
    {
        fprintf( stderr, "Ack Receive Error\n" );
        return 0;
    }    
    
    // 데이터 전송
    fprintf( stderr, "Now Data Transfer..." );
    dwSentSize = 0;
    while( dwSentSize < dwDataLength )
    {
        // 남은 크기와 FIFO의 최대 크기 중에서 작은 것을 선택
        dwTemp = MIN( dwDataLength - dwSentSize, SERIAL_FIFOMAXSIZE );
        dwSentSize += dwTemp;
        
        if( fread( vcDataBuffer, 1, dwTemp, fp ) != dwTemp )
        {
            fprintf( stderr, "File Read Error\n" );
            return 0;
        }
        
        // 데이터를 전송
        if( ( WriteFile( hSerialPort, vcDataBuffer, dwTemp, &dwProcessedSize, 0 ) == FALSE ) ||
			( dwProcessedSize != dwTemp ) )
        {
            fprintf( stderr, "Socket Send Error\n" );
            return 0;
        }
        
        // Ack가 수신될 때까지 대기
        if( ( ReadFile( hSerialPort, &bAck, 1, &dwProcessedSize, 0 ) == FALSE ) ||
			( dwProcessedSize != 1 ) )
        {
            fprintf( stderr, "Ack Receive Error\n" );
            return 0;
        }
        // 진행 상황 표시
        fprintf( stderr, "#" );
    }
    
    // 파일과 시리얼을 닫음
    fclose( fp );
    CloseHandle( hSerialPort );
    
    // 전송이 완료되었음을 표시하고 엔터 키를 대기
    fprintf( stderr, "\nSend Complete. Total Size [%d] Byte\n", dwSentSize );
    fprintf( stderr, "Press Enter Key To Exit\n" );
    getchar();
    
    return 0;
}
