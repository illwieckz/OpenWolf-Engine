////////////////////////////////////////////////////////////////////////////////////////
// Copyright(C) 2013 - 2016, OpenJK contributors
// Copyright(C) 2011 - 2018 Dusan Jocic <dusanjocic@msn.com>
//
// This file is part of OpenWolf.
//
// OpenWolf is free software; you can redistribute it
// and / or modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the License,
// or (at your option) any later version.
//
// OpenWolf is distributed in the hope that it will be
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with OpenWolf; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110 - 1301  USA
//
// -------------------------------------------------------------------------------------
// File name:   r_allocator.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <OWLib/precompiled.h>

#if defined(GLSL_BUILDTOOL)
#include <iostream>
#endif

#if defined (__LINUX__)
S32 strcpy_s( UTF8* dest, U64 destsz, StringEntry src )
{
    if( strlen( src ) >= destsz )
    {
        return 1;
    }
    strcpy( dest, src );
    return 0;
}
#endif

namespace
{
Block* FindBlock( StringEntry name, Block* blocks, U64 numBlocks )
{
    for( U64 i = 0; i < numBlocks; ++i )
    {
        Block* block = blocks + i;
        
        if( Q_strncmp( block->blockHeaderTitle, name, block->blockHeaderTitleLength ) == 0 )
        {
            return block;
        }
    }
    
    return nullptr;
}
}

GPUProgramDesc ParseProgramSource( Allocator& allocator, StringEntry text )
{
    S32 numBlocks = 0;
    Block blocks[MAX_BLOCKS];
    Block* prevBlock = nullptr;
    
    S32 i = 0;
    S32 line = 1;
    while( text[i] )
    {
        S32 markerStart = i;
        S32 markerEnd = -1;
        
        if( Q_strncmp( text + i, "/*[", 3 ) == 0 )
        {
            S32 startHeaderTitle = i + 3;
            S32 endHeaderTitle = -1;
            S32 endHeaderText = -1;
            S32 j = startHeaderTitle;
            while( text[j] )
            {
                if( text[j] == ']' )
                {
                    endHeaderTitle = j;
                }
                else if( strncmp( text + j, "*/", 2 ) == 0 )
                {
                    endHeaderText = j;
                    line++;
                    break;
                }
                else if( strncmp( text + j, "*/\n", 3 ) == 0 )
                {
                    endHeaderText = j;
                    line++;
                    break;
                }
                else if( text[j] == '\n' )
                {
                    line++;
                }
                
                ++j;
            }
            
            if( endHeaderTitle == -1 || endHeaderText == -1 )
            {
#if defined(GLSL_BUILDTOOL)
                std::cerr << "Unclosed block marker\n";
#else
                Com_Printf( S_COLOR_YELLOW "Unclosed block marker\n" );
#endif
                break;
            }
            
            Block* block = blocks + numBlocks++;
            block->blockHeaderTitle = text + startHeaderTitle;
            block->blockHeaderTitleLength = endHeaderTitle - startHeaderTitle;
            block->blockHeaderText = text + endHeaderTitle + 1;
            block->blockHeaderTextLength = endHeaderText - endHeaderTitle - 1;
            block->blockText = text + endHeaderText + 3;
            block->blockTextLength = 0;
            block->blockTextFirstLine = line;
            
            if( prevBlock )
            {
                prevBlock->blockTextLength = ( text + i ) - prevBlock->blockText;
            }
            prevBlock = block;
            
            i = endHeaderText + 3;
            continue;
        }
        else if( text[i] == '\n' )
        {
            line++;
        }
        
        ++i;
    }
    
    if( prevBlock )
    {
        prevBlock->blockTextLength = ( text + i ) - prevBlock->blockText;
    }
    
    GPUProgramDesc theProgram = {};
    theProgram.numShaders = 2;
    
    Block* vertexBlock = FindBlock( "Vertex", blocks, numBlocks );
    Block* fragmentBlock = FindBlock( "Fragment", blocks, numBlocks );
    
    theProgram.shaders = ojkAllocArray<GPUShaderDesc>( allocator, theProgram.numShaders );
    
    UTF8* vertexSource = ojkAllocString( allocator, vertexBlock->blockTextLength );
    UTF8* fragmentSource = ojkAllocString( allocator, fragmentBlock->blockTextLength );
    
#ifdef _WIN32
    strncpy_s( vertexSource, vertexBlock->blockTextLength + 1, vertexBlock->blockText, vertexBlock->blockTextLength );
    strncpy_s( fragmentSource, fragmentBlock->blockTextLength + 1, fragmentBlock->blockText, fragmentBlock->blockTextLength );
#else
    strncpy( vertexSource, vertexBlock->blockText, vertexBlock->blockTextLength );
    strncpy( fragmentSource, fragmentBlock->blockText, fragmentBlock->blockTextLength );
#endif
    
    theProgram.shaders[0].type = GPUSHADER_VERTEX;
    theProgram.shaders[0].source = vertexSource;
    theProgram.shaders[0].firstLine = vertexBlock->blockTextFirstLine;
    
    theProgram.shaders[1].type = GPUSHADER_FRAGMENT;
    theProgram.shaders[1].source = fragmentSource;
    theProgram.shaders[1].firstLine = fragmentBlock->blockTextFirstLine;
    
    return theProgram;
}
