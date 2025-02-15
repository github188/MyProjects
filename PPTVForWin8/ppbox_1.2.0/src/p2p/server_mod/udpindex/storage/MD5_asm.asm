; #####################################################################################################################
;
; MD5_asm.asm
;
; Copyright (c) Shareaza Development Team, 2002-2005.
; This file is part of SHAREAZA (www.shareaza.com)
;
; Shareaza is free software; you can redistribute it
; and/or modify it under the terms of the GNU General Public License
; as published by the Free Software Foundation; either version 2 of
; the License, or (at your option) any later version.
;
; Shareaza is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with Shareaza; if not, write to the Free Software
; Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
;
; #####################################################################################################################
;
; MD5_asm - Implementation of MD5 for x86 - use together with MD5.cpp and MD5.h
;
; created              7.7.2004         by Camper
;
; last modified        20.7.2004        by Camper
;
; The integration into other projects than Shareaza is expressivly encouraged. Feel free to contact me about it.
;
; #####################################################################################################################

                        .586p
                        .model      flat, C 
                        option      casemap:none                    ; case sensitive
                        option      prologue:none                   ; we generate our own entry/exit code
                        option      epilogue:none

; #####################################################################################################################

m_nCount0               equ         4
m_nCount1               equ         8

m_nState0               equ         12                              ; offsets as laid out in MD5.h
m_nState1               equ         16
m_nState2               equ         20
m_nState3               equ         24

m_pBuffer               equ         28

; Some magic numbers for Transform...
MD5_S11                 equ         7
MD5_S12                 equ         12
MD5_S13                 equ         17
MD5_S14                 equ         22
MD5_S21                 equ         5
MD5_S22                 equ         9
MD5_S23                 equ         14
MD5_S24                 equ         20
MD5_S31                 equ         4
MD5_S32                 equ         11
MD5_S33                 equ         16
MD5_S34                 equ         23
MD5_S41                 equ         6
MD5_S42                 equ         10
MD5_S43                 equ         15
MD5_S44                 equ         21

MD5FF                   MACRO       a:REQ,b:REQ,c:REQ,d:REQ,count:REQ,s:REQ,ac:REQ
; a = b+(a+x[count]+ac+((b&c)|(~b&d)))rol s
                        mov         reg_temp1, b
                        mov         reg_temp2, b
                        add         a, [reg_base+count*4]
reg_t                   textequ     reg_temp1
reg_temp1               textequ     b                                   ; an attempt to improve instruction pairing
b                       textequ     reg_t
                        not         reg_temp1
                        and         reg_temp2, c
                        add         a, ac
                        and         reg_temp1, d
                        or          reg_temp1, reg_temp2
                        add         a, reg_temp1
                        rol         a, s
                        add         a, b
                        ENDM
                        
MD5GG                   MACRO       a:REQ,b:REQ,c:REQ,d:REQ,count:REQ,s:REQ,ac:REQ
; a = b+(a+x[count]+ac+((d&b)|(~d&c)))rol s
                        mov         reg_temp1, d
                        mov         reg_temp2, d
                        add         a, [reg_base+count*4]
reg_t                   textequ     reg_temp1
reg_temp1               textequ     d
d                       textequ     reg_t
                        not         reg_temp1
                        and         reg_temp2, b
                        add         a, ac
                        and         reg_temp1, c
                        or          reg_temp1, reg_temp2
                        add         a, reg_temp1
                        rol         a, s
                        add         a, b
                        ENDM

MD5HH                   MACRO       a:REQ,b:REQ,c:REQ,d:REQ,count:REQ,s:REQ,ac:REQ
; a = b+(a+x[count]+ac+(b^c^d)) rol s
                        mov         reg_temp1, b
                        add         a, [reg_base+count*4]
reg_t                   textequ     reg_temp1
reg_temp1               textequ     b                                   ; an attempt to improve instruction pairing
b                       textequ     reg_t
                        xor         reg_temp1, c
                        add         a, ac
                        xor         reg_temp1, d
                        add         a, reg_temp1
                        rol         a, s
                        add         a, b
                        ENDM

