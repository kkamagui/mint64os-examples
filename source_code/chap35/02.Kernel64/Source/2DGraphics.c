/**
 *  file    2DGraphics.h
 *  date    2009/09/5
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   2D Graphic에 대한 소스 파일
 */

#include "2DGraphics.h"
#include "VBE.h"
#include "Font.h"
#include "Utility.h"

/**
 *  점 그리기
 */
inline void kDrawPixel( int iX, int iY, COLOR stColor )
{
    VBEMODEINFOBLOCK* pstModeInfo;
    
    // 모드 정보 블록 반환
    pstModeInfo = kGetVBEModeInfoBlock();
    
    // Physical Address를 COLOR 타입으로 하면 픽셀 오프셋으로 계산 가능
    *( ( ( COLOR* ) ( QWORD ) pstModeInfo->dwPhysicalBasePointer ) + 
            pstModeInfo->wXResolution * iY + iX ) = stColor;
}

/**
 *  직선 그리기
 */
void kDrawLine( int iX1, int iY1, int iX2, int iY2, COLOR stColor )
{
    int iDeltaX, iDeltaY;
    int iError = 0;
    int iDeltaError;
    int iX, iY;
    int iStepX, iStepY;
    
    // 변화량 계산
    iDeltaX = iX2 - iX1;
    iDeltaY = iY2 - iY1;

    // X축 변화량에 따라 X축 증감 방향 계산
    if( iDeltaX < 0 ) 
    {
        iDeltaX = -iDeltaX; 
        iStepX = -1; 
    } 
    else 
    { 
        iStepX = 1; 
    }

    // Y축 변화량에 따라 Y축 증감 방향 계산 
    if( iDeltaY < 0 ) 
    {
        iDeltaY = -iDeltaY; 
        iStepY = -1; 
    } 
    else 
    {
        iStepY = 1; 
    }

    // X축 변화량이 Y축 변화량보다 크다면 X축을 중심으로 직선을 그림
    if( iDeltaX > iDeltaY )
    {
        // 기울기로 매 픽셀마다 더해줄 오차, Y축 변화량의 2배
        // 시프트 연산으로 * 2를 대체
        iDeltaError = iDeltaY << 1;
        iY = iY1;
        for( iX = iX1 ; iX != iX2 ; iX += iStepX )
        {
            // 점 그리기
            kDrawPixel( iX, iY, stColor );

            // 오차 누적
            iError += iDeltaError;

            // 누적된 오차가 X축 변화량보다 크면 위에 점을 선택하고 오차를 위에 점을
            // 기준으로 갱신
            if( iError >= iDeltaX )
            {
                iY += iStepY;
                // X축의 변화량의 2배를 빼줌
                // 시프트 연산으로 *2를 대체
                iError -= iDeltaX << 1;
            }
        }
        // iX == iX2인 최종 위치에 점 그리기
        kDrawPixel( iX, iY, stColor );
    }
    // Y축 변화량이 X축 변화량보다 크거나 같다면 Y축을 중심으로 직선을 그림
    else
    {
        // 기울기로 매 픽셀마다 더해줄 오차, X축 변화량의 2배
        // 시프트 연산으로 * 2를 대체
        iDeltaError = iDeltaX << 1;
        iX = iX1;
        for( iY = iY1 ; iY != iY2 ; iY += iStepY )
        {
            // 점 그리기
            kDrawPixel( iX, iY, stColor );

            // 오차 누적
            iError += iDeltaError;

            // 누적된 오차가 Y축 변화량보다 크면 위에 점을 선택하고 오차를 위에 점을
            // 기준으로 갱신
            if( iError >= iDeltaY )
            {
                iX += iStepX;
                // Y축의 변화량의 2배를 빼줌
                // 시프트 연산으로 *2를 대체
                iError -= iDeltaY << 1;
            }
        }

        // iY == iY2인 최종 위치에 점 그리기
        kDrawPixel( iX, iY, stColor );
    }
}

/**
 *  사각형 그리기
 */
