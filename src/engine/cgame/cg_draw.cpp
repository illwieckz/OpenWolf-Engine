////////////////////////////////////////////////////////////////////////////////////////
// Copyright(C) 1999 - 2005 Id Software, Inc.
// Copyright(C) 2000 - 2006 Tim Angus
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
// File name:   cg_draw.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description: draw all of the graphical elements during
//              active (after loading) gameplay
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <OWLIb/precompiled.h>

menuDef_t* menuScoreboard = NULL;

S32 drawTeamOverlayModificationCount = -1;
S32 sortedTeamPlayers[ TEAM_MAXOVERLAY ];
S32 numSortedTeamPlayers;

void idCGameLocal::AlignText( rectDef_t* rect, StringEntry text, F32 scale, F32 w, F32 h, S32 align, S32 valign, F32* x, F32* y )
{
    F32 tx, ty;
    
    if( scale > 0.0f )
    {
        w = UI_Text_Width( text, scale, 0 );
        h = UI_Text_Height( text, scale, 0 );
    }
    
    switch( align )
    {
        default:
        case ALIGN_LEFT:
            tx = 0.0f;
            break;
            
        case ALIGN_RIGHT:
            tx = rect->w - w;
            break;
            
        case ALIGN_CENTER:
            tx = ( rect->w - w ) / 2.0f;
            break;
    }
    
    switch( valign )
    {
        default:
        case VALIGN_BOTTOM:
            ty = rect->h;
            break;
            
        case VALIGN_TOP:
            ty = h;
            break;
            
        case VALIGN_CENTER:
            ty = h + ( ( rect->h - h ) / 2.0f );
            break;
    }
    
    if( x )
        *x = rect->x + tx;
        
    if( y )
        *y = rect->y + ty;
}

/*
==============
DrawFieldPadded

Draws large numbers for status bar
==============
*/
void idCGameLocal::DrawFieldPadded( S32 x, S32 y, S32 width, S32 cw, S32 ch, S32 value )
{
    UTF8  num[ 16 ], *ptr;
    S32   l, orgL;
    S32   frame;
    S32   charWidth, charHeight;
    
    if( !( charWidth = cw ) )
        charWidth = CHAR_WIDTH;
        
    if( !( charHeight = ch ) )
        charHeight = CHAR_HEIGHT;
        
    if( width < 1 )
        return;
        
    // draw number string
    if( width > 4 )
        width = 4;
        
    switch( width )
    {
        case 1:
            value = value > 9 ? 9 : value;
            value = value < 0 ? 0 : value;
            break;
        case 2:
            value = value > 99 ? 99 : value;
            value = value < -9 ? -9 : value;
            break;
        case 3:
            value = value > 999 ? 999 : value;
            value = value < -99 ? -99 : value;
            break;
        case 4:
            value = value > 9999 ? 9999 : value;
            value = value < -999 ? -999 : value;
            break;
    }
    
    Com_sprintf( num, sizeof( num ), "%d", value );
    l = strlen( num );
    
    if( l > width )
        l = width;
        
    orgL = l;
    
    x += ( 2.0f * cgDC.aspectScale );
    
    ptr = num;
    while( *ptr && l )
    {
        if( width > orgL )
        {
            DrawPic( x, y, charWidth, charHeight, cgs.media.numberShaders[ 0 ] );
            width--;
            x += charWidth;
            continue;
        }
        
        if( *ptr == '-' )
            frame = STAT_MINUS;
        else
            frame = *ptr - '0';
            
        DrawPic( x, y, charWidth, charHeight, cgs.media.numberShaders[ frame ] );
        x += charWidth;
        ptr++;
        l--;
    }
}

/*
==============
DrawField

Draws large numbers for status bar
==============
*/
void idCGameLocal::DrawField( F32 x, F32 y, S32 width, F32 cw, F32 ch, S32 value )
{
    UTF8  num[ 16 ], *ptr;
    S32   l;
    S32   frame;
    F32 charWidth, charHeight;
    
    if( !( charWidth = cw ) )
        charWidth = CHAR_WIDTH;
        
    if( !( charHeight = ch ) )
        charHeight = CHAR_HEIGHT;
        
    if( width < 1 )
        return;
        
    // draw number string
    if( width > 4 )
        width = 4;
        
    switch( width )
    {
        case 1:
            value = value > 9 ? 9 : value;
            value = value < 0 ? 0 : value;
            break;
        case 2:
            value = value > 99 ? 99 : value;
            value = value < -9 ? -9 : value;
            break;
        case 3:
            value = value > 999 ? 999 : value;
            value = value < -99 ? -99 : value;
            break;
        case 4:
            value = value > 9999 ? 9999 : value;
            value = value < -999 ? -999 : value;
            break;
    }
    
    Com_sprintf( num, sizeof( num ), "%d", value );
    l = strlen( num );
    
    if( l > width )
        l = width;
        
    x += ( 2.0f * cgDC.aspectScale ) + charWidth * ( width - l );
    
    ptr = num;
    while( *ptr && l )
    {
        if( *ptr == '-' )
            frame = STAT_MINUS;
        else
            frame = *ptr - '0';
            
        DrawPic( x, y, charWidth, charHeight, cgs.media.numberShaders[ frame ] );
        x += charWidth;
        ptr++;
        l--;
    }
}

void idCGameLocal::DrawProgressBar( rectDef_t* rect, vec4_t color, F32 scale, S32 align, S32 textalign, S32 textStyle, S32 special, F32 progress )
{
    F32   rimWidth = rect->h / 20.0f;
    F32   doneWidth, leftWidth;
    F32   tx, ty;
    UTF8    textBuffer[ 8 ];
    
    if( rimWidth < 0.6f )
        rimWidth = 0.6f;
        
    if( special >= 0.0f )
        rimWidth = special;
        
    if( progress < 0.0f )
        progress = 0.0f;
    else if( progress > 1.0f )
        progress = 1.0f;
        
    doneWidth = ( rect->w - 2 * rimWidth ) * progress;
    leftWidth = ( rect->w - 2 * rimWidth ) - doneWidth;
    
    trap_R_SetColor( color );
    
    //draw rim and bar
    if( align == ALIGN_RIGHT )
    {
        DrawPic( rect->x, rect->y, rimWidth, rect->h, cgs.media.whiteShader );
        DrawPic( rect->x + rimWidth, rect->y,
                 leftWidth, rimWidth, cgs.media.whiteShader );
        DrawPic( rect->x + rimWidth, rect->y + rect->h - rimWidth,
                 leftWidth, rimWidth, cgs.media.whiteShader );
        DrawPic( rect->x + rimWidth + leftWidth, rect->y,
                 rimWidth + doneWidth, rect->h, cgs.media.whiteShader );
    }
    else
    {
        DrawPic( rect->x, rect->y, rimWidth + doneWidth, rect->h, cgs.media.whiteShader );
        DrawPic( rimWidth + rect->x + doneWidth, rect->y,
                 leftWidth, rimWidth, cgs.media.whiteShader );
        DrawPic( rimWidth + rect->x + doneWidth, rect->y + rect->h - rimWidth,
                 leftWidth, rimWidth, cgs.media.whiteShader );
        DrawPic( rect->x + rect->w - rimWidth, rect->y, rimWidth, rect->h, cgs.media.whiteShader );
    }
    
    trap_R_SetColor( NULL );
    
    //draw text
    if( scale > 0.0 )
    {
        Com_sprintf( textBuffer, sizeof( textBuffer ), "%d%%", ( S32 )( progress * 100 ) );
        AlignText( rect, textBuffer, scale, 0.0f, 0.0f, textalign, VALIGN_CENTER, &tx, &ty );
        
        UI_Text_Paint( tx, ty, scale, color, textBuffer, 0, 0, textStyle );
    }
}

#define NO_CREDITS_TIME 2000

void idCGameLocal::DrawPlayerCreditsValue( rectDef_t* rect, vec4_t color, bool padding )
{
    S32           value;
    playerState_t* ps;
    centity_t*     cent;
    
    cent = &cg_entities[ cg.snap->ps.clientNum ];
    ps = &cg.snap->ps;
    
    //if the build timer pie is showing don't show this
    if( ( cent->currentState.weapon == WP_ABUILD ||
            cent->currentState.weapon == WP_ABUILD2 ) && ps->stats[ STAT_MISC ] )
        return;
        
    value = ps->persistant[ PERS_CREDIT ];
    if( value > -1 )
    {
        if( cg.predictedPlayerState.stats[ STAT_TEAM ] == TEAM_ALIENS )
        {
            if( !bggame->AlienCanEvolve( ( class_t )cg.predictedPlayerState.stats[ STAT_CLASS ], value, cgs.alienStage ) &&
                    cg.time - cg.lastEvolveAttempt <= NO_CREDITS_TIME && ( ( cg.time - cg.lastEvolveAttempt ) / 300 ) & 1 )
                color[ 3 ] = 0.0f;
            value /= ALIEN_CREDITS_PER_FRAG;
        }
        
        trap_R_SetColor( color );
        
        if( padding )
            DrawFieldPadded( rect->x, rect->y, 4, rect->w / 4, rect->h, value );
        else
            DrawField( rect->x, rect->y, 1, rect->w, rect->h, value );
            
        trap_R_SetColor( NULL );
    }
}

void idCGameLocal::DrawAttackFeedback( rectDef_t* rect )
{
    static bool flipAttackFeedback = false;
    S32 frame = cg.feedbackAnimation;
    qhandle_t shader;
    vec4_t hit_color = { 1, 0, 0, 0.5 };
    vec4_t miss_color = { 0.3, 0.3, 0.3, 0.5 };
    vec4_t teamhit_color = { 0.39, 0.80, 0.00, 0.5 };
    
    
    if( frame == 1 )
    {
        flipAttackFeedback = !flipAttackFeedback;
    }
    
    //when a new feedback animation event is received, the fame number is set to 1 - so
    //if it is zero, we don't need to draw anything
    if( frame == 0 || !cg_drawAlienFeedback.integer )   //drop out if we aren't currently needing to draw any feedback
    {
        //Com_Printf(".");
        return;
    }
    else
    {
        switch( cg.feedbackAnimationType )
        {
            case AFEEDBACK_HIT:
            case AFEEDBACK_MISS:
            case AFEEDBACK_TEAMHIT:
                if( flipAttackFeedback )
                    shader = cgs.media.alienAttackFeedbackShadersFlipped[ frame - 1 ];
                else
                    shader = cgs.media.alienAttackFeedbackShaders[ frame - 1 ];
                break;
            case AFEEDBACK_RANGED_HIT:
            case AFEEDBACK_RANGED_MISS:
            case AFEEDBACK_RANGED_TEAMHIT:
                if( flipAttackFeedback )
                    shader = cgs.media.alienAttackFeedbackShadersFlipped[ frame - 1 ];
                else
                    shader = cgs.media.alienAttackFeedbackShaders[ frame - 1 ];
                break;
            default:
                shader = cgs.media.alienAttackFeedbackShaders[ frame - 1 ];
                break;
                
        }
        cg.feedbackAnimation++;
        if( cg.feedbackAnimation > 10 )
            cg.feedbackAnimation = 0;
            
        switch( cg.feedbackAnimationType )
        {
            case AFEEDBACK_HIT:
            case AFEEDBACK_RANGED_HIT:
                trap_R_SetColor( hit_color );
                break;
            case AFEEDBACK_MISS:
            case AFEEDBACK_RANGED_MISS:
                trap_R_SetColor( miss_color );
                break;
            case AFEEDBACK_TEAMHIT:
            case AFEEDBACK_RANGED_TEAMHIT:
                trap_R_SetColor( teamhit_color );
                break;
        }
        DrawPic( rect->x, rect->y, rect->w, rect->h, shader );
        trap_R_SetColor( NULL );
    }
}

/*
==============
DrawPlayerStamina
==============
*/
void idCGameLocal::DrawPlayerStamina( S32 ownerDraw, rectDef_t* rect, vec4_t backColor, vec4_t foreColor, qhandle_t shader )
{
    playerState_t* ps = &cg.snap->ps;
    F32         stamina = ps->stats[ STAT_STAMINA ];
    F32         maxStaminaBy3 = ( F32 )MAX_STAMINA / 3.0f;
    F32         progress;
    vec4_t        color;
    
    switch( ownerDraw )
    {
        case CG_PLAYER_STAMINA_1:
            progress = ( stamina - 2 * ( S32 )maxStaminaBy3 ) / maxStaminaBy3;
            break;
        case CG_PLAYER_STAMINA_2:
            progress = ( stamina - ( S32 )maxStaminaBy3 ) / maxStaminaBy3;
            break;
        case CG_PLAYER_STAMINA_3:
            progress = stamina / maxStaminaBy3;
            break;
        case CG_PLAYER_STAMINA_4:
            progress = ( stamina + MAX_STAMINA ) / MAX_STAMINA;
            break;
        default:
            return;
    }
    
    if( progress > 1.0f )
        progress  = 1.0f;
    else if( progress < 0.0f )
        progress = 0.0f;
        
    Vector4Lerp( progress, backColor, foreColor, color );
    
    trap_R_SetColor( color );
    DrawPic( rect->x, rect->y, rect->w, rect->h, shader );
    trap_R_SetColor( NULL );
}

/*
==============
DrawPlayerStaminaBolt
==============
*/
void idCGameLocal::DrawPlayerStaminaBolt( rectDef_t* rect, vec4_t backColor, vec4_t foreColor, qhandle_t shader )
{
    F32 stamina = cg.snap->ps.stats[ STAT_STAMINA ];
    vec4_t color;
    
    if( stamina < 0 )
        Vector4Copy( backColor, color );
    else if( cg.predictedPlayerState.stats[ STAT_STATE ] & SS_SPEEDBOOST )
        Vector4Lerp( ( sin( cg.time / 150.f ) + 1 ) / 2,
                     backColor, foreColor, color );
    else
        Vector4Copy( foreColor, color );
        
    trap_R_SetColor( color );
    DrawPic( rect->x, rect->y, rect->w, rect->h, shader );
    trap_R_SetColor( NULL );
}

/*
==============
DrawPlayerClipsRing
==============
*/
void idCGameLocal::DrawPlayerClipsRing( rectDef_t* rect, vec4_t backColor, vec4_t foreColor, qhandle_t shader )
{
    playerState_t* ps = &cg.snap->ps;
    centity_t*     cent;
    F32         buildTime = ps->stats[ STAT_MISC ];
    F32         progress;
    F32         maxDelay;
    weapon_t      weapon;
    vec4_t        color;
    
    cent = &cg_entities[ cg.snap->ps.clientNum ];
    weapon = bggame->GetPlayerWeapon( ps );
    
    switch( weapon )
    {
        case WP_ABUILD:
        case WP_ABUILD2:
        case WP_HBUILD:
            if( buildTime > MAXIMUM_BUILD_TIME )
                buildTime = MAXIMUM_BUILD_TIME;
            progress = ( MAXIMUM_BUILD_TIME - buildTime ) / MAXIMUM_BUILD_TIME;
            
            Vector4Lerp( progress, backColor, foreColor, color );
            break;
            
        default:
            if( ps->weaponstate == WEAPON_RELOADING )
            {
                maxDelay = ( F32 )bggame->Weapon( ( weapon_t )cent->currentState.weapon )->reloadTime;
                progress = ( maxDelay - ( F32 )ps->weaponTime ) / maxDelay;
                
                Vector4Lerp( progress, backColor, foreColor, color );
            }
            else
                ::memcpy( color, foreColor, sizeof( color ) );
            break;
    }
    
    trap_R_SetColor( color );
    DrawPic( rect->x, rect->y, rect->w, rect->h, shader );
    trap_R_SetColor( NULL );
}

