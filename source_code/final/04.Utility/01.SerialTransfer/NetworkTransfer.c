/**
 *  file    NetworkTransfer.c
 *  date    2009/06/06
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   네트워크로 데이터를 전송하는데 프로그램의 소스 파일
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>

// 기타 매크로
#define DWORD               unsigned int
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
    struct sockaddr_in stSocketAddr;
    int iSocket;
    BYTE bAck;
    DWORD dwDataLength;
    DWORD dwSentSize;
    DWORD dwTemp;
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
    // 네트워크 접속
    //--------------------------------------------------------------------------
    // 접속할 QEMU의 Address를 설정
    stSocketAddr.sin_family = AF_INET;
    stSocketAddr.sin_port = htons( 4444 );
    stSocketAddr.sin_addr.s_addr = inet_addr( "127.0.0.1" );

    // 소켓 생성 후, QEMU에 접속 시도
    iSocket = socket( AF_INET, SOCK_STREAM, 0 );
    if( connect( iSocket, ( struct sockaddr* ) &stSocketAddr, 
                 sizeof( stSocketAddr ) ) == -1 )
    {
        fprintf( stderr, "Socket Connect Error, IP 127.0.0.1, Port 4444\n" );
        return 0;
    }
    else
    {
        fprintf( stderr, "Socket Connect Success, IP 127.0.0.1, Port 4444\n" );
    }
    
    //--------------------------------------------------------------------------
    // 데이터 전송
    //--------------------------------------------------------------------------
    // 데이터 길이를 전송
    if( send( iSocket, &dwDataLength, 4, 0 ) != 4 )
    {
        fprintf( stderr, "Data Length Send Fail, [%d] Byte\n", dwDataLength );
        return 0;
    }
    else
    {
        fprintf( stderr, "Data Length Send Success, [%d] Byte\n", dwDataLength );
    }
    // Ack를 수신할 때까지 대기
    if( recv( iSocket, &bAck, 1, 0 ) != 1 )
    {
        fprintf( stderr, "Ack Receive Error\n" );
        return 0;
    }
    
    // 데이터를 전송
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
        if( send( iSocket, vcDataBuffer, dwTemp, 0 ) != dwTemp )
        {
            fprintf( stderr, "Socket Send Error\n" );
            return 0;
        }
        
        // Ack가 수신될 때까지 대기
        if( recv( iSocket, &bAck, 1, 0 ) != 1 )
        {
            fprintf( stderr, "Ack Receive Error\n" );
            return 0;
        }
        // 진행 상황 표시
        fprintf( stderr, "#" );
    }
    
    // 파일과 소켓을 닫음
    fclose( fp );
    close( iSocket );
    
    // 전송이 완료되었음을 표시하고 엔터 키를 대기
    fprintf( stderr, "\nSend Complete. Total Size [%d] Byte\n", dwSentSize );
    fprintf( stderr, "Press Enter Key To Exit\n" );
    getchar();
    
    return 0;
}