void kDrawRect( int iX1, int iY1, int iX2, int iY2, COLOR stColor, BOOL bFill )
{
    int iWidth;
    int iTemp;
    int iY;
    int iStepY;
    COLOR* pstVideoMemoryAddress;
    VBEMODEINFOBLOCK* pstModeInfo;

    // 채움 여부에 따라 코드를 분리
    if( bFill == FALSE )
    {
        // 네 점을 이웃한 것끼리 직선으로 연결
        kDrawLine( iX1, iY1, iX2, iY1, stColor );
        kDrawLine( iX1, iY1, iX1, iY2, stColor );
        kDrawLine( iX2, iY1, iX2, iY2, stColor );
        kDrawLine( iX1, iY2, iX2, iY2, stColor );
    }
    else
    {
        // VBE 모드 정보 블록을 반환
        pstModeInfo = kGetVBEModeInfoBlock();
        
        // 비디오 메모리 어드레스 계산
        pstVideoMemoryAddress = 
            ( COLOR* ) ( ( QWORD ) pstModeInfo->dwPhysicalBasePointer );
        
        // kMemSetWord() 함수는 X의 값이 증가하는 방향으로 데이터를 채우므로
        // x1과 x2를 비교하여 두 점을 교환
        if( iX2 < iX1 )
        {
            iTemp = iX1;
            iX1 = iX2;
            iX2 = iTemp;
            
            iTemp = iY1;
            iY1 = iY2;
            iY2 = iTemp;
        }
        
        // 사각형의 X축 길이를 계산
        iWidth = iX2 - iX1 + 1;
        
        // y1과 y2를 비교하여 매 회마다 증가시킬 Y 값을 저장
        if( iY1 <= iY2 )
        {
            iStepY = 1;         
        }
        else
        {
            iStepY = -1;
        }
        
        // 출력을 시작할 비디오 메모리 어드레스를 계산
        pstVideoMemoryAddress += iY1 * pstModeInfo->wXResolution + iX1;
        
        // 루프를 돌면서 각 Y축마다 값을 채움
        for( iY = iY1 ; iY != iY2 ; iY += iStepY )
        {
            // 비디오 메모리에 사각형의 너비만큼 출력
            kMemSetWord( pstVideoMemoryAddress, stColor, iWidth );
            
            // 출력할 비디오 메모리 어드레스 갱신
            // x, y 좌표로 매번 비디오 메모리 어드레스를 계산하는 것을 피하려고
            // X축 해상도를 이용하여 다음 Y의 어드레스를 계산 
            if( iStepY >= 0 )
            {
                pstVideoMemoryAddress += pstModeInfo->wXResolution;
            }
            else
            {
                pstVideoMemoryAddress -= pstModeInfo->wXResolution;
            }
        }
        
        // 비디오 메모리에 사각형의 너비만큼 출력, 마지막 줄 출력
        kMemSetWord( pstVideoMemoryAddress, stColor, iWidth );
    }
}

/**
 *  원 그리기
 */
void kDrawCircle( int iX, int iY, int iRadius, COLOR stColor, BOOL bFill )
{
    int iCircleX, iCircleY;
    int iDistance;
    
    // 반지름이 0보다 작다면 그릴 필요 없음
    if( iRadius < 0 )
    {
        return ;
    }
    
    // (0, R)인 좌표에서 시작
    iCircleY = iRadius;

    // 채움 여부에 따라 시작점을 그림
    if( bFill == FALSE )
    {
        // 시작점은 네 접점 모두 그림
        kDrawPixel( 0 + iX, iRadius + iY, stColor );
        kDrawPixel( 0 + iX, -iRadius + iY, stColor );
        kDrawPixel( iRadius + iX, 0 + iY, stColor );
        kDrawPixel( -iRadius + iX, 0 + iY, stColor );
    }
    else
    {
        // 시작 직선은 X축과 Y축 모두 그림
        kDrawLine( 0 + iX, iRadius + iY, 0 + iX, -iRadius + iY, stColor );
        kDrawLine( iRadius + iX, 0 + iY, -iRadius + iX, 0 + iY, stColor );
    }
    
    // 최초 시작점의 중심점과 원의 거리
    iDistance = -iRadius;

    // 원 그리기
    for( iCircleX = 1 ; iCircleX <= iCircleY ; iCircleX++ )
    {
        // 원에서 떨어진 거리 계산
        // 시프트 연산으로 * 2를 대체
        iDistance += ( iCircleX << 1 ) - 1;  //2 * iCircleX - 1;
                    
        // 중심점이 원의 외부에 있으면 아래에 있는 점 선택
        if( iDistance >= 0 )
        {
            iCircleY--;
            
            // 새로운 점에서 다시 원과 거리 계산
            // 시프트 연산으로 * 2를 대체
            iDistance += ( -iCircleY << 1 ) + 2; //-2 * iCircleY + 2;
        }
        
        // 채움 여부에 따라 그림
        if( bFill == FALSE )
        {
            // 8 방향 모두 점 그림
            kDrawPixel( iCircleX + iX, iCircleY + iY, stColor );
            kDrawPixel( iCircleX + iX, -iCircleY + iY, stColor );
            kDrawPixel( -iCircleX + iX, iCircleY + iY, stColor );
            kDrawPixel( -iCircleX + iX, -iCircleY + iY, stColor );
            kDrawPixel( iCircleY + iX, iCircleX + iY, stColor );
            kDrawPixel( iCircleY + iX, -iCircleX + iY, stColor );
            kDrawPixel( -iCircleY + iX, iCircleX + iY, stColor );
            kDrawPixel( -iCircleY + iX, -iCircleX + iY, stColor );
        }
        else
        {
            // 대칭되는 점을 찾아 X축에 평행한 직선을 그어 채워진 원을 그림
            // 평행선을 그리는 것은 사각형 그리기 함수로 빠르게 처리할 수 있음
            kDrawRect( -iCircleX + iX, iCircleY + iY, 
                iCircleX + iX, iCircleY + iY, stColor, TRUE );
            kDrawRect( -iCircleX + iX, -iCircleY + iY, 
                iCircleX + iX, -iCircleY + iY, stColor, TRUE );
            kDrawRect( -iCircleY + iX, iCircleX + iY, 
                iCircleY + iX, iCircleX + iY, stColor, TRUE );
            kDrawRect( -iCircleY + iX, -iCircleX + iY, 
                iCircleY + iX, -iCircleX + iY, stColor, TRUE );
        }
    }
}