/*
==============
DrawPlayerBuildTimerRing
==============
*/
void idCGameLocal::DrawPlayerBuildTimerRing( rectDef_t* rect, vec4_t backColor, vec4_t foreColor, qhandle_t shader )
{
    playerState_t* ps = &cg.snap->ps;
    centity_t*     cent;
    F32         buildTime = ps->stats[ STAT_MISC ];
    F32         progress;
    vec4_t        color;
    
    cent = &cg_entities[ cg.snap->ps.clientNum ];
    
    if( buildTime > MAXIMUM_BUILD_TIME )
        buildTime = MAXIMUM_BUILD_TIME;
    progress = ( MAXIMUM_BUILD_TIME - buildTime ) / MAXIMUM_BUILD_TIME;
    
    Vector4Lerp( progress, backColor, foreColor, color );
    
    trap_R_SetColor( color );
    DrawPic( rect->x, rect->y, rect->w, rect->h, shader );
    trap_R_SetColor( NULL );
}

/*
==============
DrawPlayerBoosted
==============
*/
void idCGameLocal::DrawPlayerBoosted( rectDef_t* rect, vec4_t backColor, vec4_t foreColor, qhandle_t shader )
{
    if( cg.snap->ps.stats[ STAT_STATE ] & SS_BOOSTED )
        trap_R_SetColor( foreColor );
    else
        trap_R_SetColor( backColor );
        
    DrawPic( rect->x, rect->y, rect->w, rect->h, shader );
    trap_R_SetColor( NULL );
}

/*
==============
DrawPlayerBoosterBolt
==============
*/
void idCGameLocal::DrawPlayerBoosterBolt( rectDef_t* rect, vec4_t backColor, vec4_t foreColor, qhandle_t shader )
{
    vec4_t color;
    
    // Flash bolts when the boost is almost out
    if( ( cg.snap->ps.stats[ STAT_STATE ] & SS_BOOSTED ) &&
            ( cg.snap->ps.stats[ STAT_STATE ] & SS_BOOSTEDWARNING ) )
        Vector4Lerp( ( sin( cg.time / 100.f ) + 1 ) / 2,
                     backColor, foreColor, color );
    else
        Vector4Copy( foreColor, color );
        
    trap_R_SetColor( color );
    DrawPic( rect->x, rect->y, rect->w, rect->h, shader );
    trap_R_SetColor( NULL );
}

/*
==============
DrawPlayerPoisonBarbs
==============
*/
void idCGameLocal::DrawPlayerPoisonBarbs( rectDef_t* rect, vec4_t color, qhandle_t shader )
{
    bool vertical;
    F32    x = rect->x, y = rect->y;
    F32    width = rect->w, height = rect->h;
    F32    diff;
    S32      iconsize, numBarbs, maxBarbs;
    
    maxBarbs = bggame->Weapon( ( weapon_t )cg.snap->ps.weapon )->maxAmmo;
    numBarbs = cg.snap->ps.ammo;
    if( maxBarbs <= 0 || numBarbs <= 0 )
        return;
        
    // adjust these first to ensure the aspect ratio of the barb image is
    // preserved
    AdjustFrom640( &x, &y, &width, &height );
    
    if( height > width )
    {
        vertical = true;
        iconsize = width;
        if( maxBarbs != 1 ) // avoid division by zero
            diff = ( height - iconsize ) / ( F32 )( maxBarbs - 1 );
        else
            diff = 0; // doesn't matter, won't be used
    }
    else
    {
        vertical = false;
        iconsize = height;
        if( maxBarbs != 1 )
            diff = ( width - iconsize ) / ( F32 )( maxBarbs - 1 );
        else
            diff = 0;
    }
    
    trap_R_SetColor( color );
    
    for( ; numBarbs > 0; numBarbs-- )
    {
        trap_R_DrawStretchPic( x, y, iconsize, iconsize, 0, 0, 1, 1, shader );
        if( vertical )
            y += diff;
        else
            x += diff;
    }
    
    trap_R_SetColor( NULL );
}

/*
==============
DrawPlayerWallclimbing
==============
*/
void idCGameLocal::DrawPlayerWallclimbing( rectDef_t* rect, vec4_t backColor, vec4_t foreColor, qhandle_t shader )
{
    if( cg.snap->ps.stats[ STAT_STATE ] & SS_WALLCLIMBING )
        trap_R_SetColor( foreColor );
    else
        trap_R_SetColor( backColor );
        
    DrawPic( rect->x, rect->y, rect->w, rect->h, shader );
    trap_R_SetColor( NULL );
}

void idCGameLocal::DrawPlayerAmmoValue( rectDef_t* rect, vec4_t color )
{
    S32 value;
    
    switch( bggame->PrimaryWeapon( cg.snap->ps.stats ) )
    {
        case WP_NONE:
        case WP_BLASTER:
            return;
            
        case WP_ABUILD:
        case WP_ABUILD2:
            // BP remaining
            value = cgs.alienBuildPoints;
            break;
            
        case WP_HBUILD:
            // BP remaining
            value = cgs.humanBuildPoints;
            break;
            
        default:
            value = cg.snap->ps.ammo;
            break;
    }
    
    if( value > 999 )
        value = 999;
        
    if( value > -1 )
    {
        trap_R_SetColor( color );
        DrawField( rect->x, rect->y, 4, rect->w / 4, rect->h, value );
        trap_R_SetColor( NULL );
    }
}

void idCGameLocal::DrawStack( rectDef_t* rect, vec4_t color, F32 fill, S32 align, S32 valign, F32 val, S32 max )
{
    S32 i;
    F32 each;
    S32 ival;
    F32 frac;
    F32 nudge = 0;
    F32 fmax = max; // otherwise we'd be (F32) casting everywhere
    
    if( val <= 0 || max <= 0 )
        return;
        
    ival = ( S32 )val;
    frac = val - ival;
    
    trap_R_SetColor( color );
    
    if( rect->h >= rect->w ) // vertical stack
    {
        each = fill * rect->h / fmax;
        if( each * cgs.screenYScale < 4.f ) // FIXME: magic number
        {
            F32 offy, h = rect->h * val / fmax;
            switch( valign )
            {
                case VALIGN_TOP:
                    offy = 0;
                    break;
                case VALIGN_CENTER:
                    offy = ( rect->h - h ) / 2;
                    break;
                case VALIGN_BOTTOM:
                default:
                    offy = rect->h - h;
                    break;
            }
            DrawPic( rect->x, rect->y + offy, rect->w, h, cgs.media.whiteShader );
            trap_R_SetColor( NULL );
            return;
        }
        
        if( fmax > 1 )
            nudge = ( 1 - fill ) / ( fmax - 1 );
        else
            return;
        for( i = 0; i < ival; i++ )
        {
            F32 start;
            switch( valign )
            {
                case VALIGN_TOP:
                    start = ( i * ( 1 + nudge ) + frac ) / fmax;
                    break;
                case VALIGN_CENTER:
                    // TODO (fallthrough for now)
                default:
                case VALIGN_BOTTOM:
                    start = 1 - ( val - i - ( i + fmax - val ) * nudge ) / fmax;
                    break;
            }
            DrawPic( rect->x, rect->y + rect->h * start, rect->w, each,
                     cgs.media.whiteShader );
        }
        color[ 3 ] *= frac;
        trap_R_SetColor( color );
        switch( valign )
        {
            case VALIGN_TOP:
                DrawPic( rect->x, rect->y - rect->h * ( 1 - frac ) / fmax,
                         rect->w, each, cgs.media.whiteShader );
                break;
            case VALIGN_CENTER:
                // fallthrough
            default:
            case VALIGN_BOTTOM:
                DrawPic( rect->x, rect->y + rect->h *
                         ( 1 + ( ( 1 - fill ) / fmax ) - frac / fmax ),
                         rect->w, each, cgs.media.whiteShader );
        }
    }
    else // horizontal stack
    {
        each = fill * rect->w / fmax;
        if( each < 4.f )
        {
            F32 offx, w = rect->w * val / fmax;
            switch( align )
            {
                case ALIGN_LEFT:
                default:
                    offx = 0;
                    break;
                case ALIGN_CENTER:
                    offx = ( rect->w - w ) / 2;
                    break;
                case ALIGN_RIGHT:
                    offx = rect->w - w;
                    break;
            }
            DrawPic( rect->x + offx, rect->y, w, rect->h, cgs.media.whiteShader );
            trap_R_SetColor( NULL );
            return;
        }
        
        if( fmax > 1 )
            nudge = ( 1 - fill ) / ( fmax - 1 );
        for( i = 0; i < ival; i++ )
        {
            F32 start;
            switch( align )
            {
                case ALIGN_LEFT:
                    start = ( i * ( 1 + nudge ) + frac ) / fmax;
                    break;
                case ALIGN_CENTER:
                    // TODO (fallthrough for now)
                default:
                case ALIGN_RIGHT:
                    start = 1 - ( val - i - ( i + fmax - val ) * nudge ) / fmax;
                    break;
            }
            DrawPic( rect->x + rect->w * start, rect->y, each, rect->h,
                     cgs.media.whiteShader );
        }
        color[ 3 ] *= frac;
        trap_R_SetColor( color );
        switch( align )
        {
            case ALIGN_LEFT:
                DrawPic( rect->x - ( 1 - frac ) * rect->w / fmax, rect->y,
                         each, rect->h, cgs.media.whiteShader );
                break;
            case ALIGN_CENTER:
                // fallthrough
            default:
            case ALIGN_RIGHT:
                DrawPic( rect->x + rect->w *
                         ( 1 + ( ( 1 - fill ) / fmax ) - frac / fmax ),
                         rect->y, each, rect->h, cgs.media.whiteShader );
        }
    }
    
    trap_R_SetColor( NULL );
}

void idCGameLocal::DrawPlayerAmmoStack( rectDef_t* rect, vec4_t backColor, vec4_t foreColor, S32 textalign, S32 textvalign )
{
    F32 val;
    S32 maxVal;
    static S32 lastws, maxwt, lastammo, ammodiff;
    playerState_t* ps = &cg.snap->ps;
    weapon_t primary = bggame->PrimaryWeapon( ps->stats );
    
    if( !cg_drawAmmoStack.integer )
        return;
        
    switch( primary )
    {
        case WP_NONE:
        case WP_BLASTER:
            return;
            
        case WP_ABUILD:
        case WP_ABUILD2:
        case WP_HBUILD:
            // FIXME: send max BP values over the network
            return;
            
        default:
            val = ps->ammo;
            maxVal = bggame->Weapon( primary )->maxAmmo;
            if( bggame->Weapon( primary )->usesEnergy &&
                    bggame->InventoryContainsUpgrade( UP_BATTPACK, ps->stats ) )
                maxVal *= BATTPACK_MODIFIER;
            break;
    }
    
    if( ps->weaponstate != lastws || ps->weaponTime > maxwt )
    {
        maxwt = ps->weaponTime;
        lastws = ps->weaponstate;
    }
    else if( ps->weaponTime == 0 )
    {
        maxwt = 0;
    }
    
    if( lastammo != ps->ammo )
    {
        ammodiff = lastammo - ps->ammo;
        lastammo = ps->ammo;
    }
    
    // smoothing effects if we're holding primary weapon
    if( primary == bggame->GetPlayerWeapon( ps ) )
    {
        switch( ps->weaponstate )
        {
            case WEAPON_FIRING:
                if( maxwt > 0 )
                {
                    F32 f = ps->weaponTime / ( F32 )maxwt;
                    val += ammodiff * f * f;
                }
                break;
                
            case WEAPON_RELOADING:
                val = ps->weaponTime / ( F32 )maxwt;
                val *= val;
                val = ( 1 - val ) * ( maxVal - ps->ammo ) + ps->ammo;
                break;
                
            default:
                maxwt = 0;
                break;
        }
    }
    
    trap_R_SetColor( backColor );
    DrawPic( rect->x, rect->y, rect->w, rect->h, cgs.media.whiteShader );
    trap_R_SetColor( NULL );
    DrawStack( rect, foreColor, 0.8, textalign, textvalign,
               val, maxVal );
}

void idCGameLocal::DrawPlayerClipsStack( rectDef_t* rect, vec4_t backColor, vec4_t foreColor, S32 textalign, S32 textvalign )
{
    F32         val;
    S32           maxVal;
    static S32    lastws, maxwt;
    playerState_t* ps = &cg.snap->ps;
    weapon_t      primary = bggame->PrimaryWeapon( ps->stats );
    
    if( !cg_drawAmmoStack.integer )
        return;
        
    switch( primary )
    {
        case WP_NONE:
        case WP_BLASTER:
        case WP_ABUILD:
        case WP_ABUILD2:
        case WP_HBUILD:
            return;
            
        default:
            val = ps->clips;
            maxVal = bggame->Weapon( primary )->maxClips;
            break;
    }
    
    if( ps->weaponstate != lastws || ps->weaponTime > maxwt )
    {
        maxwt = ps->weaponTime;
        lastws = ps->weaponstate;
    }
    else if( ps->weaponTime == 0 )
        maxwt = 0;
        
    switch( ps->weaponstate )
    {
        case WEAPON_RELOADING:
            if( maxwt > 0 )
            {
                F32 f = ps->weaponTime / ( F32 )maxwt;
                val += f * f - 1;
            }
            break;
            
        default:
            maxwt = 0;
            break;
    }
    
    trap_R_SetColor( backColor );
    DrawPic( rect->x, rect->y, rect->w, rect->h, cgs.media.whiteShader );
    trap_R_SetColor( NULL );
    DrawStack( rect, foreColor, 0.8, textalign, textvalign,
               val, maxVal );
}

/*
==============
DrawAlienSense
==============
*/
void idCGameLocal::DrawAlienSense( rectDef_t* rect )
{
    if( bggame->ClassHasAbility( ( class_t )cg.snap->ps.stats[ STAT_CLASS ], SCA_ALIENSENSE ) )
        AlienSense( rect );
}


/*
==============
DrawHumanScanner
==============
*/
void idCGameLocal::DrawHumanScanner( rectDef_t* rect, qhandle_t shader, vec4_t color )
{
    if( bggame->InventoryContainsUpgrade( UP_HELMET, cg.snap->ps.stats ) )
        Scanner( rect, shader, color );
}


