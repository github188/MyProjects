/* MD5.CPP - RSA Data Security, Inc., MD5 message-digest algorithm
 */

/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.
 */

#include "StdAfx.h"
//#include "Shareaza.h"
#include "MD5.h"
#include "tchar.h"

//////////////////////////////////////////////////////////////////////
// CMD5 construction
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

CMD5::CMD5()
{
	Reset();
}

CMD5::~CMD5()
{
}


static unsigned char MD5_PADDING[64] = {
  0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

extern "C" void MD5_Add_p5(CMD5*, LPCVOID pData, DWORD nLength);

//////////////////////////////////////////////////////////////////////
// CMD5 reset m_nState

void CMD5::Reset()
{
	m_nCount[0] = m_nCount[1] = 0;
	/* Load magic initialization constants. */
	m_nState[0] = 0x67452301;
	m_nState[1] = 0xefcdab89;
	m_nState[2] = 0x98badcfe;
	m_nState[3] = 0x10325476;
}

//////////////////////////////////////////////////////////////////////
// CMD5 add data

void CMD5::Add(LPCVOID pData, size_t nLength)
{
	MD5_Add_p5(this, pData, nLength);
}

//////////////////////////////////////////////////////////////////////
// CMD5 finish hash operation

void CMD5::Finish()
{
	unsigned int bits[2], index = 0;
	// Save number of bits
	bits[1] = ( m_nCount[1] << 3 ) + ( m_nCount[0] >> 29);
	bits[0] = m_nCount[0] << 3;
	// Pad out to 56 mod 64.
	index = (unsigned int)(m_nCount[0] & 0x3f);
	MD5_Add_p5(this, MD5_PADDING, (index < 56) ? (56 - index) : (120 - index) );
	// Append length (before padding)
	MD5_Add_p5(this, bits, 8 );
}

//////////////////////////////////////////////////////////////////////
// CMD5 get hash bytes

void CMD5::GetHash(SMD5* pHash)
{
	/* Store m_nState in digest */
	memcpy(pHash, m_nState, 16);
}

//////////////////////////////////////////////////////////////////////
 //CMD5 convert hash to string

tstring CMD5::HashToString(const SMD5* pHash, BOOL bURN)
 {
	TCHAR buf[255] ;
	_stprintf_s(buf,bURN ?
 		TEXT("md5:%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x") :
 		TEXT("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"),
 		pHash->n[0], pHash->n[1], pHash->n[2], pHash->n[3],
 		pHash->n[4], pHash->n[5], pHash->n[6], pHash->n[7],
 		pHash->n[8], pHash->n[9], pHash->n[10], pHash->n[11],
 		pHash->n[12], pHash->n[13], pHash->n[14], pHash->n[15] );
	tstring str(buf);
 	
 	return str;
 }

//////////////////////////////////////////////////////////////////////
// CMD5 parse from string

BOOL CMD5::HashFromString(LPCTSTR pszHash, SMD5* pMD5)
{
	if ( _tcslen( pszHash ) < 32 ) return FALSE;
	
	BYTE* pOut = (BYTE*)pMD5;
	
	for ( int nPos = 16 ; nPos ; nPos--, pOut++ )
	{
		if ( *pszHash >= '0' && *pszHash <= '9' )
			*pOut = ( *pszHash - '0' ) << 4;
		else if ( *pszHash >= 'A' && *pszHash <= 'F' )
			*pOut = ( *pszHash - 'A' + 10 ) << 4;
		else if ( *pszHash >= 'a' && *pszHash <= 'f' )
			*pOut = ( *pszHash - 'a' + 10 ) << 4;
		pszHash++;
		if ( *pszHash >= '0' && *pszHash <= '9' )
			*pOut |= ( *pszHash - '0' );
		else if ( *pszHash >= 'A' && *pszHash <= 'F' )
			*pOut |= ( *pszHash - 'A' + 10 );
		else if ( *pszHash >= 'a' && *pszHash <= 'f' )
			*pOut |= ( *pszHash - 'a' + 10 );
		pszHash++;
	}
	
	return TRUE;
}

BOOL CMD5::HashFromURN(LPCTSTR pszHash, SMD5* pMD5)
{
	if ( pszHash == NULL ) return FALSE;
	
	size_t nLen = _tcslen( pszHash );
	
	if ( nLen >= 8 + 32 && _tcsnicmp( pszHash, _T("urn:md5:"), 8 ) == 0 )
	{
		return HashFromString( pszHash + 8, pMD5 );
	}
	else if ( nLen >= 4 + 32 && _tcsnicmp( pszHash, _T("md5:"), 4 ) == 0 )
	{
		return HashFromString( pszHash + 4, pMD5 );
	}
	
	return FALSE;
}
void CHmacMD5::Reset()
{
	CMD5::Reset();
	b_reset_ = true;
}

void CHmacMD5::Add(LPCVOID pData, size_t nLength)
{
	if(nLength==0)
		return;
	if(b_reset_)
	{
		CMD5::Add( m_key, sizeof(m_key));
		b_reset_ = false;
	}
	CMD5::Add( pData, nLength);
}

void CHmacMD5::Finish()
{
	CMD5::Add( m_key, sizeof(m_key));
	CMD5::Finish();
}

CHmacMD5::CHmacMD5(u_char *key,size_t keylen):CMD5(),b_reset_(true)
{
	CMD5::p md5_ = CMD5::Create();
	md5_->Add(key,keylen);
	md5_->Add(key,keylen);
	md5_->Finish();
	SMD5 key_resource;
	md5_->GetHash(&key_resource);

	memset(m_key,0,sizeof(m_key));
	memcpy(m_key,(u_char *)&key_resource,min(sizeof(SMD5),DEFAULT_KEY_LEN));
}

CHmacMD5::~CHmacMD5()
{
}