/**
 *  문자 출력
 */
void kDrawText( int iX, int iY, COLOR stTextColor, COLOR stBackgroundColor, 
        const char* pcString, int iLength )
{
    int iCurrentX, iCurrentY;
    int i, j, k;
    BYTE bBitmask;
    int iBitmaskStartIndex;
    VBEMODEINFOBLOCK* pstModeInfo;
    COLOR* pstVideoMemoryAddress;

    // VBE 모드 정보 블록을 반환
    pstModeInfo = kGetVBEModeInfoBlock();
    
    // 비디오 메모리 어드레스 계산
    pstVideoMemoryAddress = 
        ( COLOR* ) ( ( QWORD ) pstModeInfo->dwPhysicalBasePointer );
    
    // 문자를 출력하는 X좌표
    iCurrentX = iX;
    
    // 문자의 개수만큼 반복
    for( k = 0 ; k < iLength ; k++ )
    {
        // 문자를 출력할 위치의 Y좌표를 구함
        iCurrentY = iY * pstModeInfo->wXResolution;

        // 비트맵 폰트 데이터에서 문자 비트맵이 시작하는 위치를 계산
        // 1바이트 * FONT_HEIGHT로 구성되어 있으므로 문자의 비트맵 위치는
        // 아래와 같이 계산 가능
        iBitmaskStartIndex = pcString[ k ] * FONT_ENGLISHHEIGHT;
        
        // 문자 출력
        for( j = 0 ; j < FONT_ENGLISHHEIGHT ; j++ )
        {
            // 이번 라인에서 출력할 폰트 비트맵 
            bBitmask = g_vucEnglishFont[ iBitmaskStartIndex++ ];
            
            // 현재 라인 출력
            for( i = 0 ; i < FONT_ENGLISHWIDTH ; i++ )
            {
                // 비트가 설정되어있으면 화면에 문자색을 표시
                if( bBitmask & ( 0x01 << ( FONT_ENGLISHWIDTH - i - 1 ) ) )
                {
                    pstVideoMemoryAddress[ iCurrentY + iCurrentX + i ] = stTextColor;
                }
                // 비트가 설정되어있지 않으면 화면에 배경색을 표시
                else
                {
                    pstVideoMemoryAddress[ iCurrentY + iCurrentX + i ] = stBackgroundColor;
                }
            }
            
            // 다음 라인으로 이동해야 하므로, 현재 Y좌표에 화면의 너비만큼 더해줌
            iCurrentY += pstModeInfo->wXResolution;
        }
        
        // 문자 하나를 다 출력했으면 폰트의 너비만큼 X 좌표를 이동하여 다음 문자를 출력
        iCurrentX += FONT_ENGLISHWIDTH;
    }
}