/*
==============
DrawUsableBuildable
==============
*/
void idCGameLocal::DrawUsableBuildable( rectDef_t* rect, qhandle_t shader, vec4_t color )
{
    vec3_t        view, point;
    trace_t       trace;
    entityState_t* es;
    
    AngleVectors( cg.refdefViewAngles, view, NULL, NULL );
    VectorMA( cg.refdef.vieworg, 64, view, point );
    Trace( &trace, cg.refdef.vieworg, NULL, NULL, point, cg.predictedPlayerState.clientNum, MASK_SHOT );
    
    es = &cg_entities[ trace.entityNum ].currentState;
    
    if( es->eType == ET_BUILDABLE && bggame->Buildable( ( buildable_t )es->modelindex )->usable &&
            cg.predictedPlayerState.stats[ STAT_TEAM ] == bggame->Buildable( ( buildable_t )es->modelindex )->team )
    {
        //hack to prevent showing the usable buildable when you aren't carrying an energy weapon
        if( ( es->modelindex == BA_H_REACTOR || es->modelindex == BA_H_REPEATER ) &&
                ( !bggame->Weapon( ( weapon_t )cg.snap->ps.weapon )->usesEnergy ||
                  bggame->Weapon( ( weapon_t )cg.snap->ps.weapon )->infiniteAmmo ) )
            return;
            
        trap_R_SetColor( color );
        DrawPic( rect->x, rect->y, rect->w, rect->h, shader );
        trap_R_SetColor( NULL );
    }
}

#define BUILD_DELAY_TIME  2000

void idCGameLocal::DrawPlayerBuildTimer( rectDef_t* rect, vec4_t color )
{
    S32 index;
    playerState_t* ps;
    
    ps = &cg.snap->ps;
    
    if( ps->stats[ STAT_MISC ] <= 0 )
        return;
        
    switch( bggame->PrimaryWeapon( ps->stats ) )
    {
        case WP_ABUILD:
        case WP_ABUILD2:
        case WP_HBUILD:
            break;
            
        default:
            return;
    }
    
    index = 8 * ( ps->stats[ STAT_MISC ] - 1 ) / MAXIMUM_BUILD_TIME;
    if( index > 7 )
        index = 7;
    else if( index < 0 )
        index = 0;
        
    if( cg.time - cg.lastBuildAttempt <= BUILD_DELAY_TIME &&
            ( ( cg.time - cg.lastBuildAttempt ) / 300 ) % 2 )
    {
        color[ 0 ] = 1.0f;
        color[ 1 ] = color[ 2 ] = 0.0f;
        color[ 3 ] = 1.0f;
    }
    
    trap_R_SetColor( color );
    DrawPic( rect->x, rect->y, rect->w, rect->h, cgs.media.buildWeaponTimerPie[ index ] );
    trap_R_SetColor( NULL );
}

void idCGameLocal::DrawPlayerClipsValue( rectDef_t* rect, vec4_t color )
{
    S32           value;
    playerState_t* ps = &cg.snap->ps;
    
    switch( bggame->PrimaryWeapon( ps->stats ) )
    {
        case WP_NONE:
        case WP_BLASTER:
        case WP_ABUILD:
        case WP_ABUILD2:
        case WP_HBUILD:
            return;
            
        default:
            value = ps->clips;
            
            if( value > -1 )
            {
                trap_R_SetColor( color );
                DrawField( rect->x, rect->y, 4, rect->w / 4, rect->h, value );
                trap_R_SetColor( NULL );
            }
            break;
    }
}

void idCGameLocal::DrawPlayerHealthValue( rectDef_t* rect, vec4_t color )
{
    trap_R_SetColor( color );
    DrawField( rect->x, rect->y, 4, rect->w / 4, rect->h, cg.snap->ps.stats[ STAT_HEALTH ] );
    trap_R_SetColor( NULL );
}

/*
==============
DrawPlayerHealthCross
==============
*/
void idCGameLocal::DrawPlayerHealthCross( rectDef_t* rect, vec4_t ref_color )
{
    qhandle_t shader;
    vec4_t color;
    F32 ref_alpha;
    
    // Pick the current icon
    shader = cgs.media.healthCross;
    if( cg.snap->ps.stats[ STAT_STATE ] & SS_HEALING_3X )
        shader = cgs.media.healthCross3X;
    else if( cg.snap->ps.stats[ STAT_STATE ] & SS_HEALING_2X )
    {
        if( cg.snap->ps.stats[ STAT_TEAM ] == TEAM_ALIENS )
            shader = cgs.media.healthCross2X;
        else
            shader = cgs.media.healthCrossMedkit;
    }
    else if( cg.snap->ps.stats[ STAT_STATE ] & SS_POISONED )
        shader = cgs.media.healthCrossPoisoned;
        
    // Pick the alpha value
    Vector4Copy( ref_color, color );
    if( cg.snap->ps.stats[ STAT_TEAM ] == TEAM_HUMANS &&
            cg.snap->ps.stats[ STAT_HEALTH ] < 10 )
    {
        color[ 0 ] = 1.0f;
        color[ 1 ] = color[ 2 ] = 0.0f;
    }
    ref_alpha = ref_color[ 3 ];
    if( cg.snap->ps.stats[ STAT_STATE ] & SS_HEALING_ACTIVE )
        ref_alpha = 1.f;
        
    // Don't fade from nothing
    if( !cg.lastHealthCross )
        cg.lastHealthCross = shader;
        
    // Fade the icon during transition
    if( cg.lastHealthCross != shader )
    {
        cg.healthCrossFade += cg.frametime / 500.f;
        if( cg.healthCrossFade > 1.f )
        {
            cg.healthCrossFade = 0.f;
            cg.lastHealthCross = shader;
        }
        else
        {
            // Fading between two icons
            color[ 3 ] = ref_alpha * cg.healthCrossFade;
            trap_R_SetColor( color );
            DrawPic( rect->x, rect->y, rect->w, rect->h, shader );
            color[ 3 ] = ref_alpha * ( 1.f - cg.healthCrossFade );
            trap_R_SetColor( color );
            DrawPic( rect->x, rect->y, rect->w, rect->h, cg.lastHealthCross );
            trap_R_SetColor( NULL );
            return;
        }
    }
    
    // Not fading, draw a single icon
    color[ 3 ] = ref_alpha;
    trap_R_SetColor( color );
    DrawPic( rect->x, rect->y, rect->w, rect->h, shader );
    trap_R_SetColor( NULL );
}

F32 idCGameLocal::ChargeProgress( void )
{
    F32 progress;
    S32 min = 0, max = 0;
    
    if( cg.snap->ps.weapon == WP_ALEVEL3 )
    {
        min = LEVEL3_POUNCE_TIME_MIN;
        max = LEVEL3_POUNCE_TIME;
    }
    else if( cg.snap->ps.weapon == WP_ALEVEL3_UPG )
    {
        min = LEVEL3_POUNCE_TIME_MIN;
        max = LEVEL3_POUNCE_TIME_UPG;
    }
    else if( cg.snap->ps.weapon == WP_ALEVEL4 )
    {
        if( cg.predictedPlayerState.stats[ STAT_STATE ] & SS_CHARGING )
        {
            min = 0;
            max = LEVEL4_TRAMPLE_DURATION;
        }
        else
        {
            min = LEVEL4_TRAMPLE_CHARGE_MIN;
            max = LEVEL4_TRAMPLE_CHARGE_MAX;
        }
    }
    else if( cg.snap->ps.weapon == WP_LUCIFER_CANNON )
    {
        min = LCANNON_CHARGE_TIME_MIN;
        max = LCANNON_CHARGE_TIME_MAX;
    }
    if( max - min <= 0.f )
        return 0.f;
    progress = ( ( F32 )cg.predictedPlayerState.stats[ STAT_MISC ] - min ) /
               ( max - min );
    if( progress > 1.f )
        return 1.f;
    if( progress < 0.f )
        return 0.f;
    return progress;
}

#define CHARGE_BAR_FADE_RATE 0.002f

void idCGameLocal::DrawPlayerChargeBarBG( rectDef_t* rect, vec4_t ref_color, qhandle_t shader )
{
    vec4_t color;
    
    if( !cg_drawChargeBar.integer || cg.chargeMeterAlpha <= 0.f )
        return;
        
    color[ 0 ] = ref_color[ 0 ];
    color[ 1 ] = ref_color[ 1 ];
    color[ 2 ] = ref_color[ 2 ];
    color[ 3 ] = ref_color[ 3 ] * cg.chargeMeterAlpha;
    
    // Draw meter background
    trap_R_SetColor( color );
    DrawPic( rect->x, rect->y, rect->w, rect->h, shader );
    trap_R_SetColor( NULL );
}

// FIXME: This should come from the element info
#define CHARGE_BAR_CAP_SIZE 3

void idCGameLocal::DrawPlayerChargeBar( rectDef_t* rect, vec4_t ref_color, qhandle_t shader )
{
    vec4_t color;
    F32 x, y, width, height, cap_size, progress;
    
    if( !cg_drawChargeBar.integer )
        return;
        
    // Get progress proportion and pump fade
    progress = ChargeProgress();
    if( progress <= 0.f )
    {
        cg.chargeMeterAlpha -= CHARGE_BAR_FADE_RATE * cg.frametime;
        if( cg.chargeMeterAlpha <= 0.f )
        {
            cg.chargeMeterAlpha = 0.f;
            return;
        }
    }
    else
    {
        cg.chargeMeterValue = progress;
        cg.chargeMeterAlpha += CHARGE_BAR_FADE_RATE * cg.frametime;
        if( cg.chargeMeterAlpha > 1.f )
            cg.chargeMeterAlpha = 1.f;
    }
    
    color[ 0 ] = ref_color[ 0 ];
    color[ 1 ] = ref_color[ 1 ];
    color[ 2 ] = ref_color[ 2 ];
    color[ 3 ] = ref_color[ 3 ] * cg.chargeMeterAlpha;
    
    // Flash red for Lucifer Cannon warning
    if( cg.snap->ps.weapon == WP_LUCIFER_CANNON &&
            cg.snap->ps.stats[ STAT_MISC ] >= LCANNON_CHARGE_TIME_WARN &&
            ( cg.time & 128 ) )
    {
        color[ 0 ] = 1.f;
        color[ 1 ] = 0.f;
        color[ 2 ] = 0.f;
    }
    
    x = rect->x;
    y = rect->y;
    
    // Horizontal charge bar
    if( rect->w >= rect->h )
    {
        width = ( rect->w - CHARGE_BAR_CAP_SIZE * 2 ) * cg.chargeMeterValue;
        height = rect->h;
        AdjustFrom640( &x, &y, &width, &height );
        cap_size = CHARGE_BAR_CAP_SIZE * cgs.screenXScale;
        
        // Draw the meter
        trap_R_SetColor( color );
        trap_R_DrawStretchPic( x, y, cap_size, height, 0, 0, 1, 1, shader );
        trap_R_DrawStretchPic( x + width + cap_size, y, cap_size, height,
                               1, 0, 0, 1, shader );
        trap_R_DrawStretchPic( x + cap_size, y, width, height, 1, 0, 1, 1, shader );
        trap_R_SetColor( NULL );
    }
    
    // Vertical charge bar
    else
    {
        y += rect->h;
        width = rect->w;
        height = ( rect->h - CHARGE_BAR_CAP_SIZE * 2 ) * cg.chargeMeterValue;
        AdjustFrom640( &x, &y, &width, &height );
        cap_size = CHARGE_BAR_CAP_SIZE * cgs.screenYScale;
        
        // Draw the meter
        trap_R_SetColor( color );
        trap_R_DrawStretchPic( x, y - cap_size, width, cap_size,
                               0, 1, 1, 0, shader );
        trap_R_DrawStretchPic( x, y - height - cap_size * 2, width,
                               cap_size, 0, 0, 1, 1, shader );
        trap_R_DrawStretchPic( x, y - height - cap_size, width, height,
                               0, 1, 1, 1, shader );
        trap_R_SetColor( NULL );
    }
}

void idCGameLocal::DrawProgressLabel( rectDef_t* rect, F32 text_x, F32 text_y, vec4_t color, F32 scale, S32 textalign, S32 textvalign, StringEntry s, F32 fraction )
{
    vec4_t white = { 1.0f, 1.0f, 1.0f, 1.0f };
    F32 tx, ty;
    
    AlignText( rect, s, scale, 0.0f, 0.0f, textalign, textvalign, &tx, &ty );
    
    if( fraction < 1.0f )
        UI_Text_Paint( text_x + tx, text_y + ty, scale, white,
                       s, 0, 0, ITEM_TEXTSTYLE_NORMAL );
    else
        UI_Text_Paint( text_x + tx, text_y + ty, scale, color,
                       s, 0, 0, ITEM_TEXTSTYLE_NEON );
}

void idCGameLocal::DrawMediaProgress( rectDef_t* rect, vec4_t color, F32 scale, S32 align, S32 textalign, S32 textStyle, S32 special )
{
    DrawProgressBar( rect, color, scale, align, textalign, textStyle, special, cg.mediaFraction );
}

void idCGameLocal::DrawMediaProgressLabel( rectDef_t* rect, F32 text_x, F32 text_y, vec4_t color, F32 scale, S32 textalign, S32 textvalign )
{
    DrawProgressLabel( rect, text_x, text_y, color, scale, textalign, textvalign, "Map and Textures", cg.mediaFraction );
}

void idCGameLocal::DrawBuildablesProgress( rectDef_t* rect, vec4_t color, F32 scale, S32 align, S32 textalign, S32 textStyle, S32 special )
{
    DrawProgressBar( rect, color, scale, align, textalign, textStyle, special, cg.buildablesFraction );
}

void idCGameLocal::DrawBuildablesProgressLabel( rectDef_t* rect, F32 text_x, F32 text_y, vec4_t color, F32 scale, S32 textalign, S32 textvalign )
{
    DrawProgressLabel( rect, text_x, text_y, color, scale, textalign, textvalign, "Buildable Models", cg.buildablesFraction );
}

void idCGameLocal::DrawCharModelProgress( rectDef_t* rect, vec4_t color, F32 scale, S32 align, S32 textalign, S32 textStyle, S32 special )
{
    DrawProgressBar( rect, color, scale, align, textalign, textStyle, special, cg.charModelFraction );
}

void idCGameLocal::DrawCharModelProgressLabel( rectDef_t* rect, F32 text_x, F32 text_y, vec4_t color, F32 scale, S32 textalign, S32 textvalign )
{
    DrawProgressLabel( rect, text_x, text_y, color, scale, textalign, textvalign, "Character Models", cg.charModelFraction );
}

void idCGameLocal::DrawOverallProgress( rectDef_t* rect, vec4_t color, F32 scale, S32 align, S32 textalign, S32 textStyle, S32 special )
{
    F32 total;
    
    total = ( cg.charModelFraction + cg.buildablesFraction + cg.mediaFraction ) / 3.0f;
    DrawProgressBar( rect, color, scale, align, textalign, textStyle, special, total );
}

