/**
 *  file    CacheManager.h
 *  date    2009/05/17
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   파일 시스템 캐시에 관련된 헤더 파일
 */

#ifndef __CACHEMANAGER_H__
#define __CACHEMANAGER_H__

#include "Types.h"

////////////////////////////////////////////////////////////////////////////////
//
// 매크로
//
////////////////////////////////////////////////////////////////////////////////
// 클러스터 링크 테이블 영역의 최대 캐시 버퍼의 개수
#define CACHE_MAXCLUSTERLINKTABLEAREACOUNT      16
// 데이터 영역의 최대 캐시 버퍼의 수
#define CACHE_MAXDATAAREACOUNT                  32
// 유효하지 않은 태그, 비어있는 캐시 버퍼를 나타내는 데 사용
#define CACHE_INVALIDTAG                        0xFFFFFFFF

// 캐시 테이블의 최대 개수. 클러스터 링크 테이블과 데이터 영역만 있으므로 2로 설정
#define CACHE_MAXCACHETABLEINDEX                2
// 클러스터 링크 테이블 영역의 인덱스
#define CACHE_CLUSTERLINKTABLEAREA              0
// 데이터 영역의 인덱스
#define CACHE_DATAAREA                          1

////////////////////////////////////////////////////////////////////////////////
//
// 구조체
//
////////////////////////////////////////////////////////////////////////////////
// 파일 시스템 캐시를 구성하는 캐시 버퍼의 구조체
typedef struct kCacheBufferStruct
{
    // 캐시와 대응하는 클러스터 링크 테이블 영역이나 데이터 영역의 인덱스
    DWORD dwTag;

    // 캐시 버퍼에 접근한 시간
    DWORD dwAccessTime;

    // 데이터의 내용이 변경되었는지 여부
    BOOL bChanged;

    // 데이터 버퍼
    BYTE* pbBuffer;
} CACHEBUFFER;

// 파일 시스템 캐시를 관리하는 캐시 매니저의 구조체
typedef struct kCacheManagerStruct
{
    // 클러스터 링크 테이블 영역과 데이터 영역의 접근 시간 필드
    DWORD vdwAccessTime[ CACHE_MAXCACHETABLEINDEX ];

    // 클러스터 링크 테이블 영역과 데이터 영역의 데이터 버퍼
    BYTE* vpbBuffer[ CACHE_MAXCACHETABLEINDEX ];

    // 클러스터 링크 테이블 영역과 데이터 영역의 캐시 버퍼
    // 두 값 중에서 큰 값만큼 생성해야 함
    CACHEBUFFER vvstCacheBuffer[ CACHE_MAXCACHETABLEINDEX ][ CACHE_MAXDATAAREACOUNT ];

    // 캐시 버퍼의 최댓값을 저장
    DWORD vdwMaxCount[ CACHE_MAXCACHETABLEINDEX ];
} CACHEMANAGER;

////////////////////////////////////////////////////////////////////////////////
//
// 함수
//
////////////////////////////////////////////////////////////////////////////////
BOOL kInitializeCacheManager( void );
CACHEBUFFER* kAllocateCacheBuffer( int iCacheTableIndex );
CACHEBUFFER* kFindCacheBuffer( int iCacheTableIndex, DWORD dwTag );
CACHEBUFFER* kGetVictimInCacheBuffer( int iCacheTableIndex );
void kDiscardAllCacheBuffer( int iCacheTableIndex );
BOOL kGetCacheBufferAndCount( int iCacheTableIndex, 
        CACHEBUFFER** ppstCacheBuffer, int* piMaxCount );

static void kCutDownAccessTime( int iCacheTableIndex );

#endif /*__CACHEMANAGER_H__*/