MD5II                   MACRO       a:REQ,b:REQ,c:REQ,d:REQ,count:REQ,s:REQ,ac:REQ
; a = b+(a+x[count]+ac+(c^(~d|b))) rol s
                        mov         reg_temp1, d
                        add         a, [reg_base+count*4]
reg_t                   textequ     reg_temp1
reg_temp1               textequ     d
d                       textequ     reg_t
                        not         reg_temp1
                        or          reg_temp1, b
                        add         a, ac
                        xor         reg_temp1, c
                        add         a, reg_temp1
                        rol         a, s
                        add         a, b
                        ENDM

                        .code

MD5_Transform_p5        PROC                                            ; we expect ebp to point to the Data stream
                                                                        ; all other registers (eax,ebx,ecx,edx,esi,edi) will be destroyed
__this                  textequ     <[esp+32+2*4]>                      ; 1*pusha+2*call

; set alias for registers
reg_a                   textequ     <eax>
reg_b                   textequ     <ebx>
reg_c                   textequ     <ecx>
reg_d                   textequ     <edx>
reg_temp1               textequ     <esi>
reg_temp2               textequ     <edi>
reg_base                textequ     <ebp>

                        mov         reg_temp1, __this
                        mov         reg_a, [reg_temp1+m_nState0]
                        mov         reg_b, [reg_temp1+m_nState1]
                        mov         reg_c, [reg_temp1+m_nState2]
                        mov         reg_d, [reg_temp1+m_nState3]

; round 1

                        MD5FF       reg_a, reg_b, reg_c, reg_d,  0, MD5_S11,0D76AA478H  ;  1
                        MD5FF       reg_d, reg_a, reg_b, reg_c,  1, MD5_S12,0E8C7B756H  ;  2
                        MD5FF       reg_c, reg_d, reg_a, reg_b,  2, MD5_S13, 242070DBH  ;  3
                        MD5FF       reg_b, reg_c, reg_d, reg_a,  3, MD5_S14,0C1BDCEEEH  ;  4
                        MD5FF       reg_a, reg_b, reg_c, reg_d,  4, MD5_S11,0F57C0FAFH  ;  5
                        MD5FF       reg_d, reg_a, reg_b, reg_c,  5, MD5_S12, 4787C62AH  ;  6
                        MD5FF       reg_c, reg_d, reg_a, reg_b,  6, MD5_S13,0A8304613H  ;  7
                        MD5FF       reg_b, reg_c, reg_d, reg_a,  7, MD5_S14,0FD469501H  ;  8
                        MD5FF       reg_a, reg_b, reg_c, reg_d,  8, MD5_S11, 698098D8H  ;  9
                        MD5FF       reg_d, reg_a, reg_b, reg_c,  9, MD5_S12, 8B44F7AFH  ; 10
                        MD5FF       reg_c, reg_d, reg_a, reg_b, 10, MD5_S13,0FFFF5BB1H  ; 11
                        MD5FF       reg_b, reg_c, reg_d, reg_a, 11, MD5_S14, 895CD7BEH  ; 12
                        MD5FF       reg_a, reg_b, reg_c, reg_d, 12, MD5_S11, 6B901122H  ; 13
                        MD5FF       reg_d, reg_a, reg_b, reg_c, 13, MD5_S12,0FD987193H  ; 14
                        MD5FF       reg_c, reg_d, reg_a, reg_b, 14, MD5_S13,0A679438EH  ; 15
                        MD5FF       reg_b, reg_c, reg_d, reg_a, 15, MD5_S14, 49B40821H  ; 16