void idCGameLocal::DrawLevelShot( rectDef_t* rect )
{
    StringEntry  s;
    StringEntry  info;
    qhandle_t   levelshot;
    qhandle_t   detail;
    
    info = ConfigString( CS_SERVERINFO );
    s = Info_ValueForKey( info, "mapname" );
    levelshot = trap_R_RegisterShaderNoMip( va( "levelshots/%s.tga", s ) );
    
    if( !levelshot )
        levelshot = trap_R_RegisterShaderNoMip( "gfx/2d/load_screen" );
        
    trap_R_SetColor( NULL );
    DrawPic( rect->x, rect->y, rect->w, rect->h, levelshot );
    
    // blend a detail texture over it
    detail = trap_R_RegisterShader( "gfx/misc/detail" );
    DrawPic( rect->x, rect->y, rect->w, rect->h, detail );
}

void idCGameLocal::DrawLevelName( rectDef_t* rect, F32 text_x, F32 text_y, vec4_t color, F32 scale, S32 textalign, S32 textvalign, S32 textStyle )
{
    StringEntry  s;
    
    s = ConfigString( CS_MESSAGE );
    
    UI_DrawTextBlock( rect, text_x, text_y, color, scale, textalign, textvalign, textStyle, s );
}

void idCGameLocal::DrawMOTD( rectDef_t* rect, F32 text_x, F32 text_y, vec4_t color, F32 scale, S32 textalign, S32 textvalign, S32 textStyle )
{
    StringEntry  s;
    
    s = ConfigString( CS_MOTD );
    
    UI_DrawTextBlock( rect, text_x, text_y, color, scale, textalign, textvalign, textStyle, s );
}

void idCGameLocal::DrawHostname( rectDef_t* rect, F32 text_x, F32 text_y, vec4_t color, F32 scale, S32 textalign, S32 textvalign, S32 textStyle )
{
    UTF8 buffer[ 1024 ];
    StringEntry  info;
    
    info = ConfigString( CS_SERVERINFO );
    
    Q_strncpyz( buffer, Info_ValueForKey( info, "sv_hostname" ), 1024 );
    Q_CleanStr( buffer );
    
    UI_DrawTextBlock( rect, text_x, text_y, color, scale, textalign, textvalign, textStyle, buffer );
}

/*
==============
DrawDemoPlayback
==============
*/
void idCGameLocal::DrawDemoPlayback( rectDef_t* rect, vec4_t color, qhandle_t shader )
{
    if( !cg_drawDemoState.integer )
        return;
        
    if( trap_GetDemoState( ) != DS_PLAYBACK )
        return;
        
    trap_R_SetColor( color );
    DrawPic( rect->x, rect->y, rect->w, rect->h, shader );
    trap_R_SetColor( NULL );
}

/*
==============
DrawDemoRecording
==============
*/
void idCGameLocal::DrawDemoRecording( rectDef_t* rect, vec4_t color, qhandle_t shader )
{
    if( !cg_drawDemoState.integer )
        return;
        
    if( trap_GetDemoState( ) != DS_RECORDING )
        return;
        
    trap_R_SetColor( color );
    DrawPic( rect->x, rect->y, rect->w, rect->h, shader );
    trap_R_SetColor( NULL );
}

/*
======================
UpdateMediaFraction

======================
*/
void idCGameLocal::UpdateMediaFraction( F32 newFract )
{
    cg.mediaFraction = newFract;
    
    trap_UpdateScreen( );
}



/*
====================
DrawLoadingScreen

Draw all the status / pacifier stuff during level loading
====================
*/
void idCGameLocal::DrawLoadingScreen( void )
{
    Menu_Paint( Menus_FindByName( "Loading" ), true );
}

F32 idCGameLocal::GetValue( S32 ownerDraw )
{
    centity_t* cent;
    playerState_t* ps;
    weapon_t weapon;
    
    cent = &cg_entities[ cg.snap->ps.clientNum ];
    ps = &cg.snap->ps;
    weapon = bggame->GetPlayerWeapon( ps );
    
    switch( ownerDraw )
    {
        case CG_PLAYER_AMMO_VALUE:
            if( weapon )
                return ps->ammo;
            break;
        case CG_PLAYER_CLIPS_VALUE:
            if( weapon )
                return ps->clips;
            break;
        case CG_PLAYER_HEALTH:
            return ps->stats[ STAT_HEALTH ];
            break;
        default:
            break;
    }
    
    return -1;
}

StringEntry idCGameLocal::GetKillerText( void )
{
    StringEntry s = "";
    if( cg.killerName[ 0 ] )
        s = va( "Fragged by %s", cg.killerName );
        
    return s;
}

void idCGameLocal::DrawKiller( rectDef_t* rect, F32 scale, vec4_t color, qhandle_t shader, S32 textStyle )
{
    // fragged by ... line
    if( cg.killerName[ 0 ] )
    {
        S32 x = rect->x + rect->w / 2;
        UI_Text_Paint( x - UI_Text_Width( GetKillerText( ), scale, 0 ) / 2, rect->y + rect->h, scale, color, GetKillerText( ), 0, 0, textStyle );
    }
}

void idCGameLocal::DrawTeamSpectators( rectDef_t* rect, F32 scale, S32 textvalign, vec4_t color, qhandle_t shader )
{
    F32 y;
    
    if( cg.spectatorLen )
    {
        F32 maxX;
        
        if( cg.spectatorWidth == -1 )
        {
            cg.spectatorWidth = 0;
            cg.spectatorPaintX = rect->x + 1;
            cg.spectatorPaintX2 = -1;
        }
        
        if( cg.spectatorOffset > cg.spectatorLen )
        {
            cg.spectatorOffset = 0;
            cg.spectatorPaintX = rect->x + 1;
            cg.spectatorPaintX2 = -1;
        }
        
        if( cg.time > cg.spectatorTime )
        {
            cg.spectatorTime = cg.time + 10;
            
            if( cg.spectatorPaintX <= rect->x + 2 )
            {
                if( cg.spectatorOffset < cg.spectatorLen )
                {
                    // skip colour directives
                    if( Q_IsColorString( &cg.spectatorList[ cg.spectatorOffset ] ) )
                        cg.spectatorOffset += 2;
                    else
                    {
                        cg.spectatorPaintX += UI_Text_Width( &cg.spectatorList[ cg.spectatorOffset ], scale, 1 ) - 1;
                        cg.spectatorOffset++;
                    }
                }
                else
                {
                    cg.spectatorOffset = 0;
                    
                    if( cg.spectatorPaintX2 >= 0 )
                        cg.spectatorPaintX = cg.spectatorPaintX2;
                    else
                        cg.spectatorPaintX = rect->x + rect->w - 2;
                        
                    cg.spectatorPaintX2 = -1;
                }
            }
            else
            {
                cg.spectatorPaintX--;
                
                if( cg.spectatorPaintX2 >= 0 )
                    cg.spectatorPaintX2--;
            }
        }
        
        maxX = rect->x + rect->w - 2;
        AlignText( rect, NULL, 0.0f, 0.0f, UI_Text_EmHeight( scale ),
                   ALIGN_LEFT, textvalign, NULL, &y );
                   
        UI_Text_Paint_Limit( &maxX, cg.spectatorPaintX, y, scale, color,
                             &cg.spectatorList[ cg.spectatorOffset ], 0, 0 );
                             
        if( cg.spectatorPaintX2 >= 0 )
        {
            F32 maxX2 = rect->x + rect->w - 2;
            UI_Text_Paint_Limit( &maxX2, cg.spectatorPaintX2, y, scale,
                                 color, cg.spectatorList, 0, cg.spectatorOffset );
        }
        
        if( cg.spectatorOffset && maxX > 0 )
        {
            // if we have an offset ( we are skipping the first part of the string ) and we fit the string
            if( cg.spectatorPaintX2 == -1 )
                cg.spectatorPaintX2 = rect->x + rect->w - 2;
        }
        else
            cg.spectatorPaintX2 = -1;
    }
}

/*
==================
DrawTeamLabel
==================
*/
void idCGameLocal::DrawTeamLabel( rectDef_t* rect, team_t team, F32 text_x, F32 text_y, vec4_t color, F32 scale, S32 textalign, S32 textvalign, S32 textStyle )
{
    UTF8*  t;
    UTF8  stage[ MAX_TOKEN_CHARS ];
    UTF8*  s;
    F32 tx, ty;
    
    stage[ 0 ] = '\0';
    
    switch( team )
    {
        case TEAM_ALIENS:
            t = "Aliens";
            if( cg.intermissionStarted )
                Com_sprintf( stage, MAX_TOKEN_CHARS, "(Stage %d)", cgs.alienStage + 1 );
            break;
            
        case TEAM_HUMANS:
            t = "Humans";
            if( cg.intermissionStarted )
                Com_sprintf( stage, MAX_TOKEN_CHARS, "(Stage %d)", cgs.humanStage + 1 );
            break;
            
        default:
            t = "";
            break;
    }
    
    switch( textalign )
    {
        default:
        case ALIGN_LEFT:
            s = va( "%s %s", t, stage );
            break;
            
        case ALIGN_RIGHT:
            s = va( "%s %s", stage, t );
            break;
    }
    
    AlignText( rect, s, scale, 0.0f, 0.0f, textalign, textvalign, &tx, &ty );
    UI_Text_Paint( text_x + tx, text_y + ty, scale, color, s, 0, 0, textStyle );
}

/*
==================
DrawStageReport
==================
*/
void idCGameLocal::DrawStageReport( rectDef_t* rect, F32 text_x, F32 text_y, vec4_t color, F32 scale, S32 textalign, S32 textvalign, S32 textStyle )
{
    UTF8  s[ MAX_TOKEN_CHARS ];
    UTF8* reward;
    F32 tx, ty;
    
    if( cg.intermissionStarted )
        return;
        
    if( cg.snap->ps.stats[ STAT_TEAM ] == TEAM_NONE )
        return;
        
    if( cg.snap->ps.stats[ STAT_TEAM ] == TEAM_ALIENS )
    {
        S32 frags = ceil( ( F32 )( cgs.alienNextStageThreshold - cgs.alienCredits ) / ALIEN_CREDITS_PER_FRAG );
        if( frags < 0 )
            frags = 0;
            
        if( cgs.alienStage < S3 )
            reward = "next stage";
        else
            reward = "enemy stagedown";
            
        if( cgs.alienNextStageThreshold < 0 )
            Com_sprintf( s, MAX_TOKEN_CHARS, "Stage %d", cgs.alienStage + 1 );
        else if( frags == 1 )
            Com_sprintf( s, MAX_TOKEN_CHARS, "Stage %d, 1 frag for %s",
                         cgs.alienStage + 1, reward );
        else
            Com_sprintf( s, MAX_TOKEN_CHARS, "Stage %d, %d frags for %s",
                         cgs.alienStage + 1, frags, reward );
    }
    else if( cg.snap->ps.stats[ STAT_TEAM ] == TEAM_HUMANS )
    {
        S32 credits = cgs.humanNextStageThreshold - cgs.humanCredits;
        
        if( credits < 0 )
            credits = 0;
            
        if( cgs.humanStage < S3 )
            reward = "next stage";
        else
            reward = "enemy stagedown";
            
        if( cgs.humanNextStageThreshold < 0 )
            Com_sprintf( s, MAX_TOKEN_CHARS, "Stage %d", cgs.humanStage + 1 );
        else if( credits == 1 )
            Com_sprintf( s, MAX_TOKEN_CHARS, "Stage %d, 1 credit for %s",
                         cgs.humanStage + 1, reward );
        else
            Com_sprintf( s, MAX_TOKEN_CHARS, "Stage %d, %d credits for %s",
                         cgs.humanStage + 1, credits, reward );
    }
    
    AlignText( rect, s, scale, 0.0f, 0.0f, textalign, textvalign, &tx, &ty );
    
    UI_Text_Paint( text_x + tx, text_y + ty, scale, color, s, 0, 0, textStyle );
}

/*
==================
DrawFPS
==================
*/
#define FPS_FRAMES  20
#define FPS_STRING  "fps"
void idCGameLocal::DrawFPS( rectDef_t* rect, F32 text_x, F32 text_y, F32 scale, vec4_t color, S32 textalign, S32 textvalign, S32 textStyle, bool scalableText )
{
    UTF8*        s;
    F32       tx, ty;
    F32       w, h, totalWidth;
    S32         strLength;
    static S32  previousTimes[ FPS_FRAMES ];
    static S32  index;
    S32         i, total;
    S32         fps;
    static S32  previous;
    S32         t, frameTime;
    
    if( !cg_drawFPS.integer )
        return;
        
    // don't use serverTime, because that will be drifting to
    // correct for internet lag changes, timescales, timedemos, etc
    t = trap_Milliseconds( );
    frameTime = t - previous;
    previous = t;
    
    previousTimes[ index % FPS_FRAMES ] = frameTime;
    index++;
    
    if( index > FPS_FRAMES )
    {
        // average multiple frames together to smooth changes out a bit
        total = 0;
        
        for( i = 0 ; i < FPS_FRAMES ; i++ )
            total += previousTimes[ i ];
            
        if( !total )
            total = 1;
            
        fps = 1000 * FPS_FRAMES / total;
        
        s = va( "%d", fps );
        w = UI_Text_Width( "0", scale, 0 );
        h = UI_Text_Height( "0", scale, 0 );
        strLength = DrawStrlen( s );
        totalWidth = UI_Text_Width( FPS_STRING, scale, 0 ) + w * strLength;
        
        AlignText( rect, s, 0.0f, totalWidth, h, textalign, textvalign, &tx, &ty );
        
        if( scalableText )
        {
            for( i = 0; i < strLength; i++ )
            {
                UTF8 c[ 2 ];
                
                c[ 0 ] = s[ i ];
                c[ 1 ] = '\0';
                
                UI_Text_Paint( text_x + tx + i * w, text_y + ty, scale, color, c, 0, 0, textStyle );
            }
            
            UI_Text_Paint( text_x + tx + i * w, text_y + ty, scale, color, FPS_STRING, 0, 0, textStyle );
        }
        else
        {
            trap_R_SetColor( color );
            DrawField( rect->x, rect->y, 3, rect->w / 3, rect->h, fps );
            trap_R_SetColor( NULL );
        }
    }
}

