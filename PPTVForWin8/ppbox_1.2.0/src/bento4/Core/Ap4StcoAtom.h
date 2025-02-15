/*****************************************************************
|
|    AP4 - stco Atoms 
|
|    Copyright 2002-2008 Axiomatic Systems, LLC
|
|
|    This file is part of Bento4/AP4 (MP4 Atom Processing Library).
|
|    Unless you have obtained Bento4 under a difference license,
|    this version of Bento4 is Bento4|GPL.
|    Bento4|GPL is free software; you can redistribute it and/or modify
|    it under the terms of the GNU General Public License as published by
|    the Free Software Foundation; either version 2, or (at your option)
|    any later version.
|
|    Bento4|GPL is distributed in the hope that it will be useful,
|    but WITHOUT ANY WARRANTY; without even the implied warranty of
|    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
|    GNU General Public License for more details.
|
|    You should have received a copy of the GNU General Public License
|    along with Bento4|GPL; see the file COPYING.  If not, write to the
|    Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA
|    02111-1307, USA.
|
 ****************************************************************/

#ifndef _AP4_STCO_ATOM_H_
#define _AP4_STCO_ATOM_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Types.h"
#include "Ap4Atom.h"

/*----------------------------------------------------------------------
|   AP4_StcoAtom
+---------------------------------------------------------------------*/
class AP4_StcoAtom : public AP4_Atom
{
public:
    AP4_IMPLEMENT_DYNAMIC_CAST_D(AP4_StcoAtom, AP4_Atom)

    // class methods
    static AP4_StcoAtom* Create(AP4_Size size, AP4_ByteStream& stream);

    // methods
    AP4_StcoAtom(AP4_UI32* offsets, AP4_UI32 offset_count);
    ~AP4_StcoAtom();
    virtual AP4_Result InspectFields(AP4_AtomInspector& inspector);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);
    AP4_Cardinal GetChunkCount()   { return m_EntryCount;  }
    AP4_UI32*    GetChunkOffsets() { return m_Entries;     }
    AP4_Result   GetChunkOffset(AP4_Ordinal chunk, AP4_UI32& chunk_offset);
    AP4_Result   SetChunkOffset(AP4_Ordinal chunk, AP4_UI32  chunk_offset);
    AP4_Result   AdjustChunkOffsets(int delta);

    // ADD FOR PPBOX
    void ReSetEntryCount(AP4_UI32 entry_count);
    AP4_Result AdjustChunkOffsets(AP4_Cardinal form, int delta);

private:
    // methods
    AP4_StcoAtom(AP4_UI32        size, 
                 AP4_UI32        version,
                 AP4_UI32        flags,
                 AP4_ByteStream& stream);

    // members
    AP4_UI32* m_Entries;
    AP4_UI32  m_EntryCount;
};

#endif // _AP4_STCO_ATOM_H_