; round 2

                        MD5GG       reg_a, reg_b, reg_c, reg_d,  1, MD5_S21,0F61E2562H  ; 17
                        MD5GG       reg_d, reg_a, reg_b, reg_c,  6, MD5_S22,0C040B340H  ; 18
                        MD5GG       reg_c, reg_d, reg_a, reg_b, 11, MD5_S23, 265E5A51H  ; 19
                        MD5GG       reg_b, reg_c, reg_d, reg_a,  0, MD5_S24,0E9B6C7AAH  ; 20
                        MD5GG       reg_a, reg_b, reg_c, reg_d,  5, MD5_S21,0D62F105DH  ; 21
                        MD5GG       reg_d, reg_a, reg_b, reg_c, 10, MD5_S22,  2441453H  ; 22
                        MD5GG       reg_c, reg_d, reg_a, reg_b, 15, MD5_S23,0D8A1E681H  ; 23
                        MD5GG       reg_b, reg_c, reg_d, reg_a,  4, MD5_S24,0E7D3FBC8H  ; 24
                        MD5GG       reg_a, reg_b, reg_c, reg_d,  9, MD5_S21, 21E1CDE6H  ; 25
                        MD5GG       reg_d, reg_a, reg_b, reg_c, 14, MD5_S22,0C33707D6H  ; 26
                        MD5GG       reg_c, reg_d, reg_a, reg_b,  3, MD5_S23,0F4D50D87H  ; 27
                        MD5GG       reg_b, reg_c, reg_d, reg_a,  8, MD5_S24, 455A14EDH  ; 28
                        MD5GG       reg_a, reg_b, reg_c, reg_d, 13, MD5_S21,0A9E3E905H  ; 29
                        MD5GG       reg_d, reg_a, reg_b, reg_c,  2, MD5_S22,0FCEFA3F8H  ; 30
                        MD5GG       reg_c, reg_d, reg_a, reg_b,  7, MD5_S23, 676F02D9H  ; 31
                        MD5GG       reg_b, reg_c, reg_d, reg_a, 12, MD5_S24, 8D2A4C8AH  ; 32

; round 3

                        MD5HH       reg_a, reg_b, reg_c, reg_d,  5, MD5_S31,0FFFA3942H  ; 33
                        MD5HH       reg_d, reg_a, reg_b, reg_c,  8, MD5_S32, 8771F681H  ; 34
                        MD5HH       reg_c, reg_d, reg_a, reg_b, 11, MD5_S33, 6D9D6122H  ; 35
                        MD5HH       reg_b, reg_c, reg_d, reg_a, 14, MD5_S34,0FDE5380CH  ; 36
                        MD5HH       reg_a, reg_b, reg_c, reg_d,  1, MD5_S31,0A4BEEA44H  ; 37
                        MD5HH       reg_d, reg_a, reg_b, reg_c,  4, MD5_S32, 4BDECFA9H  ; 38
                        MD5HH       reg_c, reg_d, reg_a, reg_b,  7, MD5_S33,0F6BB4B60H  ; 39
                        MD5HH       reg_b, reg_c, reg_d, reg_a, 10, MD5_S34,0BEBFBC70H  ; 40
                        MD5HH       reg_a, reg_b, reg_c, reg_d, 13, MD5_S31, 289B7EC6H  ; 41
                        MD5HH       reg_d, reg_a, reg_b, reg_c,  0, MD5_S32,0EAA127FAH  ; 42
                        MD5HH       reg_c, reg_d, reg_a, reg_b,  3, MD5_S33,0D4EF3085H  ; 43
                        MD5HH       reg_b, reg_c, reg_d, reg_a,  6, MD5_S34,  4881D05H  ; 44
                        MD5HH       reg_a, reg_b, reg_c, reg_d,  9, MD5_S31,0D9D4D039H  ; 45
                        MD5HH       reg_d, reg_a, reg_b, reg_c, 12, MD5_S32,0E6DB99E5H  ; 46
                        MD5HH       reg_c, reg_d, reg_a, reg_b, 15, MD5_S33, 1FA27CF8H  ; 47
                        MD5HH       reg_b, reg_c, reg_d, reg_a,  2, MD5_S34,0C4AC5665H  ; 48