/*
==================
DrawSpeed
==================
*/
void idCGameLocal::DrawSpeed( rectDef_t* rect, F32 text_x, F32 text_y, F32 scale, vec4_t color, S32 textalign, S32 textvalign, S32 textStyle )
{
    UTF8*          s;
    F32         tx, ty;
    F32         w, h, totalWidth;
    S32           i, strLength;
    playerState_t* ps;
    F32         speed;
    static F32  speedRecord = 0;
    static F32  previousSpeed = 0;
    static vec4_t previousColor = { 0, 1, 0, 1 };
    
    if( cg.snap->ps.pm_type == PM_INTERMISSION )
        return;
        
    if( !cg_drawSpeed.integer )
        return;
        
    if( cg_drawSpeed.integer > 1 )
    {
        speedRecord = 0;
        trap_Cvar_Set( "cg_drawSpeed", "1" );
    }
    
    ps = &cg.snap->ps;
    speed = VectorLength( ps->velocity );
    
    if( speed > speedRecord )
        speedRecord = speed;
        
    if( floor( speed ) > floor( previousSpeed ) )
    {
        color[0] = 0;
        color[1] = 1;
        color[2] = 0;
        color[3] = 1;
    }
    else if( floor( speed ) < floor( previousSpeed ) )
    {
        color[0] = 1;
        color[1] = 0;
        color[2] = 0;
        color[3] = 1;
    }
    else
    {
        color[0] = previousColor[0];
        color[1] = previousColor[1];
        color[2] = previousColor[2];
        color[3] = previousColor[3];
    }
    
    previousColor[0] = color[0];
    previousColor[1] = color[1];
    previousColor[2] = color[2];
    previousColor[3] = color[3];
    
    previousSpeed = speed;
    
    s = va( "Speed: %.0f/%.0f", speed, speedRecord );
    w = UI_Text_Width( "0", scale, 0 );
    h = UI_Text_Height( "0", scale, 0 );
    strLength = DrawStrlen( s );
    totalWidth = UI_Text_Width( FPS_STRING, scale, 0 ) + w * strLength;
    
    AlignText( rect, s, 0.0f, totalWidth, h, textalign, textvalign, &tx, &ty );
    
    for( i = 0; i < strLength; i++ )
    {
        UTF8 c[ 2 ];
        
        c[ 0 ] = s[ i ];
        c[ 1 ] = '\0';
        
        UI_Text_Paint( text_x + tx + i * w, text_y + ty, scale, color, c, 0, 0, textStyle );
    }
}


/*
=================
DrawTimerMins
=================
*/
void idCGameLocal::DrawTimerMins( rectDef_t* rect, vec4_t color )
{
    S32 mins, seconds, msec;
    
    if( !cg_drawTimer.integer )
        return;
        
    msec = cg.time - cgs.levelStartTime;
    
    seconds = msec / 1000;
    mins = seconds / 60;
    seconds -= mins * 60;
    
    trap_R_SetColor( color );
    DrawField( rect->x, rect->y, 3, rect->w / 3, rect->h, mins );
    trap_R_SetColor( NULL );
}


/*
=================
DrawTimerSecs
=================
*/
void idCGameLocal::DrawTimerSecs( rectDef_t* rect, vec4_t color )
{
    S32 mins, seconds, msec;
    
    if( !cg_drawTimer.integer )
        return;
        
    msec = cg.time - cgs.levelStartTime;
    
    seconds = msec / 1000;
    mins = seconds / 60;
    seconds -= mins * 60;
    
    trap_R_SetColor( color );
    DrawFieldPadded( rect->x, rect->y, 2, rect->w / 2, rect->h, seconds );
    trap_R_SetColor( NULL );
}

/*
=================
DrawTimer
=================
*/
void idCGameLocal::DrawTimer( rectDef_t* rect, F32 text_x, F32 text_y, F32 scale, vec4_t color, S32 textalign, S32 textvalign, S32 textStyle )
{
    UTF8* s;
    F32 tx, ty, w, h, totalWidth;
    S32 i, strLength, mins, seconds, tens, msec;
    
    if( !cg_drawTimer.integer )
        return;
        
    msec = cg.time - cgs.levelStartTime;
    
    seconds = msec / 1000;
    mins = seconds / 60;
    seconds -= mins * 60;
    tens = seconds / 10;
    seconds -= tens * 10;
    
    s = va( "%d:%d%d", mins, tens, seconds );
    w = UI_Text_Width( "0", scale, 0 );
    h = UI_Text_Height( "0", scale, 0 );
    strLength = DrawStrlen( s );
    totalWidth = w * strLength;
    
    AlignText( rect, s, 0.0f, totalWidth, h, textalign, textvalign, &tx, &ty );
    
    for( i = 0; i < strLength; i++ )
    {
        UTF8 c[ 2 ];
        
        c[ 0 ] = s[ i ];
        c[ 1 ] = '\0';
        
        UI_Text_Paint( text_x + tx + i * w, text_y + ty, scale, color, c, 0, 0, textStyle );
    }
}

/*
=================
DrawClock
=================
*/
void idCGameLocal::DrawClock( rectDef_t* rect, F32 text_x, F32 text_y, F32 scale, vec4_t color, S32 textalign, S32 textvalign, S32 textStyle )
{
    UTF8*    s;
    F32   tx, ty;
    S32     i, strLength;
    F32   w, h, totalWidth;
    qtime_t qt;
    S32     t;
    
    if( !cg_drawClock.integer )
        return;
        
    t = trap_RealTime( &qt );
    
    if( cg_drawClock.integer == 2 )
    {
        s = va( "%02d%s%02d", qt.tm_hour, ( qt.tm_sec % 2 ) ? ":" : " ",
                qt.tm_min );
    }
    else
    {
        UTF8* pm = "am";
        S32 h = qt.tm_hour;
        
        if( h == 0 )
            h = 12;
        else if( h == 12 )
            pm = "pm";
        else if( h > 12 )
        {
            h -= 12;
            pm = "pm";
        }
        
        s = va( "%d%s%02d%s", h, ( qt.tm_sec % 2 ) ? ":" : " ", qt.tm_min, pm );
    }
    w = UI_Text_Width( "0", scale, 0 );
    h = UI_Text_Height( "0", scale, 0 );
    strLength = DrawStrlen( s );
    totalWidth = w * strLength;
    
    AlignText( rect, s, 0.0f, totalWidth, h, textalign, textvalign, &tx, &ty );
    
    for( i = 0; i < strLength; i++ )
    {
        UTF8 c[ 2 ];
        
        c[ 0 ] = s[ i ];
        c[ 1 ] = '\0';
        
        UI_Text_Paint( text_x + tx + i * w, text_y + ty, scale, color, c, 0, 0, textStyle );
    }
}

/*
==================
DrawSnapshot
==================
*/
void idCGameLocal::DrawSnapshot( rectDef_t* rect, F32 text_x, F32 text_y, F32 scale, vec4_t color, S32 textalign, S32 textvalign, S32 textStyle )
{
    UTF8*    s;
    F32   tx, ty;
    
    if( !cg_drawSnapshot.integer )
        return;
        
    s = va( "time:%d snap:%d cmd:%d", cg.snap->serverTime, cg.latestSnapshotNum, cgs.serverCommandSequence );
    
    AlignText( rect, s, scale, 0.0f, 0.0f, textalign, textvalign, &tx, &ty );
    
    UI_Text_Paint( text_x + tx, text_y + ty, scale, color, s, 0, 0, textStyle );
}

/*
===============================================================================

LAGOMETER

===============================================================================
*/

#define LAG_SAMPLES   128

typedef struct
{
    S32 frameSamples[ LAG_SAMPLES ];
    S32 frameCount;
    S32 snapshotFlags[ LAG_SAMPLES ];
    S32 snapshotSamples[ LAG_SAMPLES ];
    S32 snapshotCount;
} lagometer_t;

lagometer_t   lagometer;

/*
==============
AddLagometerFrameInfo

Adds the current interpolate / extrapolate bar for this frame
==============
*/
void idCGameLocal::AddLagometerFrameInfo( void )
{
    S32     offset;
    
    offset = cg.time - cg.latestSnapshotTime;
    lagometer.frameSamples[ lagometer.frameCount & ( LAG_SAMPLES - 1 ) ] = offset;
    lagometer.frameCount++;
}

/*
==============
AddLagometerSnapshotInfo

Each time a snapshot is received, log its ping time and
the number of snapshots that were dropped before it.

Pass NULL for a dropped packet.
==============
*/
#define PING_FRAMES 40
void idCGameLocal::AddLagometerSnapshotInfo( snapshot_t* snap )
{
    static S32  previousPings[ PING_FRAMES ];
    static S32  index;
    S32         i;
    
    // dropped packet
    if( !snap )
    {
        lagometer.snapshotSamples[ lagometer.snapshotCount & ( LAG_SAMPLES - 1 ) ] = -1;
        lagometer.snapshotCount++;
        return;
    }
    
    // add this snapshot's info
    lagometer.snapshotSamples[ lagometer.snapshotCount & ( LAG_SAMPLES - 1 ) ] = snap->ping;
    lagometer.snapshotFlags[ lagometer.snapshotCount & ( LAG_SAMPLES - 1 ) ] = snap->snapFlags;
    lagometer.snapshotCount++;
    
    cg.ping = 0;
    if( cg.snap )
    {
        previousPings[ index++ ] = cg.snap->ping;
        index = index % PING_FRAMES;
        
        for( i = 0; i < PING_FRAMES; i++ )
        {
            cg.ping += previousPings[ i ];
        }
        
        cg.ping /= PING_FRAMES;
    }
}

/*
==============
DrawDisconnect

Should we draw something differnet for long lag vs no packets?
==============
*/
void idCGameLocal::DrawDisconnect( void )
{
    F32       x, y;
    S32         cmdNum;
    usercmd_t   cmd;
    StringEntry  s;
    S32         w;
    vec4_t      color = { 1.0f, 1.0f, 1.0f, 1.0f };
    
    // draw the phone jack if we are completely past our buffers
    cmdNum = trap_GetCurrentCmdNumber( ) - CMD_BACKUP + 1;
    trap_GetUserCmd( cmdNum, &cmd );
    
    // special check for map_restart
    if( cmd.serverTime <= cg.snap->ps.commandTime || cmd.serverTime > cg.time )
        return;
        
    // also add text in center of screen
    s = "Connection Interrupted";
    w = UI_Text_Width( s, 0.7f, 0 );
    UI_Text_Paint( 320 - w / 2, 100, 0.7f, color, s, 0, 0, ITEM_TEXTSTYLE_SHADOWED );
    
    // blink the icon
    if( ( cg.time >> 9 ) & 1 )
        return;
        
    x = 640 - 48;
    y = 480 - 48;
    
    DrawPic( x, y, 48, 48, trap_R_RegisterShader( "gfx/2d/net.tga" ) );
}

#define MAX_LAGOMETER_PING  900
#define MAX_LAGOMETER_RANGE 300


/*
==============
DrawLagometer
==============
*/
void idCGameLocal::DrawLagometer( rectDef_t* rect, F32 text_x, F32 text_y, F32 scale, vec4_t textColor )
{
    S32     a, x, y, i;
    F32   v;
    F32   ax, ay, aw, ah, mid, range;
    S32     color;
    vec4_t  adjustedColor;
    F32   vscale;
    vec4_t  white = { 1.0f, 1.0f, 1.0f, 1.0f };
    
    if( cg.snap->ps.pm_type == PM_INTERMISSION )
        return;
        
    if( !cg_lagometer.integer )
        return;
        
    if( cg.demoPlayback )
        return;
        
    Vector4Copy( textColor, adjustedColor );
    adjustedColor[ 3 ] = 0.25f;
    
    trap_R_SetColor( adjustedColor );
    DrawPic( rect->x, rect->y, rect->w, rect->h, cgs.media.whiteShader );
    trap_R_SetColor( NULL );
    
    //
    // draw the graph
    //
    ax = x = rect->x;
    ay = y = rect->y;
    aw = rect->w;
    ah = rect->h;
    
    trap_R_SetColor( NULL );
    
    AdjustFrom640( &ax, &ay, &aw, &ah );
    
    color = -1;
    range = ah / 3;
    mid = ay + range;
    
    vscale = range / MAX_LAGOMETER_RANGE;
    
    // draw the frame interpoalte / extrapolate graph
    for( a = 0 ; a < aw ; a++ )
    {
        i = ( lagometer.frameCount - 1 - a ) & ( LAG_SAMPLES - 1 );
        v = lagometer.frameSamples[ i ];
        v *= vscale;
        
        if( v > 0 )
        {
            if( color != 1 )
            {
                color = 1;
                trap_R_SetColor( g_color_table[ ColorIndex( COLOR_YELLOW ) ] );
            }
            
            if( v > range )
                v = range;
                
            trap_R_DrawStretchPic( ax + aw - a, mid - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader );
        }
        else if( v < 0 )
        {
            if( color != 2 )
            {
                color = 2;
                trap_R_SetColor( g_color_table[ ColorIndex( COLOR_BLUE ) ] );
            }
            
            v = -v;
            if( v > range )
                v = range;
                
            trap_R_DrawStretchPic( ax + aw - a, mid, 1, v, 0, 0, 0, 0, cgs.media.whiteShader );
        }
    }
    
    // draw the snapshot latency / drop graph
    range = ah / 2;
    vscale = range / MAX_LAGOMETER_PING;
    
    for( a = 0 ; a < aw ; a++ )
    {
        i = ( lagometer.snapshotCount - 1 - a ) & ( LAG_SAMPLES - 1 );
        v = lagometer.snapshotSamples[ i ];
        
        if( v > 0 )
        {
            if( lagometer.snapshotFlags[ i ] & SNAPFLAG_RATE_DELAYED )
            {
                if( color != 5 )
                {
                    color = 5;  // YELLOW for rate delay
                    trap_R_SetColor( g_color_table[ ColorIndex( COLOR_YELLOW ) ] );
                }
            }
            else
            {
                if( color != 3 )
                {
                    color = 3;
                    
                    trap_R_SetColor( g_color_table[ ColorIndex( COLOR_GREEN ) ] );
                }
            }
            
            v = v * vscale;
            
            if( v > range )
                v = range;
                
            trap_R_DrawStretchPic( ax + aw - a, ay + ah - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader );
        }
        else if( v < 0 )
        {
            if( color != 4 )
            {
                color = 4;    // RED for dropped snapshots
                trap_R_SetColor( g_color_table[ ColorIndex( COLOR_RED ) ] );
            }
            
            trap_R_DrawStretchPic( ax + aw - a, ay + ah - range, 1, range, 0, 0, 0, 0, cgs.media.whiteShader );
        }
    }
    
    trap_R_SetColor( NULL );
    
    if( cg_nopredict.integer || cg_synchronousClients.integer )
        UI_Text_Paint( ax, ay, 0.5, white, "snc", 0, 0, ITEM_TEXTSTYLE_NORMAL );
    else
    {
        UTF8*        s;
        
        s = va( "%d", cg.ping );
        ax = rect->x + ( rect->w / 2.0f ) - ( UI_Text_Width( s, scale, 0 ) / 2.0f ) + text_x;
        ay = rect->y + ( rect->h / 2.0f ) + ( UI_Text_Height( s, scale, 0 ) / 2.0f ) + text_y;
        
        Vector4Copy( textColor, adjustedColor );
        adjustedColor[ 3 ] = 0.5f;
        UI_Text_Paint( ax, ay, scale, adjustedColor, s, 0, 0, ITEM_TEXTSTYLE_NORMAL );
    }
    
    DrawDisconnect( );
}

