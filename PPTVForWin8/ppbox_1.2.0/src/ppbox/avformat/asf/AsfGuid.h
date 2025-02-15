// AsfGuid.h

#ifndef _PPBOX_AVFORMAT_ASF_ASF_GUID_H_
#define _PPBOX_AVFORMAT_ASF_ASF_GUID_H_

#include <framework/string/Uuid.h>

namespace ppbox
{
    namespace avformat
    {


        framework::string::UUID const ASF_Reserved_1 = { 0xABD3D211, 0xA9BA, 0x11CF,{ 0x8E, 0xE6, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65 } };

        framework::string::UUID const ASF_No_Error_Correction        =  { 0x20FB5700, 0x5B55,0x11CF,{ 0xA8, 0xFD,0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B } };
        framework::string::UUID const ASF_Audio_Spread               =  { 0xBFC3CD50, 0x618F,0x11CF,{ 0x8B, 0xB2,0x00, 0xAA, 0x00, 0xB4, 0xE2, 0x20 } };
        framework::string::UUID const ASF_Audio_Media                =  { 0xF8699E40, 0x5B4D, 0x11CF,{ 0xA8, 0xFD, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B } };
        framework::string::UUID const ASF_Video_Media                =  { 0xBC19EFC0, 0x5B4D, 0x11CF,{ 0xA8, 0xFD, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B } };

        framework::string::UUID const ASF_HEADER_OBJECT              =  { 0x75B22630, 0x668E, 0x11CF, { 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C } } ;
        framework::string::UUID const ASF_DATA_OBJECT                =  { 0x75B22636, 0x668E, 0x11CF, { 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C } }; 
        framework::string::UUID const ASF_Simple_Index_Object        =  { 0x33000890, 0xE5B1, 0x11CF, { 0x89, 0xF4, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xCB } } ;
        framework::string::UUID const ASF_Index_Object               =  { 0xD6E229D3, 0x35DA, 0x11D1, { 0x90, 0x34, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xBE } } ;
        framework::string::UUID const ASF_Media_Object_Index_Object  =  { 0xFEB103F8, 0x12AD, 0x4C64, { 0x84, 0x0F, 0x2A, 0x1D, 0x2F, 0x7A, 0xD4, 0x8C } } ;
        framework::string::UUID const ASF_Timecode_Index_Object      =  { 0x3CB73FD0, 0x0C4A, 0x4803, { 0x95, 0x3D, 0xED, 0xF7, 0xB6, 0x22, 0x8F, 0x0C } } ;
        /************************************************************************/
        /* 
        Header Object GUIDs
        */
        /************************************************************************/
        framework::string::UUID const ASF_FILE_PROPERTIES_OBJECT        =    { 0x8CABDCA1, 0xA947, 0x11CF, { 0x8E, 0xE4, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65 } } ;
        framework::string::UUID const ASF_STREAM_PROPERTIES_OBJECT        =    { 0xB7DC0791, 0xA9B7, 0x11CF, { 0x8E, 0xE6, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65 } } ;
        framework::string::UUID const ASF_HEADER_EXTENSION_OBJECT        =    { 0x5FBF03B5, 0xA92E, 0x11CF, { 0x8E, 0xE3, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65 } } ;
        framework::string::UUID const ASF_CODEC_LIST_OBJECT                =    { 0x86D15240, 0x311D, 0x11D0, { 0xA3, 0xA4, 0x00, 0xA0, 0xC9, 0x03, 0x48, 0xF6 } } ;
        framework::string::UUID const ASF_SCRIPT_COMMAND_OBJECT            =    { 0x1EFB1A30, 0x0B62, 0x11D0, { 0xA3, 0x9B, 0x00, 0xA0, 0xC9, 0x03, 0x48, 0xF6 } } ;
        framework::string::UUID const ASF_MARKER_OBJECT                        =    { 0xF487CD01, 0xA951, 0x11CF, { 0x8E, 0xE6, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65 } } ;
        framework::string::UUID const ASF_BITRATE_MUTUAL_EXCLUSION_OBJECT    =    { 0xD6E229DC, 0x35DA, 0x11D1, { 0x90, 0x34, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xBE } } ;
        framework::string::UUID const ASF_ERROR_CORRECTION_OBJECT            =    { 0x75B22635, 0x668E, 0x11CF, { 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C } } ;
        framework::string::UUID const ASF_CONTENT_DESCRIPTION_OBJECT    =        { 0x75B22633, 0x668E, 0x11CF, { 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C } } ;
        framework::string::UUID const ASF_EXTENDED_CONTENT_DESCRIPTION_OBJECT    =  { 0xD2D0A440, 0xE307, 0x11D2, { 0x97, 0xF0, 0x00, 0xA0, 0xC9, 0x5E, 0xA8, 0x50 } } ;
        framework::string::UUID const ASF_CONTENT_BRANDING_OBJECT                =  { 0x2211B3FA, 0xBD23, 0x11D2, { 0xB4, 0xB7, 0x00, 0xA0, 0xC9, 0x55, 0xFC, 0x6E } } ;
        framework::string::UUID const ASF_STREAM_BITRATE_PROPERTIES_OBJECT    =  { 0x7BF875CE, 0x468D, 0x11D1, { 0x8D, 0x82, 0x00, 0x60, 0x97, 0xC9, 0xA2, 0xB2 } } ;
        framework::string::UUID const ASF_CONTENT_ENCRYPTION_OBJECT            =  { 0x2211B3FB, 0xBD23, 0x11D2, { 0xB4, 0xB7, 0x00, 0xA0, 0xC9, 0x55, 0xFC, 0x6E } } ;

    } // namespace avformat
} // namespace ppbox

#endif // _PPBOX_AVFORMAT_ASF_ASF_GUID_H_