; round 4

                        MD5II       reg_a, reg_b, reg_c, reg_d,  0, MD5_S41,0F4292244H  ; 49
                        MD5II       reg_d, reg_a, reg_b, reg_c,  7, MD5_S42, 432AFF97H  ; 50
                        MD5II       reg_c, reg_d, reg_a, reg_b, 14, MD5_S43,0AB9423A7H  ; 51
                        MD5II       reg_b, reg_c, reg_d, reg_a,  5, MD5_S44,0FC93A039H  ; 52
                        MD5II       reg_a, reg_b, reg_c, reg_d, 12, MD5_S41, 655B59C3H  ; 53
                        MD5II       reg_d, reg_a, reg_b, reg_c,  3, MD5_S42, 8F0CCC92H  ; 54
                        MD5II       reg_c, reg_d, reg_a, reg_b, 10, MD5_S43,0FFEFF47DH  ; 55
                        MD5II       reg_b, reg_c, reg_d, reg_a,  1, MD5_S44, 85845DD1H  ; 56
                        MD5II       reg_a, reg_b, reg_c, reg_d,  8, MD5_S41, 6FA87E4FH  ; 57
                        MD5II       reg_d, reg_a, reg_b, reg_c, 15, MD5_S42,0FE2CE6E0H  ; 58
                        MD5II       reg_c, reg_d, reg_a, reg_b,  6, MD5_S43,0A3014314H  ; 59
                        MD5II       reg_b, reg_c, reg_d, reg_a, 13, MD5_S44, 4E0811A1H  ; 60
                        MD5II       reg_a, reg_b, reg_c, reg_d,  4, MD5_S41,0F7537E82H  ; 61
                        MD5II       reg_d, reg_a, reg_b, reg_c, 11, MD5_S42,0BD3AF235H  ; 62
                        MD5II       reg_c, reg_d, reg_a, reg_b,  2, MD5_S43, 2AD7D2BBH  ; 63
                        MD5II       reg_b, reg_c, reg_d, reg_a,  9, MD5_S44,0EB86D391H  ; 64

                        mov         reg_temp1, __this
                        add         [reg_temp1+m_nState0], reg_a
                        add         [reg_temp1+m_nState1], reg_b
                        add         [reg_temp1+m_nState2], reg_c
                        add         [reg_temp1+m_nState3], reg_d

                        ret

MD5_Transform_p5        ENDP

MD5_Add_p5              PROC        PUBLIC, _this:DWORD, _Data:DWORD, _nLength:DWORD

                        pusha
__this                  textequ     <[esp+36]>                              ; different offset due to pusha
__Data                  textequ     <[esp+40]>
__nLength               textequ     <[esp+44]>


                        mov         ecx, __nLength
                        and         ecx, ecx
                        jz          get_out
                        xor         edx, edx
                        mov         ebp, __Data
                        mov         edi, __this
                        mov         ebx, [edi+m_nCount0]
                        mov         eax, ebx
                        add         ebx, ecx
                        mov         [edi+m_nCount0], ebx
                        adc         [edi+m_nCount1], edx

                        and         eax, 63
                        jnz         partial_buffer
full_blocks:            mov         ecx, __nLength
                        and         ecx, ecx
                        jz          get_out
                        sub         ecx, 64
                        jb          end_of_stream
                        mov         __nLength, ecx
                        call        MD5_Transform_p5
                        add         ebp, 64
                        jmp         full_blocks

end_of_stream:          mov         edi, __this
                        mov         esi, ebp
                        lea         edi, [edi+m_pBuffer]
                        add         ecx, 64
                        rep movsb
                        jmp         get_out

partial_buffer:         add         ecx, eax                                ; eax = offset in buffer, ecx = _nLength
                        cmp         ecx, 64
                        jb          short_stream                            ; we can't fill the buffer
                        mov         ecx, -64
                        add         ecx, eax
                        add         __nLength, ecx                          ; _nlength += (offset-64)
@@:                     mov         bl, [ebp]
                        inc         ebp
                        mov         byte ptr [edi+m_pBuffer+64+ecx], bl
                        inc         ecx
                        jnz         @B                                      ; offset = 64
                        mov         __Data, ebp
                        lea         ebp, [edi+m_pBuffer]
                        call        MD5_Transform_p5
                        mov         ebp, __Data
                        jmp         full_blocks

short_stream:           sub         ecx, eax                                ;  --> ecx=_nLength
                        mov         esi, ebp
                        lea         edi, [edi+m_pBuffer+eax]
                        rep movsb

get_out:                popa
                        ret

MD5_Add_p5              ENDP

                end