/*
===================
DrawConsole
===================
*/
void idCGameLocal::DrawConsole( rectDef_t* rect, F32 text_x, F32 text_y, vec4_t color, F32 scale, S32 textalign, S32 textvalign, S32 textStyle )
{
    UI_DrawTextBlock( rect, text_x, text_y, color, scale, textalign, textvalign, textStyle, cg.consoleText );
}

/*
===================
DrawTutorial
===================
*/
void idCGameLocal::DrawTutorial( rectDef_t* rect, F32 text_x, F32 text_y, vec4_t color, F32 scale, S32 textalign, S32 textvalign, S32 textStyle )
{
    if( !cg_tutorial.integer )
        return;
        
    UI_DrawTextBlock( rect, text_x, text_y, color, scale, textalign, textvalign, textStyle, TutorialText( ) );
}

/*
===================
DrawWeaponIcon
===================
*/
void idCGameLocal::DrawWeaponIcon( rectDef_t* rect, vec4_t color )
{
    centity_t*     cent;
    playerState_t* ps;
    weapon_t      weapon;
    
    cent = &cg_entities[ cg.snap->ps.clientNum ];
    ps = &cg.snap->ps;
    weapon = bggame->GetPlayerWeapon( ps );
    
    // don't display if dead
    if( cg.predictedPlayerState.stats[ STAT_HEALTH ] <= 0 )
        return;
        
    if( weapon == 0 )
        return;
        
    RegisterWeapon( weapon );
    
    if( ps->clips == 0 && !bggame->Weapon( weapon )->infiniteAmmo )
    {
        F32 ammoPercent = ( F32 )ps->ammo / ( F32 )bggame->Weapon( weapon )->maxAmmo;
        
        if( ammoPercent < 0.33f )
        {
            color[ 0 ] = 1.0f;
            color[ 1 ] = color[ 2 ] = 0.0f;
        }
    }
    
    if( cg.predictedPlayerState.stats[ STAT_TEAM ] == TEAM_ALIENS &&
            !bggame->AlienCanEvolve( ( class_t )cg.predictedPlayerState.stats[ STAT_CLASS ],
                                     ps->persistant[ PERS_CREDIT ], cgs.alienStage ) )
    {
        if( cg.time - cg.lastEvolveAttempt <= NO_CREDITS_TIME )
        {
            if( ( ( cg.time - cg.lastEvolveAttempt ) / 300 ) % 2 )
                color[ 3 ] = 0.0f;
        }
    }
    
    trap_R_SetColor( color );
    DrawPic( rect->x, rect->y, rect->w, rect->h,
             cg_weapons[ weapon ].weaponIcon );
    trap_R_SetColor( NULL );
}



/*
================================================================================

CROSSHAIR

================================================================================
*/


/*
=================
DrawCrosshair
=================
*/
void idCGameLocal::DrawCrosshair( rectDef_t* rect, vec4_t color )
{
    F32         w, h;
    qhandle_t     hShader;
    F32         x, y;
    weaponInfo_t*  wi;
    weapon_t      weapon;
    
    weapon = bggame->GetPlayerWeapon( &cg.snap->ps );
    
    if( cg_drawCrosshair.integer == CROSSHAIR_ALWAYSOFF )
        return;
        
    if( cg_drawCrosshair.integer == CROSSHAIR_RANGEDONLY &&
            !bggame->Weapon( weapon )->longRanged )
        return;
        
    if( ( cg.snap->ps.persistant[ PERS_SPECSTATE ] != SPECTATOR_NOT ) ||
            ( cg.snap->ps.stats[ STAT_STATE ] & SS_HOVELING ) )
        return;
        
    if( cg.renderingThirdPerson )
        return;
        
    wi = &cg_weapons[ weapon ];
    
    w = h = wi->crossHairSize * cg_crosshairSize.value;
    w *= cgDC.aspectScale;
    
    //FIXME: this still ignores the width/height of the rect, but at least it's
    //neater than cg_crosshairX/cg_crosshairY
    x = rect->x + ( rect->w / 2 ) - ( w / 2 );
    y = rect->y + ( rect->h / 2 ) - ( h / 2 );
    
    hShader = wi->crossHair;
    
    //aiming at a friendly player/buildable, dim the crosshair
    if( cg.time == cg.crosshairClientTime || cg.crosshairBuildable >= 0 )
    {
        S32 i;
        for( i = 0; i < 3; i++ )
            color[i] *= .5f;
    }
    
    if( hShader != 0 )
    {
        trap_R_SetColor( color );
        DrawPic( x, y, w, h, hShader );
        trap_R_SetColor( NULL );
    }
}

/*
=================
ScanForCrosshairEntity
=================
*/
void idCGameLocal::ScanForCrosshairEntity( void )
{
    trace_t   trace;
    vec3_t    start, end;
    S32       content;
    team_t    team;
    
    VectorCopy( cg.refdef.vieworg, start );
    VectorMA( start, 131072, cg.refdef.viewaxis[ 0 ], end );
    
    Trace( &trace, start, vec3_origin, vec3_origin, end, cg.snap->ps.clientNum, CONTENTS_SOLID | CONTENTS_BODY );
    
    // if the player is in fog, don't show it
    content = trap_CM_PointContents( trace.endpos, 0 );
    if( content & CONTENTS_FOG )
        return;
        
    if( trace.entityNum >= MAX_CLIENTS )
    {
        entityState_t* s = &cg_entities[ trace.entityNum ].currentState;
        if( s->eType == ET_BUILDABLE && bggame->Buildable( ( buildable_t )s->modelindex )->team ==
                cg.snap->ps.stats[ STAT_TEAM ] )
            cg.crosshairBuildable = trace.entityNum;
        else
            cg.crosshairBuildable = -1;
        return;
    }
    
    team = cgs.clientinfo[ trace.entityNum ].team;
    
    if( cg.snap->ps.persistant[ PERS_SPECSTATE ] == SPECTATOR_NOT )
    {
        //only display team names of those on the same team as this player
        if( team != cg.snap->ps.stats[ STAT_TEAM ] )
            return;
    }
    
    // update the fade timer
    cg.crosshairClientNum = trace.entityNum;
    cg.crosshairClientTime = cg.time;
}


/*
=====================
DrawLocation
=====================
*/
void idCGameLocal::DrawLocation( rectDef_t* rect, F32 scale, S32 textalign, vec4_t color )
{
    StringEntry    location;
    centity_t*     locent;
    F32         maxX;
    F32         tx = rect->x, ty = rect->y;
    maxX = rect->x + rect->w;
    
    locent = GetPlayerLocation( );
    if( locent )
        location = ConfigString( CS_LOCATIONS + locent->currentState.generic1 );
    else
        location = ConfigString( CS_LOCATIONS );
    if( UI_Text_Width( location, scale, 0 ) < rect->w )
        AlignText( rect, location, scale, 0.0f, 0.0f, textalign, VALIGN_CENTER, &tx, &ty );
        
    UI_Text_Paint_Limit( &maxX, tx, ty, scale, color, location, 0, 0 );
    trap_R_SetColor( NULL );
}

/*
=====================
DrawCrosshairNames
=====================
*/
void idCGameLocal::DrawCrosshairNames( rectDef_t* rect, F32 scale, S32 textStyle )
{
    F32*   color;
    UTF8*    name;
    F32   w, x;
    
    if( !cg_drawCrosshairNames.integer )
        return;
        
    if( cg.renderingThirdPerson )
        return;
        
    // scan the known entities to see if the crosshair is sighted on one
    ScanForCrosshairEntity( );
    
    // draw the name of the player being looked at
    color = FadeColor( cg.crosshairClientTime, CROSSHAIR_CLIENT_TIMEOUT );
    if( !color )
    {
        trap_R_SetColor( NULL );
        return;
    }
    
    name = cgs.clientinfo[ cg.crosshairClientNum ].name;
    w = UI_Text_Width( name, scale, 0 );
    x = rect->x + rect->w / 2;
    UI_Text_Paint( x - w / 2, rect->y + rect->h, scale, color, name, 0, 0, textStyle );
    trap_R_SetColor( NULL );
}


/*
===============
DrawSquadMarkers
===============
*/
#define SQUAD_MARKER_W        16.f
#define SQUAD_MARKER_H        8.f
#define SQUAD_MARKER_BORDER   8.f
void idCGameLocal::DrawSquadMarkers( vec4_t color )
{
    centity_t* cent;
    vec3_t origin;
    qhandle_t shader;
    F32 x, y, w, h, distance, scale, u1 = 0.f, v1 = 0.f, u2 = 1.f, v2 = 1.f;
    S32 i;
    bool vertical, flip;
    
    if( cg.snap->ps.persistant[ PERS_SPECSTATE ] != SPECTATOR_NOT )
        return;
    trap_R_SetColor( color );
    for( i = 0; i < cg.snap->numEntities; i++ )
    {
        cent = cg_entities + cg.snap->entities[ i ].number;
        if( cent->currentState.eType != ET_PLAYER ||
                cgs.clientinfo[ cg.snap->entities[ i ].number ].team !=
                cg.snap->ps.stats[ STAT_TEAM ] ||
                !cent->pe.squadMarked )
            continue;
            
        // Find where on screen the player is
        VectorCopy( cent->lerpOrigin, origin );
        origin[ 2 ] += ( ( cent->currentState.solid >> 16 ) & 255 ) - 30;
        if( !WorldToScreenWrap( origin, &x, &y ) )
            continue;
            
        // Scale the size of the marker with distance
        distance = Distance( cent->lerpOrigin, cg.refdef.vieworg );
        if( !distance )
            continue;
        scale = 200.f / distance;
        if( scale > 1.f )
            scale = 1.f;
        if( scale < 0.25f )
            scale = 0.25f;
            
        // Don't let the marker go off-screen
        vertical = false;
        flip = false;
        if( x < SQUAD_MARKER_BORDER )
        {
            x = SQUAD_MARKER_BORDER;
            vertical = true;
            flip = false;
        }
        if( x > 640.f - SQUAD_MARKER_BORDER )
        {
            x = 640.f - SQUAD_MARKER_BORDER;
            vertical = true;
            flip = true;
        }
        if( y < SQUAD_MARKER_BORDER )
        {
            y = SQUAD_MARKER_BORDER;
            vertical = false;
            flip = true;
        }
        if( y > 480.f - SQUAD_MARKER_BORDER )
        {
            y = 480.f - SQUAD_MARKER_BORDER;
            vertical = false;
            flip = false;
        }
        
        // Draw the marker
        if( vertical )
        {
            shader = cgs.media.squadMarkerV;
            if( flip )
            {
                u1 = 1.f;
                u2 = 0.f;
            }
            w = SQUAD_MARKER_H * scale;
            h = SQUAD_MARKER_W * scale;
        }
        else
        {
            shader = cgs.media.squadMarkerH;
            if( flip )
            {
                v1 = 1.f;
                v2 = 0.f;
            }
            w = SQUAD_MARKER_W * scale;
            h = SQUAD_MARKER_H * scale;
        }
        AdjustFrom640( &x, &y, &w, &h );
        trap_R_DrawStretchPic( x - w / 2, y - h / 2, w, h, u1, v1, u2, v2,
                               shader );
    }
    trap_R_SetColor( NULL );
}

