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

#if defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)
void strncpy_s( UTF8* dest, S64 destSize, StringEntry src, S64 srcSize )
{
    // This isn't really a safe version, but I know the inputs to expect.
    S64 len = ( std::min )( srcSize, destSize );
    memcpy( dest, src, len );
    if( ( destSize - len ) > 0 )
    {
        ::memset( dest + len, 0, destSize - len );
    }
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
        if( strncmp( text + i, "/*[", 3 ) == 0 )
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
    
    static StringEntry shaderBlockNames[GPUSHADER_TYPE_COUNT] =
    {
        "Vertex", "Fragment", "Geometry"
    };
    
    GPUProgramDesc theProgram = {};
    const Block* parsedBlocks[GPUSHADER_TYPE_COUNT] = {};
    
for( const auto & shaderBlockName : shaderBlockNames )
    {
        Block* block = FindBlock( shaderBlockName, blocks, numBlocks );
        if( block )
            parsedBlocks[theProgram.numShaders++] = block;
    }
    
    theProgram.shaders = ojkAllocArray<GPUShaderDesc>( allocator, theProgram.numShaders );
    
    int shaderIndex = 0;
    for( int shaderType = 0;
            shaderType < theProgram.numShaders;
            ++shaderType )
    {
        const Block* block = parsedBlocks[shaderType];
        if( !block )
            continue;
            
        UTF8* source = ojkAllocString( allocator, block->blockTextLength );
        
        strncpy_s(
            source,
            block->blockTextLength + 1,
            block->blockText,
            block->blockTextLength );
            
        GPUShaderDesc& shaderDesc = theProgram.shaders[shaderIndex];
        shaderDesc.type = static_cast<GPUShaderType>( shaderType );
        shaderDesc.source = source;
        shaderDesc.firstLineNumber = block->blockTextFirstLine;
        ++shaderIndex;
    }
    
    return theProgram;
}