/*
===============
OwnerDraw

Draw an owner drawn item
===============
*/
void idCGameLocal::OwnerDraw( F32 x, F32 y, F32 w, F32 h, F32 text_x, F32 text_y, S32 ownerDraw, S32 ownerDrawFlags, S32 align, S32 textalign, S32 textvalign, F32 special, F32 scale, vec4_t foreColor, vec4_t backColor, qhandle_t shader, S32 textStyle )
{
    rectDef_t rect;
    
    if( cg_drawStatus.integer == 0 )
        return;
        
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    
    switch( ownerDraw )
    {
        case CG_PLAYER_CREDITS_VALUE:
            DrawPlayerCreditsValue( &rect, foreColor, true );
            break;
        case CG_PLAYER_CREDITS_VALUE_NOPAD:
            DrawPlayerCreditsValue( &rect, foreColor, false );
            break;
        case CG_PLAYER_STAMINA_1:
        case CG_PLAYER_STAMINA_2:
        case CG_PLAYER_STAMINA_3:
        case CG_PLAYER_STAMINA_4:
            DrawPlayerStamina( ownerDraw, &rect, backColor, foreColor, shader );
            break;
        case CG_PLAYER_STAMINA_BOLT:
            DrawPlayerStaminaBolt( &rect, backColor, foreColor, shader );
            break;
        case CG_PLAYER_AMMO_VALUE:
            DrawPlayerAmmoValue( &rect, foreColor );
            break;
        case CG_PLAYER_AMMO_STACK:
            DrawPlayerAmmoStack( &rect, backColor, foreColor, textalign,
                                 textvalign );
            break;
        case CG_PLAYER_CLIPS_VALUE:
            DrawPlayerClipsValue( &rect, foreColor );
            break;
        case CG_PLAYER_CLIPS_STACK:
            DrawPlayerClipsStack( &rect, backColor, foreColor, textalign,
                                  textvalign );
            break;
        case CG_PLAYER_BUILD_TIMER:
            DrawPlayerBuildTimer( &rect, foreColor );
            break;
        case CG_PLAYER_HEALTH:
            DrawPlayerHealthValue( &rect, foreColor );
            break;
        case CG_PLAYER_HEALTH_CROSS:
            DrawPlayerHealthCross( &rect, foreColor );
            break;
        case CG_PLAYER_CHARGE_BAR_BG:
            DrawPlayerChargeBarBG( &rect, foreColor, shader );
            break;
        case CG_PLAYER_CHARGE_BAR:
            DrawPlayerChargeBar( &rect, foreColor, shader );
            break;
        case CG_PLAYER_CLIPS_RING:
            DrawPlayerClipsRing( &rect, backColor, foreColor, shader );
            break;
        case CG_PLAYER_BUILD_TIMER_RING:
            DrawPlayerBuildTimerRing( &rect, backColor, foreColor, shader );
            break;
        case CG_PLAYER_WALLCLIMBING:
            DrawPlayerWallclimbing( &rect, backColor, foreColor, shader );
            break;
        case CG_PLAYER_BOOSTED:
            DrawPlayerBoosted( &rect, backColor, foreColor, shader );
            break;
        case CG_PLAYER_BOOST_BOLT:
            DrawPlayerBoosterBolt( &rect, backColor, foreColor, shader );
            break;
        case CG_PLAYER_POISON_BARBS:
            DrawPlayerPoisonBarbs( &rect, foreColor, shader );
            break;
        case CG_PLAYER_ALIEN_SENSE:
            DrawAlienSense( &rect );
            break;
        case CG_PLAYER_HUMAN_SCANNER:
            DrawHumanScanner( &rect, shader, foreColor );
            break;
        case CG_PLAYER_USABLE_BUILDABLE:
            DrawUsableBuildable( &rect, shader, foreColor );
            break;
        case CG_KILLER:
            DrawKiller( &rect, scale, foreColor, shader, textStyle );
            break;
        case CG_PLAYER_SELECT:
            DrawItemSelect( &rect, foreColor );
            break;
        case CG_PLAYER_WEAPONICON:
            DrawWeaponIcon( &rect, foreColor );
            break;
        case CG_PLAYER_ATTACK_FEEDBACK:
            DrawAttackFeedback( &rect );
            break;
        case CG_PLAYER_SELECTTEXT:
            DrawItemSelectText( &rect, scale, textStyle );
            break;
        case CG_SPECTATORS:
            DrawTeamSpectators( &rect, scale, textvalign, foreColor, shader );
            break;
        case CG_PLAYER_LOCATION:
            DrawLocation( &rect, scale, textalign, foreColor );
            break;
        case CG_PLAYER_CROSSHAIRNAMES:
            DrawCrosshairNames( &rect, scale, textStyle );
            break;
        case CG_PLAYER_CROSSHAIR:
            DrawCrosshair( &rect, foreColor );
            break;
        case CG_STAGE_REPORT_TEXT:
            DrawStageReport( &rect, text_x, text_y, foreColor, scale, textalign, textvalign, textStyle );
            break;
        case CG_ALIENS_SCORE_LABEL:
            DrawTeamLabel( &rect, TEAM_ALIENS, text_x, text_y, foreColor, scale, textalign, textvalign, textStyle );
            break;
        case CG_HUMANS_SCORE_LABEL:
            DrawTeamLabel( &rect, TEAM_HUMANS, text_x, text_y, foreColor, scale, textalign, textvalign, textStyle );
            break;
        case CG_SQUAD_MARKERS:
            DrawSquadMarkers( foreColor );
            break;
            
            //loading screen
        case CG_LOAD_LEVELSHOT:
            DrawLevelShot( &rect );
            break;
        case CG_LOAD_MEDIA:
            DrawMediaProgress( &rect, foreColor, scale, align, textalign, textStyle, special );
            break;
        case CG_LOAD_MEDIA_LABEL:
            DrawMediaProgressLabel( &rect, text_x, text_y, foreColor, scale, textalign, textvalign );
            break;
        case CG_LOAD_BUILDABLES:
            DrawBuildablesProgress( &rect, foreColor, scale, align, textalign, textStyle, special );
            break;
        case CG_LOAD_BUILDABLES_LABEL:
            DrawBuildablesProgressLabel( &rect, text_x, text_y, foreColor, scale, textalign, textvalign );
            break;
        case CG_LOAD_CHARMODEL:
            DrawCharModelProgress( &rect, foreColor, scale, align, textalign, textStyle, special );
            break;
        case CG_LOAD_CHARMODEL_LABEL:
            DrawCharModelProgressLabel( &rect, text_x, text_y, foreColor, scale, textalign, textvalign );
            break;
        case CG_LOAD_OVERALL:
            DrawOverallProgress( &rect, foreColor, scale, align, textalign, textStyle, special );
            break;
        case CG_LOAD_LEVELNAME:
            DrawLevelName( &rect, text_x, text_y, foreColor, scale, textalign, textvalign, textStyle );
            break;
        case CG_LOAD_MOTD:
            DrawMOTD( &rect, text_x, text_y, foreColor, scale, textalign, textvalign, textStyle );
            break;
        case CG_LOAD_HOSTNAME:
            DrawHostname( &rect, text_x, text_y, foreColor, scale, textalign, textvalign, textStyle );
            break;
            
        case CG_FPS:
            DrawFPS( &rect, text_x, text_y, scale, foreColor, textalign, textvalign, textStyle, true );
            break;
        case CG_FPS_FIXED:
            DrawFPS( &rect, text_x, text_y, scale, foreColor, textalign, textvalign, textStyle, false );
            break;
        case CG_TIMER:
            DrawTimer( &rect, text_x, text_y, scale, foreColor, textalign, textvalign, textStyle );
            break;
        case CG_CLOCK:
            DrawClock( &rect, text_x, text_y, scale, foreColor, textalign, textvalign, textStyle );
            break;
        case CG_SPEED:
            DrawSpeed( &rect, text_x, text_y, scale, foreColor, textalign, textvalign, textStyle );
            break;
        case CG_TIMER_MINS:
            DrawTimerMins( &rect, foreColor );
            break;
        case CG_TIMER_SECS:
            DrawTimerSecs( &rect, foreColor );
            break;
        case CG_SNAPSHOT:
            DrawSnapshot( &rect, text_x, text_y, scale, foreColor, textalign, textvalign, textStyle );
            break;
        case CG_LAGOMETER:
            DrawLagometer( &rect, text_x, text_y, scale, foreColor );
            break;
            
        case CG_DEMO_PLAYBACK:
            DrawDemoPlayback( &rect, foreColor, shader );
            break;
        case CG_DEMO_RECORDING:
            DrawDemoRecording( &rect, foreColor, shader );
            break;
            
        case CG_CONSOLE:
            DrawConsole( &rect, text_x, text_y, foreColor, scale, textalign, textvalign, textStyle );
            break;
            
        case CG_TUTORIAL:
            DrawTutorial( &rect, text_x, text_y, foreColor, scale, textalign, textvalign, textStyle );
            break;
            
        default:
            break;
    }
}

void idCGameLocal::MouseEvent( S32 x, S32 y )
{
    S32 n;
    
    //Dushan - stupid so much
    cgDC.cursorx = cgs.cursorX;
    cgDC.cursory = cgs.cursorY;
    
    
    if( ( cg.predictedPlayerState.pm_type == PM_NORMAL ||
            cg.predictedPlayerState.pm_type == PM_SPECTATOR ) &&
            cg.showScores == false )
    {
        trap_Key_SetCatcher( 0 );
        return;
    }
    
    cgs.cursorX += ( x * cgDC.aspectScale );
    if( cgs.cursorX < 0 )
        cgs.cursorX = 0;
    else if( cgs.cursorX > SCREEN_WIDTH )
        cgs.cursorX = SCREEN_WIDTH;
        
    cgs.cursorY += y;
    if( cgs.cursorY < 0 )
        cgs.cursorY = 0;
    else if( cgs.cursorY > SCREEN_HEIGHT )
        cgs.cursorY = SCREEN_HEIGHT;
        
    n = Display_CursorType( cgs.cursorX, cgs.cursorY );
    cgs.activeCursor = 0;
    if( n == CURSOR_ARROW )
        cgs.activeCursor = cgs.media.selectCursor;
    else if( n == CURSOR_SIZER )
        cgs.activeCursor = cgs.media.sizeCursor;
        
    cgDC.cursordx = x;
    cgDC.cursordy = y;
    cgDC.cursorx = cgs.cursorX;
    cgDC.cursory = cgs.cursorY;
    
    if( cgs.capturedItem )
        Display_MouseMove( cgs.capturedItem, x, y );
    else
        Display_MouseMove( NULL, cgs.cursorX, cgs.cursorY );
}

/*
==================
HideTeamMenus
==================
*/
void idCGameLocal::HideTeamMenu( void )
{
    Menus_CloseByName( "teamMenu" );
    Menus_CloseByName( "getMenu" );
}

/*
==================
ShowTeamMenus
==================
*/
void idCGameLocal::ShowTeamMenu( void )
{
    Menus_ActivateByName( "teamMenu" );
}

/*
==================
EventHandling
==================
 type 0 - no event handling
      1 - team menu
      2 - hud editor
*/
void idCGameLocal::EventHandling( S32 type, bool fForced )
{
    cgs.eventHandling = ( bool )type;
    
    if( type == CGAME_EVENT_NONE )
        HideTeamMenu( );
}

void idCGameLocal::KeyEvent( S32 key, bool down )
{
    if( !down )
        return;
        
    if( cg.predictedPlayerState.pm_type == PM_NORMAL ||
            ( cg.predictedPlayerState.pm_type == PM_SPECTATOR &&
              cg.showScores == false ) )
    {
        cgameLocal.EventHandling( CGAME_EVENT_NONE, true );
        trap_Key_SetCatcher( 0 );
        return;
    }
    
    Display_HandleKey( key, down, cgs.cursorX, cgs.cursorY );
    
    if( cgs.capturedItem )
        cgs.capturedItem = NULL;
    else
    {
        if( key == K_MOUSE2 && down )
            cgs.capturedItem = Display_CaptureItem( cgs.cursorX, cgs.cursorY );
    }
}

S32 idCGameLocal::ClientNumFromName( StringEntry p )
{
    S32 i;
    
    for( i = 0; i < cgs.maxclients; i++ )
    {
        if( cgs.clientinfo[ i ].infoValid &&
                Q_stricmp( cgs.clientinfo[ i ].name, p ) == 0 )
            return i;
    }
    
    return -1;
}

void idCGameLocal::RunMenuScript( UTF8** args )
{
}

/*
================
DrawLighting
================
*/
void idCGameLocal::DrawLighting( void )
{
    centity_t*   cent;
    
    cent = &cg_entities[ cg.snap->ps.clientNum ];
    
    //fade to black if stamina is low
    if( ( cg.snap->ps.stats[ STAT_STAMINA ] < -800 ) &&
            ( cg.snap->ps.stats[ STAT_TEAM ] == TEAM_HUMANS ) )
    {
        vec4_t black = { 0, 0, 0, 0 };
        black[ 3 ] = 1.0 - ( ( F32 )( cg.snap->ps.stats[ STAT_STAMINA ] + 1000 ) / 200.0f );
        trap_R_SetColor( black );
        DrawPic( 0, 0, 640, 480, cgs.media.whiteShader );
        trap_R_SetColor( NULL );
    }
}

/*
===============================================================================

CENTER PRINTING

===============================================================================
*/


/*
==============
CenterPrint

Called for important messages that should stay in the center of the screen
for a few moments
==============
*/
void idCGameLocal::CenterPrint( StringEntry str, S32 y, S32 charWidth )
{
    UTF8*  s;
    
    Q_strncpyz( cg.centerPrint, str, sizeof( cg.centerPrint ) );
    
    cg.centerPrintTime = cg.time;
    cg.centerPrintY = y;
    cg.centerPrintCharWidth = charWidth;
    
    // count the number of lines for centering
    cg.centerPrintLines = 1;
    s = cg.centerPrint;
    while( *s )
    {
        if( *s == '\n' )
            cg.centerPrintLines++;
            
        s++;
    }
}


/*
===================
DrawCenterString
===================
*/
void idCGameLocal::DrawCenterString( void )
{
    UTF8*  start;
    S32   l;
    S32   x, y, w;
    S32 h;
    F32* color;
    
    if( !cg.centerPrintTime )
        return;
        
    color = FadeColor( cg.centerPrintTime, 1000 * cg_centertime.value );
    if( !color )
        return;
        
    trap_R_SetColor( color );
    
    start = cg.centerPrint;
    
    y = cg.centerPrintY - cg.centerPrintLines * BIGCHAR_HEIGHT / 2;
    
    while( 1 )
    {
        UTF8 linebuffer[ 1024 ];
        
        for( l = 0; l < 50; l++ )
        {
            if( !start[ l ] || start[ l ] == '\n' )
                break;
                
            linebuffer[ l ] = start[ l ];
        }
        
        linebuffer[ l ] = 0;
        
        w = UI_Text_Width( linebuffer, 0.5, 0 );
        h = UI_Text_Height( linebuffer, 0.5, 0 );
        x = ( SCREEN_WIDTH - w ) / 2;
        UI_Text_Paint( x, y + h, 0.5, color, linebuffer, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE );
        y += h + 6;
        
        while( *start && ( *start != '\n' ) )
            start++;
            
        if( !*start )
            break;
            
        start++;
    }
    
    trap_R_SetColor( NULL );
}

//==============================================================================

//FIXME: both vote notes are hardcoded, change to ownerdrawn?

/*
=================
DrawVote
=================
*/
void idCGameLocal::DrawVote( void )
{
    UTF8*    s;
    S32     sec;
    vec4_t  white = { 1.0f, 1.0f, 1.0f, 1.0f };
    UTF8    yeskey[ 32 ], nokey[ 32 ];
    
    if( !cgs.voteTime )
        return;
        
    // play a talk beep whenever it is modified
    if( cgs.voteModified )
    {
        cgs.voteModified = false;
        trap_S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );
    }
    
    sec = ( VOTE_TIME - ( cg.time - cgs.voteTime ) ) / 1000;
    
    if( sec < 0 )
        sec = 0;
    Q_strncpyz( yeskey, KeyBinding( "vote yes" ), sizeof( yeskey ) );
    Q_strncpyz( nokey, KeyBinding( "vote no" ), sizeof( nokey ) );
    s = va( "VOTE(%i): \"%s\"  [%s]Yes:%i [%s]No:%i", sec, cgs.voteString, yeskey, cgs.voteYes, nokey, cgs.voteNo );
    UI_Text_Paint( 8, 340, 0.3f, white, s, 0, 0, ITEM_TEXTSTYLE_NORMAL );
}

/*
=================
DrawTeamVote
=================
*/
void idCGameLocal::DrawTeamVote( void )
{
    UTF8*    s;
    S32     sec, cs_offset;
    vec4_t  white = { 1.0f, 1.0f, 1.0f, 1.0f };
    UTF8    yeskey[ 32 ], nokey[ 32 ];
    
    if( cg.predictedPlayerState.stats[ STAT_TEAM ] == TEAM_HUMANS )
        cs_offset = 0;
    else if( cg.predictedPlayerState.stats[ STAT_TEAM ] == TEAM_ALIENS )
        cs_offset = 1;
    else
        return;
        
    if( !cgs.teamVoteTime[ cs_offset ] )
        return;
        
    // play a talk beep whenever it is modified
    if( cgs.teamVoteModified[ cs_offset ] )
    {
        cgs.teamVoteModified[ cs_offset ] = false;
        trap_S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );
    }
    
    sec = ( VOTE_TIME - ( cg.time - cgs.teamVoteTime[ cs_offset ] ) ) / 1000;
    
    if( sec < 0 )
        sec = 0;
        
    Q_strncpyz( yeskey, KeyBinding( "teamvote yes" ), sizeof( yeskey ) );
    Q_strncpyz( nokey, KeyBinding( "teamvote no" ), sizeof( nokey ) );
    s = va( "TEAMVOTE(%i): \"%s\"  [%s]Yes:%i   [%s]No:%i", sec,
            cgs.teamVoteString[ cs_offset ],
            yeskey, cgs.teamVoteYes[cs_offset],
            nokey, cgs.teamVoteNo[ cs_offset ] );
            
    UI_Text_Paint( 8, 360, 0.3f, white, s, 0, 0, ITEM_TEXTSTYLE_NORMAL );
}


bool idCGameLocal::DrawScoreboard( void )
{
    static bool firstTime = true;
    F32 fade, *fadeColor;
    
    if( menuScoreboard )
        menuScoreboard->window.flags &= ~WINDOW_FORCED;
        
    if( cg_paused.integer )
    {
        cg.deferredPlayerLoading = 0;
        firstTime = true;
        return false;
    }
    
    if( cg.showScores ||
            cg.predictedPlayerState.pm_type == PM_INTERMISSION )
    {
        fade = 1.0;
        fadeColor = colorWhite;
    }
    else
    {
        cg.deferredPlayerLoading = 0;
        cg.killerName[ 0 ] = 0;
        firstTime = true;
        return false;
    }
    
    
    if( menuScoreboard == NULL )
        menuScoreboard = Menus_FindByName( "teamscore_menu" );
        
    if( menuScoreboard )
    {
        if( firstTime )
        {
            SetScoreSelection( menuScoreboard );
            firstTime = false;
        }
        
        Menu_Paint( menuScoreboard, true );
    }
    
    return true;
}

/*
=================
DrawIntermission
=================
*/
void idCGameLocal::DrawIntermission( void )
{
    if( cg_drawStatus.integer )
        Menu_Paint( Menus_FindByName( "default_hud" ), true );
        
    cg.scoreFadeTime = cg.time;
    cg.scoreBoardShowing = DrawScoreboard( );
}

#define FOLLOWING_STRING "Following: "
#define CHASING_STRING "Chasing: "

/*
=================
DrawFollow
=================
*/
bool idCGameLocal::DrawFollow( void )
{
    F32       w;
    vec4_t      color;
    UTF8        buffer[ MAX_STRING_CHARS ];
    
    if( cg.snap->ps.clientNum == cg.clientNum )
        return false;
        
    color[ 0 ] = 1;
    color[ 1 ] = 1;
    color[ 2 ] = 1;
    color[ 3 ] = 1;
    
    if( !cg.chaseFollow )
        strcpy( buffer, FOLLOWING_STRING );
    else
        strcpy( buffer, CHASING_STRING );
    strcat( buffer, cgs.clientinfo[ cg.snap->ps.clientNum ].name );
    
    w = UI_Text_Width( buffer, 0.7f, 0 );
    UI_Text_Paint( 320 - w / 2, 400, 0.7f, color, buffer, 0, 0, ITEM_TEXTSTYLE_SHADOWED );
    
    return true;
}

/*
=================
DrawQueue
=================
*/
bool idCGameLocal::DrawQueue( void )
{
    F32       w;
    vec4_t      color;
    S32         position, remainder;
    UTF8*        ordinal, buffer[ MAX_STRING_CHARS ];
    
    if( !( cg.snap->ps.pm_flags & PMF_QUEUED ) )
        return false;
        
    color[ 0 ] = 1;
    color[ 1 ] = 1;
    color[ 2 ] = 1;
    color[ 3 ] = 1;
    
    position = cg.snap->ps.persistant[ PERS_QUEUEPOS ] + 1;
    if( position < 1 )
        return false;
    remainder = position % 10;
    ordinal = "th";
    if( remainder == 1 )
        ordinal = "st";
    else if( remainder == 2 )
        ordinal = "nd";
    else if( remainder == 3 )
        ordinal = "rd";
    Com_sprintf( buffer, MAX_STRING_CHARS, "You are %d%s in the spawn queue",
                 position, ordinal );
                 
    w = UI_Text_Width( buffer, 0.7f, 0 );
    UI_Text_Paint( 320 - w / 2, 360, 0.7f, color, buffer, 0, 0, ITEM_TEXTSTYLE_SHADOWED );
    
    if( cg.snap->ps.persistant[ PERS_SPAWNS ] == 0 )
        Com_sprintf( buffer, MAX_STRING_CHARS, "There are no spawns remaining" );
    else if( cg.snap->ps.persistant[ PERS_SPAWNS ] == 1 )
        Com_sprintf( buffer, MAX_STRING_CHARS, "There is 1 spawn remaining" );
    else
        Com_sprintf( buffer, MAX_STRING_CHARS, "There are %d spawns remaining",
                     cg.snap->ps.persistant[ PERS_SPAWNS ] );
                     
    w = UI_Text_Width( buffer, 0.7f, 0 );
    UI_Text_Paint( 320 - w / 2, 400, 0.7f, color, buffer, 0, 0, ITEM_TEXTSTYLE_SHADOWED );
    
    return true;
}


void idCGameLocal::DrawBotInfo( void )
{
    UTF8  buffer[ MAX_STRING_CHARS ];
    F32 w;
    vec4_t      color = { 1, 1, 1, 1 };
    F32 scale = 0.4f;
    StringEntry  info;
    UTF8	key[MAX_INFO_KEY];
    UTF8	value[MAX_INFO_VALUE];
    S32 y;
    S32 clientnum;
    
    /*Com_sprintf( buffer, MAX_STRING_CHARS, "BotInfo Test.");
    w = UI_Text_Width( buffer, scale, 0 );
    Text_Paint( 640 - w , 160, scale, color, buffer, 0, 0, ITEM_TEXTSTYLE_NORMAL );
    */
    
    // we dont spec? so give info for the crosshairplayer
    if( !cgs.clientinfo[ cg.snap->ps.clientNum ].botSkill )
    {
        clientnum = cgameLocal.CrosshairPlayer();
    }
    else	// info for the bot we spec
        clientnum = cg.snap->ps.clientNum;
        
    info = ConfigString( CS_BOTINFOS + clientnum );
    
    y = 160;
    while( 1 )
    {
        Info_NextPair( &info, key, value );
        if( !key[0] ) break;
        if( !value[0] ) break;
        
        Com_sprintf( buffer, MAX_STRING_CHARS, va( "%s: %s", key, value ) );
        y += UI_Text_Height( buffer, scale, 0 ) + 5;
        w = UI_Text_Width( buffer, scale, 0 );
        UI_Text_Paint( 630 - w , y, scale, color, buffer, 0, 0, ITEM_TEXTSTYLE_NORMAL );
    }
    
}

//==================================================================================

#define SPECTATOR_STRING "SPECTATOR"
/*
=================
Draw2D
=================
*/
void idCGameLocal::Draw2D( void )
{
    vec4_t    color;
    F32     w;
    menuDef_t* menu = NULL, *defaultMenu;
    
    color[ 0 ] = color[ 1 ] = color[ 2 ] = color[ 3 ] = 1.0f;
    
    // if we are taking a levelshot for the menu, don't draw anything
    if( cg.levelShot )
        return;
        
    if( cg_draw2D.integer == 0 )
        return;
        
    if( cg.snap->ps.pm_type == PM_INTERMISSION )
    {
        DrawIntermission( );
        return;
    }
    
    DrawLighting( );
    
    
    defaultMenu = Menus_FindByName( "default_hud" );
    
    if( cg.snap->ps.persistant[ PERS_SPECSTATE ] != SPECTATOR_NOT )
    {
        w = UI_Text_Width( SPECTATOR_STRING, 0.7f, 0 );
        UI_Text_Paint( 320 - w / 2, 440, 0.7f, color, SPECTATOR_STRING, 0, 0, ITEM_TEXTSTYLE_SHADOWED );
    }
    else
        menu = Menus_FindByName( bggame->ClassConfig( ( class_t )cg.predictedPlayerState.stats[ STAT_CLASS ] )->hudName );
        
    if( menu && !( cg.snap->ps.stats[ STAT_STATE ] & SS_HOVELING ) &&
            ( cg.snap->ps.stats[ STAT_HEALTH ] > 0 ) )
    {
        DrawBuildableStatus( );
        DrawTeamStatus( );
        if( cg_drawStatus.integer )
            Menu_Paint( menu, true );
            
    }
    else if( cg_drawStatus.integer )
        Menu_Paint( defaultMenu, true );
        
    DrawVote( );
    DrawTeamVote( );
    DrawFollow( );
    DrawQueue( );
    DrawBotInfo( );
    
    // don't draw center string if scoreboard is up
    cg.scoreBoardShowing = DrawScoreboard( );
    
    if( !cg.scoreBoardShowing )
        DrawCenterString( );
}

/*
===============
ScalePainBlendTCs
===============
*/
void idCGameLocal::ScalePainBlendTCs( F32* s1, F32* t1, F32* s2, F32* t2 )
{
    *s1 -= 0.5f;
    *t1 -= 0.5f;
    *s2 -= 0.5f;
    *t2 -= 0.5f;
    
    *s1 *= cg_painBlendZoom.value;
    *t1 *= cg_painBlendZoom.value;
    *s2 *= cg_painBlendZoom.value;
    *t2 *= cg_painBlendZoom.value;
    
    *s1 += 0.5f;
    *t1 += 0.5f;
    *s2 += 0.5f;
    *t2 += 0.5f;
}

#define PAINBLEND_BORDER    0.15f

/*
===============
PainBlend
===============
*/
void idCGameLocal::PainBlend( void )
{
    vec4_t      color;
    S32         damage;
    F32       damageAsFracOfMax;
    qhandle_t   shader = cgs.media.viewBloodShader;
    F32       x, y, w, h;
    F32       s1, t1, s2, t2;
    
    if( cg.snap->ps.persistant[ PERS_SPECSTATE ] != SPECTATOR_NOT || cg.intermissionStarted )
        return;
        
    damage = cg.lastHealth - cg.snap->ps.stats[ STAT_HEALTH ];
    
    if( damage < 0 )
        damage = 0;
        
    damageAsFracOfMax = ( F32 )damage / cg.snap->ps.stats[ STAT_MAX_HEALTH ];
    cg.lastHealth = cg.snap->ps.stats[ STAT_HEALTH ];
    
    cg.painBlendValue += damageAsFracOfMax * cg_painBlendScale.value;
    
    if( cg.painBlendValue > 0.0f )
    {
        cg.painBlendValue -= ( cg.frametime / 1000.0f ) *
                             cg_painBlendDownRate.value;
    }
    
    if( cg.painBlendValue > 1.0f )
        cg.painBlendValue = 1.0f;
    else if( cg.painBlendValue <= 0.0f )
    {
        cg.painBlendValue = 0.0f;
        return;
    }
    
    if( cg.snap->ps.stats[ STAT_TEAM ] == TEAM_ALIENS )
        VectorSet( color, 0.43f, 0.8f, 0.37f );
    else if( cg.snap->ps.stats[ STAT_TEAM ] == TEAM_HUMANS )
        VectorSet( color, 0.8f, 0.0f, 0.0f );
        
    if( cg.painBlendValue > cg.painBlendTarget )
    {
        cg.painBlendTarget += ( cg.frametime / 1000.0f ) *
                              cg_painBlendUpRate.value;
    }
    else if( cg.painBlendValue < cg.painBlendTarget )
        cg.painBlendTarget = cg.painBlendValue;
        
    if( cg.painBlendTarget > cg_painBlendMax.value )
        cg.painBlendTarget = cg_painBlendMax.value;
        
    color[ 3 ] = cg.painBlendTarget;
    
    trap_R_SetColor( color );
    
    //left
    x = 0.0f;
    y = 0.0f;
    w = PAINBLEND_BORDER * 640.0f;
    h = 480.0f;
    AdjustFrom640( &x, &y, &w, &h );
    s1 = 0.0f;
    t1 = 0.0f;
    s2 = PAINBLEND_BORDER;
    t2 = 1.0f;
    ScalePainBlendTCs( &s1, &t1, &s2, &t2 );
    trap_R_DrawStretchPic( x, y, w, h, s1, t1, s2, t2, shader );
    
    //right
    x = 640.0f - ( PAINBLEND_BORDER * 640.0f );
    y = 0.0f;
    w = PAINBLEND_BORDER * 640.0f;
    h = 480.0f;
    AdjustFrom640( &x, &y, &w, &h );
    s1 = 1.0f - PAINBLEND_BORDER;
    t1 = 0.0f;
    s2 = 1.0f;
    t2 = 1.0f;
    ScalePainBlendTCs( &s1, &t1, &s2, &t2 );
    trap_R_DrawStretchPic( x, y, w, h, s1, t1, s2, t2, shader );
    
    //top
    x = PAINBLEND_BORDER * 640.0f;
    y = 0.0f;
    w = 640.0f - ( 2 * PAINBLEND_BORDER * 640.0f );
    h = PAINBLEND_BORDER * 480.0f;
    AdjustFrom640( &x, &y, &w, &h );
    s1 = PAINBLEND_BORDER;
    t1 = 0.0f;
    s2 = 1.0f - PAINBLEND_BORDER;
    t2 = PAINBLEND_BORDER;
    ScalePainBlendTCs( &s1, &t1, &s2, &t2 );
    trap_R_DrawStretchPic( x, y, w, h, s1, t1, s2, t2, shader );
    
    //bottom
    x = PAINBLEND_BORDER * 640.0f;
    y = 480.0f - ( PAINBLEND_BORDER * 480.0f );
    w = 640.0f - ( 2 * PAINBLEND_BORDER * 640.0f );
    h = PAINBLEND_BORDER * 480.0f;
    AdjustFrom640( &x, &y, &w, &h );
    s1 = PAINBLEND_BORDER;
    t1 = 1.0f - PAINBLEND_BORDER;
    s2 = 1.0f - PAINBLEND_BORDER;
    t2 = 1.0f;
    ScalePainBlendTCs( &s1, &t1, &s2, &t2 );
    trap_R_DrawStretchPic( x, y, w, h, s1, t1, s2, t2, shader );
    
    trap_R_SetColor( NULL );
}

/*
=====================
ResetPainBlend
=====================
*/
void idCGameLocal::ResetPainBlend( void )
{
    cg.painBlendValue = 0.0f;
    cg.painBlendTarget = 0.0f;
    cg.lastHealth = cg.snap->ps.stats[ STAT_HEALTH ];
}

/*
=====================
DrawActive

Perform all drawing needed to completely fill the screen
=====================
*/
void idCGameLocal::DrawActive( stereoFrame_t stereoView )
{
    F32   separation;
    vec3_t    baseOrg;
    
    // optionally draw the info screen instead
    if( !cg.snap )
        return;
        
    switch( stereoView )
    {
        case STEREO_CENTER:
            separation = 0;
            break;
        case STEREO_LEFT:
            separation = -cg_stereoSeparation.value / 2;
            break;
        case STEREO_RIGHT:
            separation = cg_stereoSeparation.value / 2;
            break;
        default:
            separation = 0;
            Error( "idCGameLocal::DrawActive: Undefined stereoView" );
    }
    
    // clear around the rendered view if sized down
    TileClear( );
    
    // offset vieworg appropriately if we're doing stereo separation
    VectorCopy( cg.refdef.vieworg, baseOrg );
    
    if( separation != 0 )
        VectorMA( cg.refdef.vieworg, -separation, cg.refdef.viewaxis[ 1 ],
                  cg.refdef.vieworg );
                  
    // draw 3D view
    trap_R_RenderScene( &cg.refdef );
    
    // restore original viewpoint if running stereo
    if( separation != 0 )
        VectorCopy( baseOrg, cg.refdef.vieworg );
        
    // first person blend blobs, done after AnglesToAxis
    if( !cg.renderingThirdPerson )
        PainBlend( );
        
    // draw status bar and other floating elements
    Draw2D( );
}


