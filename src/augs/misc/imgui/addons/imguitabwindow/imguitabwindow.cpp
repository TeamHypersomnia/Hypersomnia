//- Common Code For All Addons needed just to ease inclusion as separate files in user code ----------------------
#include <imgui.h>
#undef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>
//-----------------------------------------------------------------------------------------------------------------

#include "imguitabwindow.h"


// TODO: Clean this code, it's a mess!

#if !defined(alloca)
#   ifdef _WIN32
#       include <malloc.h>     // alloca
#       if !defined(alloca)
#           define alloca _alloca  // for clang with MS Codegen
#       endif //alloca
#   elif defined(__GLIBC__) || defined(__sun)
#       include <alloca.h>     // alloca
#   else
#       include <stdlib.h>     // alloca
#   endif
#endif //alloca


namespace ImGui {

namespace DrawListHelper {

// Two main additions:
// 1) PathFillAndStroke in the same method (so that we don't have to build the path twice)
// 2) VerticalGradient: looks good
inline static void GetVerticalGradientTopAndBottomColors(ImU32 c,float fillColorGradientDeltaIn0_05,ImU32& tc,ImU32& bc)  {
    if (fillColorGradientDeltaIn0_05==0) {tc=bc=c;return;}

#   define CACHE_LAST_GRADIENT // Maybe useless, but when displaying a lot of tabs we can save some calls...
#   ifdef CACHE_LAST_GRADIENT
    //static unsigned int dbg = 0;
    static ImU32 cacheColorIn=0;static float cacheGradientIn=0.f;static ImU32 cacheTopColorOut=0;static ImU32 cacheBottomColorOut=0;
    if (cacheColorIn==c && cacheGradientIn==fillColorGradientDeltaIn0_05)   {tc=cacheTopColorOut;bc=cacheBottomColorOut;
        //fprintf(stderr,"cached: %u\n",dbg++);
    return;}
    cacheColorIn=c;cacheGradientIn=fillColorGradientDeltaIn0_05;
#   endif //CACHE_LAST_GRADIENT

    const bool negative = (fillColorGradientDeltaIn0_05<0);
    if (negative) fillColorGradientDeltaIn0_05=-fillColorGradientDeltaIn0_05;
    if (fillColorGradientDeltaIn0_05>0.5f) fillColorGradientDeltaIn0_05=0.5f;

    // New code:
    //#define IM_COL32(R,G,B,A)    (((ImU32)(A)<<IM_COL32_A_SHIFT) | ((ImU32)(B)<<IM_COL32_B_SHIFT) | ((ImU32)(G)<<IM_COL32_G_SHIFT) | ((ImU32)(R)<<IM_COL32_R_SHIFT))
    const int fcgi = fillColorGradientDeltaIn0_05*255.0f;
    const int R = (unsigned char) (c>>IM_COL32_R_SHIFT);    // The cast should reset upper bits (as far as I hope)
    const int G = (unsigned char) (c>>IM_COL32_G_SHIFT);
    const int B = (unsigned char) (c>>IM_COL32_B_SHIFT);
    const int A = (unsigned char) (c>>IM_COL32_A_SHIFT);

    int r = R+fcgi, g = G+fcgi, b = B+fcgi;
    if (r>255) r=255;if (g>255) g=255;if (b>255) b=255;
    if (negative) bc = IM_COL32(r,g,b,A); else tc = IM_COL32(r,g,b,A);

    r = R-fcgi; g = G-fcgi; b = B-fcgi;
    if (r<0) r=0;if (g<0) g=0;if (b<0) b=0;
    if (negative) tc = IM_COL32(r,g,b,A); else bc = IM_COL32(r,g,b,A);

    // Old legacy code (to remove)... [However here we lerp alpha too...]
    /*// Can we do it without the double conversion ImU32 -> ImVec4 -> ImU32 ?
    const ImVec4 cf = ColorConvertU32ToFloat4(c);
    ImVec4 tmp(cf.x+fillColorGradientDeltaIn0_05,cf.y+fillColorGradientDeltaIn0_05,cf.z+fillColorGradientDeltaIn0_05,cf.w+fillColorGradientDeltaIn0_05);
    if (tmp.x>1.f) tmp.x=1.f;if (tmp.y>1.f) tmp.y=1.f;if (tmp.z>1.f) tmp.z=1.f;if (tmp.w>1.f) tmp.w=1.f;
    if (negative) bc = ColorConvertFloat4ToU32(tmp); else tc = ColorConvertFloat4ToU32(tmp);
    tmp=ImVec4(cf.x-fillColorGradientDeltaIn0_05,cf.y-fillColorGradientDeltaIn0_05,cf.z-fillColorGradientDeltaIn0_05,cf.w-fillColorGradientDeltaIn0_05);
    if (tmp.x<0.f) tmp.x=0.f;if (tmp.y<0.f) tmp.y=0.f;if (tmp.z<0.f) tmp.z=0.f;if (tmp.w<0.f) tmp.w=0.f;
    if (negative) tc = ColorConvertFloat4ToU32(tmp); else bc = ColorConvertFloat4ToU32(tmp);*/

#   ifdef CACHE_LAST_GRADIENT
    cacheTopColorOut=tc;cacheBottomColorOut=bc;
    //fprintf(stderr,"uncached: %u\n",dbg++);
#   undef CACHE_LAST_GRADIENT // Mandatory
#   endif //CACHE_LAST_GRADIENT
}
// Can we do it from ImU32 ct and cb, without conversion to ImVec4 ?
inline static ImU32 GetVerticalGradient(const ImVec4& ct,const ImVec4& cb,float DH,float H)    {
    IM_ASSERT(H!=0);
    const float fa = DH/H;
    const float fc = (1.f-fa);
    return ColorConvertFloat4ToU32(ImVec4(
        ct.x * fc + cb.x * fa,
        ct.y * fc + cb.y * fa,
        ct.z * fc + cb.z * fa,
        ct.w * fc + cb.w * fa)
    );
}
void ImDrawListAddConvexPolyFilledWithVerticalGradient(ImDrawList *dl, const ImVec2 *points, const int points_count, ImU32 colTop, ImU32 colBot,float miny=-1.f,float maxy=-1.f)
{
    if (!dl) return;
    if (colTop==colBot)  {
        dl->AddConvexPolyFilled(points,points_count,colTop);
        return;
    }
    const ImVec2 uv = GImGui->DrawListSharedData.TexUvWhitePixel;
    const bool anti_aliased = GImGui->Style.AntiAliasedFill;
    //if (ImGui::GetIO().KeyCtrl) anti_aliased = false; // Debug

    int height=0;
    if (miny<=0 || maxy<=0) {
        const float max_float = 999999999999999999.f;
        miny=max_float;maxy=-max_float;
        for (int i = 0; i < points_count; i++) {
            const float h = points[i].y;
            if (h < miny) miny = h;
            else if (h > maxy) maxy = h;
        }
    }
    height = maxy-miny;
    const ImVec4 colTopf = ColorConvertU32ToFloat4(colTop);
    const ImVec4 colBotf = ColorConvertU32ToFloat4(colBot);


    if (anti_aliased)
    {
        // Anti-aliased Fill
        const float AA_SIZE = 1.0f;

        const ImVec4 colTransTopf(colTopf.x,colTopf.y,colTopf.z,0.f);
        const ImVec4 colTransBotf(colBotf.x,colBotf.y,colBotf.z,0.f);
        const int idx_count = (points_count-2)*3 + points_count*6;
        const int vtx_count = (points_count*2);
        dl->PrimReserve(idx_count, vtx_count);

        // Add indexes for fill
        unsigned int vtx_inner_idx = dl->_VtxCurrentIdx;
        unsigned int vtx_outer_idx = dl->_VtxCurrentIdx+1;
        for (int i = 2; i < points_count; i++)
        {
            dl->_IdxWritePtr[0] = (ImDrawIdx)(vtx_inner_idx); dl->_IdxWritePtr[1] = (ImDrawIdx)(vtx_inner_idx+((i-1)<<1)); dl->_IdxWritePtr[2] = (ImDrawIdx)(vtx_inner_idx+(i<<1));
            dl->_IdxWritePtr += 3;
        }

        // Compute normals
        ImVec2* temp_normals = (ImVec2*)alloca(points_count * sizeof(ImVec2));
        for (int i0 = points_count-1, i1 = 0; i1 < points_count; i0 = i1++)
        {
            const ImVec2& p0 = points[i0];
            const ImVec2& p1 = points[i1];
            ImVec2 diff = p1 - p0;
            diff *= ImInvLength(diff, 1.0f);
            temp_normals[i0].x = diff.y;
            temp_normals[i0].y = -diff.x;
        }

        for (int i0 = points_count-1, i1 = 0; i1 < points_count; i0 = i1++)
        {
            // Average normals
            const ImVec2& n0 = temp_normals[i0];
            const ImVec2& n1 = temp_normals[i1];
            ImVec2 dm = (n0 + n1) * 0.5f;
            float dmr2 = dm.x*dm.x + dm.y*dm.y;
            if (dmr2 > 0.000001f)
            {
                float scale = 1.0f / dmr2;
                if (scale > 100.0f) scale = 100.0f;
                dm *= scale;
            }
            dm *= AA_SIZE * 0.5f;

            // Add vertices
            //_VtxWritePtr[0].pos = (points[i1] - dm); _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;        // Inner
            //_VtxWritePtr[1].pos = (points[i1] + dm); _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col_trans;  // Outer
            dl->_VtxWritePtr[0].pos = (points[i1] - dm); dl->_VtxWritePtr[0].uv = uv; dl->_VtxWritePtr[0].col = GetVerticalGradient(colTopf,colBotf,points[i1].y-miny,height);        // Inner
            dl->_VtxWritePtr[1].pos = (points[i1] + dm); dl->_VtxWritePtr[1].uv = uv; dl->_VtxWritePtr[1].col = GetVerticalGradient(colTransTopf,colTransBotf,points[i1].y-miny,height);  // Outer
            dl->_VtxWritePtr += 2;

            // Add indexes for fringes
            dl->_IdxWritePtr[0] = (ImDrawIdx)(vtx_inner_idx+(i1<<1)); dl->_IdxWritePtr[1] = (ImDrawIdx)(vtx_inner_idx+(i0<<1)); dl->_IdxWritePtr[2] = (ImDrawIdx)(vtx_outer_idx+(i0<<1));
            dl->_IdxWritePtr[3] = (ImDrawIdx)(vtx_outer_idx+(i0<<1)); dl->_IdxWritePtr[4] = (ImDrawIdx)(vtx_outer_idx+(i1<<1)); dl->_IdxWritePtr[5] = (ImDrawIdx)(vtx_inner_idx+(i1<<1));
            dl->_IdxWritePtr += 6;
        }
        dl->_VtxCurrentIdx += (ImDrawIdx)vtx_count;
    }
    else
    {
        // Non Anti-aliased Fill
        const int idx_count = (points_count-2)*3;
        const int vtx_count = points_count;
        dl->PrimReserve(idx_count, vtx_count);
        for (int i = 0; i < vtx_count; i++)
        {
            //_VtxWritePtr[0].pos = points[i]; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
            dl->_VtxWritePtr[0].pos = points[i]; dl->_VtxWritePtr[0].uv = uv; dl->_VtxWritePtr[0].col = GetVerticalGradient(colTopf,colBotf,points[i].y-miny,height);
            dl->_VtxWritePtr++;
        }
        for (int i = 2; i < points_count; i++)
        {
            dl->_IdxWritePtr[0] = (ImDrawIdx)(dl->_VtxCurrentIdx); dl->_IdxWritePtr[1] = (ImDrawIdx)(dl->_VtxCurrentIdx+i-1); dl->_IdxWritePtr[2] = (ImDrawIdx)(dl->_VtxCurrentIdx+i);
            dl->_IdxWritePtr += 3;
        }
        dl->_VtxCurrentIdx += (ImDrawIdx)vtx_count;
    }
}
void ImDrawListPathFillWithVerticalGradientAndStroke(ImDrawList *dl, const ImU32 &fillColorTop, const ImU32 &fillColorBottom, const ImU32 &strokeColor, bool strokeClosed, float strokeThickness,float miny,float maxy)    {
    if (!dl) return;
    if (fillColorTop==fillColorBottom) dl->AddConvexPolyFilled(dl->_Path.Data,dl->_Path.Size, fillColorTop);
    else if ((fillColorTop & IM_COL32_A_MASK) != 0 || (fillColorBottom & IM_COL32_A_MASK) != 0) ImDrawListAddConvexPolyFilledWithVerticalGradient(dl, dl->_Path.Data, dl->_Path.Size, fillColorTop, fillColorBottom,miny,maxy);
    if ((strokeColor& IM_COL32_A_MASK)!= 0 && strokeThickness>0) dl->AddPolyline(dl->_Path.Data, dl->_Path.Size, strokeColor, strokeClosed, strokeThickness);
    dl->PathClear();
}
void ImDrawListPathFillAndStroke(ImDrawList *dl, const ImU32 &fillColor, const ImU32 &strokeColor, bool strokeClosed, float strokeThickness)    {
    if (!dl) return;
    if ((fillColor & IM_COL32_A_MASK) != 0) dl->AddConvexPolyFilled(dl->_Path.Data, dl->_Path.Size, fillColor);
    if ((strokeColor& IM_COL32_A_MASK)!= 0 && strokeThickness>0) dl->AddPolyline(dl->_Path.Data, dl->_Path.Size, strokeColor, strokeClosed, strokeThickness);
    dl->PathClear();
}
void ImDrawListAddRect(ImDrawList *dl, const ImVec2 &a, const ImVec2 &b, const ImU32 &fillColor, const ImU32 &strokeColor, float rounding, int rounding_corners, float strokeThickness) {
    if (!dl || (((fillColor & IM_COL32_A_MASK) == 0) && ((strokeColor & IM_COL32_A_MASK) == 0)))  return;
    dl->PathRect(a, b, rounding, rounding_corners);
    ImDrawListPathFillAndStroke(dl,fillColor,strokeColor,true,strokeThickness);
}
void ImDrawListAddRectWithVerticalGradient(ImDrawList *dl, const ImVec2 &a, const ImVec2 &b, const ImU32 &fillColorTop, const ImU32 &fillColorBottom, const ImU32 &strokeColor, float rounding, int rounding_corners, float strokeThickness) {
    if (!dl || (((fillColorTop & IM_COL32_A_MASK) == 0) && ((fillColorBottom & IM_COL32_A_MASK) == 0) && ((strokeColor & IM_COL32_A_MASK) == 0)))  return;
    if (rounding==0.f || rounding_corners==0) {
        dl->AddRectFilledMultiColor(a,b,fillColorTop,fillColorTop,fillColorBottom,fillColorBottom); // Huge speedup!
        if ((strokeColor& IM_COL32_A_MASK)!= 0 && strokeThickness>0.f) {
            dl->PathRect(a, b, rounding, rounding_corners);
            dl->AddPolyline(dl->_Path.Data, dl->_Path.Size, strokeColor, true, strokeThickness);
            dl->PathClear();
        }
    }
    else    {
        dl->PathRect(a, b, rounding, rounding_corners);
        ImDrawListPathFillWithVerticalGradientAndStroke(dl,fillColorTop,fillColorBottom,strokeColor,true,strokeThickness,a.y,b.y);
    }
}
void ImDrawListAddRectWithVerticalGradient(ImDrawList *dl, const ImVec2 &a, const ImVec2 &b, const ImU32 &fillColor, float fillColorGradientDeltaIn0_05, const ImU32 &strokeColor, float rounding, int rounding_corners, float strokeThickness)
{
    ImU32 fillColorTop,fillColorBottom;GetVerticalGradientTopAndBottomColors(fillColor,fillColorGradientDeltaIn0_05,fillColorTop,fillColorBottom);
    ImDrawListAddRectWithVerticalGradient(dl,a,b,fillColorTop,fillColorBottom,strokeColor,rounding,rounding_corners,strokeThickness);
}

void ImDrawListAddConvexPolyFilledWithHorizontalGradient(ImDrawList *dl, const ImVec2 *points, const int points_count, ImU32 colLeft, ImU32 colRight, float minx, float maxx)
{
    if (!dl) return;
    if (colLeft==colRight)  {
        dl->AddConvexPolyFilled(points,points_count,colLeft);
        return;
    }
    const ImVec2 uv = GImGui->DrawListSharedData.TexUvWhitePixel;
    const bool anti_aliased = GImGui->Style.AntiAliasedFill;
    //if (ImGui::GetIO().KeyCtrl) anti_aliased = false; // Debug

    int width=0;
    if (minx<=0 || maxx<=0) {
        const float max_float = 999999999999999999.f;
        minx=max_float;maxx=-max_float;
        for (int i = 0; i < points_count; i++) {
            const float w = points[i].x;
            if (w < minx) minx = w;
            else if (w > maxx) maxx = w;
        }
    }
    width = maxx-minx;
    const ImVec4 colLeftf  = ColorConvertU32ToFloat4(colLeft);
    const ImVec4 colRightf = ColorConvertU32ToFloat4(colRight);


    if (anti_aliased)
    {
        // Anti-aliased Fill
        const float AA_SIZE = 1.0f;

        const ImVec4 colTransLeftf(colLeftf.x,colLeftf.y,colLeftf.z,0.f);
        const ImVec4 colTransRightf(colRightf.x,colRightf.y,colRightf.z,0.f);
        const int idx_count = (points_count-2)*3 + points_count*6;
        const int vtx_count = (points_count*2);
        dl->PrimReserve(idx_count, vtx_count);

        // Add indexes for fill
        unsigned int vtx_inner_idx = dl->_VtxCurrentIdx;
        unsigned int vtx_outer_idx = dl->_VtxCurrentIdx+1;
        for (int i = 2; i < points_count; i++)
        {
            dl->_IdxWritePtr[0] = (ImDrawIdx)(vtx_inner_idx); dl->_IdxWritePtr[1] = (ImDrawIdx)(vtx_inner_idx+((i-1)<<1)); dl->_IdxWritePtr[2] = (ImDrawIdx)(vtx_inner_idx+(i<<1));
            dl->_IdxWritePtr += 3;
        }

        // Compute normals
        ImVec2* temp_normals = (ImVec2*)alloca(points_count * sizeof(ImVec2));
        for (int i0 = points_count-1, i1 = 0; i1 < points_count; i0 = i1++)
        {
            const ImVec2& p0 = points[i0];
            const ImVec2& p1 = points[i1];
            ImVec2 diff = p1 - p0;
            diff *= ImInvLength(diff, 1.0f);
            temp_normals[i0].x = diff.y;
            temp_normals[i0].y = -diff.x;
        }

        for (int i0 = points_count-1, i1 = 0; i1 < points_count; i0 = i1++)
        {
            // Average normals
            const ImVec2& n0 = temp_normals[i0];
            const ImVec2& n1 = temp_normals[i1];
            ImVec2 dm = (n0 + n1) * 0.5f;
            float dmr2 = dm.x*dm.x + dm.y*dm.y;
            if (dmr2 > 0.000001f)
            {
                float scale = 1.0f / dmr2;
                if (scale > 100.0f) scale = 100.0f;
                dm *= scale;
            }
            dm *= AA_SIZE * 0.5f;

            // Add vertices
            //_VtxWritePtr[0].pos = (points[i1] - dm); _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;        // Inner
            //_VtxWritePtr[1].pos = (points[i1] + dm); _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col_trans;  // Outer
            dl->_VtxWritePtr[0].pos = (points[i1] - dm); dl->_VtxWritePtr[0].uv = uv; dl->_VtxWritePtr[0].col = GetVerticalGradient(colLeftf,colRightf,points[i1].x-minx,width);        // Inner
            dl->_VtxWritePtr[1].pos = (points[i1] + dm); dl->_VtxWritePtr[1].uv = uv; dl->_VtxWritePtr[1].col = GetVerticalGradient(colTransLeftf,colTransRightf,points[i1].x-minx,width);  // Outer
            dl->_VtxWritePtr += 2;

            // Add indexes for fringes
            dl->_IdxWritePtr[0] = (ImDrawIdx)(vtx_inner_idx+(i1<<1)); dl->_IdxWritePtr[1] = (ImDrawIdx)(vtx_inner_idx+(i0<<1)); dl->_IdxWritePtr[2] = (ImDrawIdx)(vtx_outer_idx+(i0<<1));
            dl->_IdxWritePtr[3] = (ImDrawIdx)(vtx_outer_idx+(i0<<1)); dl->_IdxWritePtr[4] = (ImDrawIdx)(vtx_outer_idx+(i1<<1)); dl->_IdxWritePtr[5] = (ImDrawIdx)(vtx_inner_idx+(i1<<1));
            dl->_IdxWritePtr += 6;
        }
        dl->_VtxCurrentIdx += (ImDrawIdx)vtx_count;
    }
    else
    {
        // Non Anti-aliased Fill
        const int idx_count = (points_count-2)*3;
        const int vtx_count = points_count;
        dl->PrimReserve(idx_count, vtx_count);
        for (int i = 0; i < vtx_count; i++)
        {
            //_VtxWritePtr[0].pos = points[i]; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
            dl->_VtxWritePtr[0].pos = points[i]; dl->_VtxWritePtr[0].uv = uv; dl->_VtxWritePtr[0].col = GetVerticalGradient(colLeftf,colRightf,points[i].x-minx,width);
            dl->_VtxWritePtr++;
        }
        for (int i = 2; i < points_count; i++)
        {
            dl->_IdxWritePtr[0] = (ImDrawIdx)(dl->_VtxCurrentIdx); dl->_IdxWritePtr[1] = (ImDrawIdx)(dl->_VtxCurrentIdx+i-1); dl->_IdxWritePtr[2] = (ImDrawIdx)(dl->_VtxCurrentIdx+i);
            dl->_IdxWritePtr += 3;
        }
        dl->_VtxCurrentIdx += (ImDrawIdx)vtx_count;
    }
}
void ImDrawListPathFillWithHorizontalGradientAndStroke(ImDrawList *dl, const ImU32 &fillColorLeft, const ImU32 &fillColorRight, const ImU32 &strokeColor, bool strokeClosed, float strokeThickness, float minx, float maxx)    {
    if (!dl) return;
    if (fillColorLeft==fillColorRight) dl->AddConvexPolyFilled(dl->_Path.Data,dl->_Path.Size, fillColorLeft);
    else if ((fillColorLeft & IM_COL32_A_MASK) != 0 || (fillColorRight & IM_COL32_A_MASK) != 0) ImDrawListAddConvexPolyFilledWithHorizontalGradient(dl, dl->_Path.Data, dl->_Path.Size, fillColorLeft, fillColorRight,minx,maxx);
    if ((strokeColor& IM_COL32_A_MASK)!= 0 && strokeThickness>0) dl->AddPolyline(dl->_Path.Data, dl->_Path.Size, strokeColor, strokeClosed, strokeThickness);
    dl->PathClear();
}
void ImDrawListAddRectWithHorizontalGradient(ImDrawList *dl, const ImVec2 &a, const ImVec2 &b, const ImU32 &fillColorLeft, const ImU32 &fillColoRight, const ImU32 &strokeColor, float rounding, int rounding_corners, float strokeThickness) {
    if (!dl || (((fillColorLeft & IM_COL32_A_MASK) == 0) && ((fillColoRight & IM_COL32_A_MASK) == 0) && ((strokeColor & IM_COL32_A_MASK) == 0)))  return;
    if (rounding==0.f || rounding_corners==0) {
        dl->AddRectFilledMultiColor(a,b,fillColorLeft,fillColoRight,fillColoRight,fillColorLeft); // Huge speedup!
        if ((strokeColor& IM_COL32_A_MASK)!= 0 && strokeThickness>0.f) {
            dl->PathRect(a, b, rounding, rounding_corners);
            dl->AddPolyline(dl->_Path.Data, dl->_Path.Size, strokeColor, true, strokeThickness);
            dl->PathClear();
        }
    }
    else    {
        dl->PathRect(a, b, rounding, rounding_corners);
        ImDrawListPathFillWithHorizontalGradientAndStroke(dl,fillColorLeft,fillColoRight,strokeColor,true,strokeThickness,a.x,b.x);
    }
}
void ImDrawListAddRectWithHorizontalGradient(ImDrawList *dl, const ImVec2 &a, const ImVec2 &b, const ImU32 &fillColor, float fillColorGradientDeltaIn0_05, const ImU32 &strokeColor, float rounding, int rounding_corners, float strokeThickness)   {
    ImU32 fillColorTop,fillColorBottom;GetVerticalGradientTopAndBottomColors(fillColor,fillColorGradientDeltaIn0_05,fillColorTop,fillColorBottom);
    ImDrawListAddRectWithHorizontalGradient(dl,a,b,fillColorTop,fillColorBottom,strokeColor,rounding,rounding_corners,strokeThickness);
}

}   //DrawListHelper

namespace VerticalTextHelper {
ImVec2 CalcVerticalTextSize(const char* text, const char* text_end = NULL, bool hide_text_after_double_hash = false, float wrap_width = -1.0f) {
    const ImVec2 rv = ImGui::CalcTextSize(text,text_end,hide_text_after_double_hash,wrap_width);
    return ImVec2(rv.y,rv.x);
}
void RenderTextVertical(const ImFont* font,ImDrawList* draw_list, float size, ImVec2 pos, ImU32 col, const ImVec4& clip_rect, const char* text_begin,  const char* text_end=NULL, float wrap_width=0.0f, bool cpu_fine_clip=false, bool rotateCCW=false) {
    if (!text_end) text_end = text_begin + strlen(text_begin);

    const float scale = size / font->FontSize;
    const float line_height = font->FontSize * scale;

    // Align to be pixel perfect
    pos.x = (float)(int)pos.x;// + (rotateCCW ? (font->FontSize-font->DisplayOffset.y) : 0);  // Not sure it's correct
	pos.y = (float)(int)pos.y + font->ConfigData->GlyphOffset.x;


    float x = pos.x;
    float y = pos.y;
    if (x > clip_rect.z)
        return;

    const bool word_wrap_enabled = (wrap_width > 0.0f);
    const char* word_wrap_eol = NULL;
    const float y_dir = rotateCCW ? -1.f : 1.f;

    // Skip non-visible lines
    const char* s = text_begin;
    if (!word_wrap_enabled && y + line_height < clip_rect.y)
        while (s < text_end && *s != '\n')  // Fast-forward to next line
            s++;

    // Reserve vertices for remaining worse case (over-reserving is useful and easily amortized)
    const int vtx_count_max = (int)(text_end - s) * 4;
    const int idx_count_max = (int)(text_end - s) * 6;
    const int idx_expected_size = draw_list->IdxBuffer.Size + idx_count_max;
    draw_list->PrimReserve(idx_count_max, vtx_count_max);

    ImDrawVert* vtx_write = draw_list->_VtxWritePtr;
    ImDrawIdx* idx_write = draw_list->_IdxWritePtr;
    unsigned int vtx_current_idx = draw_list->_VtxCurrentIdx;
    float x1=0.f,x2=0.f,y1=0.f,y2=0.f;

    while (s < text_end)
    {
        if (word_wrap_enabled)
        {
            // Calculate how far we can render. Requires two passes on the string data but keeps the code simple and not intrusive for what's essentially an uncommon feature.
            if (!word_wrap_eol)
            {
                word_wrap_eol = font->CalcWordWrapPositionA(scale, s, text_end, wrap_width - (y - pos.y));
                if (word_wrap_eol == s) // Wrap_width is too small to fit anything. Force displaying 1 character to minimize the height discontinuity.
                    word_wrap_eol++;    // +1 may not be a character start point in UTF-8 but it's ok because we use s >= word_wrap_eol below
            }

            if (s >= word_wrap_eol)
            {
                y = pos.y;
                x += line_height;
                word_wrap_eol = NULL;

                // Wrapping skips upcoming blanks
                while (s < text_end)
                {
					//const char c = *s;
					assert(false);
					//if (ImCharIsSpace(c)) { s++; } else if (c == '\n') { s++; break; } else { break; }
                }
                continue;
            }
        }

        // Decode and advance source
        unsigned int c = (unsigned int)*s;
        if (c < 0x80)
        {
            s += 1;
        }
        else
        {
            s += ImTextCharFromUtf8(&c, s, text_end);
            if (c == 0)
                break;
        }

        if (c < 32)
        {
            if (c == '\n')
            {
                y = pos.y;
                x += line_height;

                if (x > clip_rect.z)
                    break;
                if (!word_wrap_enabled && x + line_height < clip_rect.x)
                    while (s < text_end && *s != '\n')  // Fast-forward to next line
                        s++;
                continue;
            }
            if (c == '\r')
                continue;
        }

        float char_width = 0.0f;
        if (const ImFontGlyph* glyph = font->FindGlyph((unsigned short)c))
        {
            char_width = glyph->AdvanceX * scale;
            //fprintf(stderr,"%c [%1.4f]\n",(unsigned char) glyph->Codepoint,char_width);

            // Arbitrarily assume that both space and tabs are empty glyphs as an optimization
            if (c != ' ' && c != '\t')
            {
                // We don't do a second finer clipping test on the Y axis as we've already skipped anything before clip_rect.y and exit once we pass clip_rect.w
                if (!rotateCCW)  {
                    x1 = x + (font->FontSize-glyph->Y1) * scale;
                    x2 = x + (font->FontSize-glyph->Y0) * scale;
                    y1 = y + glyph->X0 * scale;
                    y2 = y + glyph->X1 * scale;
                }
                else {
                    x1 = x + glyph->Y0 * scale;
                    x2 = x + glyph->Y1 * scale;
                    y1 = y - glyph->X1 * scale;
                    y2 = y - glyph->X0 * scale;
                }
                if (y1 <= clip_rect.w && y2 >= clip_rect.y)
                {
                    // Render a character
                    float u1 = glyph->U0;
                    float v1 = glyph->V0;
                    float u2 = glyph->U1;
                    float v2 = glyph->V1;

                    // CPU side clipping used to fit text in their frame when the frame is too small. Only does clipping for axis aligned quads.
                    if (cpu_fine_clip)
                    {
                        if (x1 < clip_rect.x)
                        {
                            u1 = u1 + (1.0f - (x2 - clip_rect.x) / (x2 - x1)) * (u2 - u1);
                            x1 = clip_rect.x;
                        }
                        if (y1 < clip_rect.y)
                        {
                            v1 = v1 + (1.0f - (y2 - clip_rect.y) / (y2 - y1)) * (v2 - v1);
                            y1 = clip_rect.y;
                        }
                        if (x2 > clip_rect.z)
                        {
                            u2 = u1 + ((clip_rect.z - x1) / (x2 - x1)) * (u2 - u1);
                            x2 = clip_rect.z;
                        }
                        if (y2 > clip_rect.w)
                        {
                            v2 = v1 + ((clip_rect.w - y1) / (y2 - y1)) * (v2 - v1);
                            y2 = clip_rect.w;
                        }
                        if (x1 >= x2)
                        {
                            y += char_width*y_dir;
                            continue;
                        }
                    }

                    // We are NOT calling PrimRectUV() here because non-inlined causes too much overhead in a debug build.
                    // Inlined here:
                    {
                        idx_write[0] = (ImDrawIdx)(vtx_current_idx); idx_write[1] = (ImDrawIdx)(vtx_current_idx+1); idx_write[2] = (ImDrawIdx)(vtx_current_idx+2);
                        idx_write[3] = (ImDrawIdx)(vtx_current_idx); idx_write[4] = (ImDrawIdx)(vtx_current_idx+2); idx_write[5] = (ImDrawIdx)(vtx_current_idx+3);
                        vtx_write[0].col = vtx_write[1].col = vtx_write[2].col = vtx_write[3].col = col;
                        vtx_write[0].pos.x = x1; vtx_write[0].pos.y = y1;
                        vtx_write[1].pos.x = x2; vtx_write[1].pos.y = y1;
                        vtx_write[2].pos.x = x2; vtx_write[2].pos.y = y2;
                        vtx_write[3].pos.x = x1; vtx_write[3].pos.y = y2;

                        if (rotateCCW) {
                            vtx_write[0].uv.x = u2; vtx_write[0].uv.y = v1;
                            vtx_write[1].uv.x = u2; vtx_write[1].uv.y = v2;
                            vtx_write[2].uv.x = u1; vtx_write[2].uv.y = v2;
                            vtx_write[3].uv.x = u1; vtx_write[3].uv.y = v1;
                        }
                        else {
                            vtx_write[0].uv.x = u1; vtx_write[0].uv.y = v2;
                            vtx_write[1].uv.x = u1; vtx_write[1].uv.y = v1;
                            vtx_write[2].uv.x = u2; vtx_write[2].uv.y = v1;
                            vtx_write[3].uv.x = u2; vtx_write[3].uv.y = v2;
                        }

                        vtx_write += 4;
                        vtx_current_idx += 4;
                        idx_write += 6;
                    }
                }
            }
        }

        y += char_width*y_dir;
    }

    // Give back unused vertices
    draw_list->VtxBuffer.resize((int)(vtx_write - draw_list->VtxBuffer.Data));
    draw_list->IdxBuffer.resize((int)(idx_write - draw_list->IdxBuffer.Data));
    draw_list->CmdBuffer[draw_list->CmdBuffer.Size-1].ElemCount -= (idx_expected_size - draw_list->IdxBuffer.Size);
    draw_list->_VtxWritePtr = vtx_write;
    draw_list->_IdxWritePtr = idx_write;
    draw_list->_VtxCurrentIdx = (unsigned int)draw_list->VtxBuffer.Size;
}
void AddTextVertical(ImDrawList* drawList,const ImFont* font, float font_size, const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end=NULL, float wrap_width=0.0f, const ImVec4* cpu_fine_clip_rect=NULL,bool rotateCCW = false)    {
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    if (text_end == NULL)
        text_end = text_begin + strlen(text_begin);
    if (text_begin == text_end)
        return;

    // Note: This is one of the few instance of breaking the encapsulation of ImDrawList, as we pull this from ImGui state, but it is just SO useful.
    // Might just move Font/FontSize to ImDrawList?
    if (font == NULL)
        font = GImGui->Font;
    if (font_size == 0.0f)
        font_size = GImGui->FontSize;

    IM_ASSERT(drawList && font->ContainerAtlas->TexID == drawList->_TextureIdStack.back());  // Use high-level ImGui::PushFont() or low-level ImDrawList::PushTextureId() to change font.

    ImVec4 clip_rect = drawList->_ClipRectStack.back();
    if (cpu_fine_clip_rect)
    {
        clip_rect.x = ImMax(clip_rect.x, cpu_fine_clip_rect->x);
        clip_rect.y = ImMax(clip_rect.y, cpu_fine_clip_rect->y);
        clip_rect.z = ImMin(clip_rect.z, cpu_fine_clip_rect->z);
        clip_rect.w = ImMin(clip_rect.w, cpu_fine_clip_rect->w);
    }
    RenderTextVertical(font, drawList, font_size, pos, col, clip_rect, text_begin, text_end, wrap_width, cpu_fine_clip_rect != NULL,rotateCCW);
}
void AddTextVertical(ImDrawList* drawList,const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end=NULL,bool rotateCCW=false)    {
    AddTextVertical(drawList,GImGui->Font, GImGui->FontSize, pos, col, text_begin, text_end,0.0f,NULL,rotateCCW);
}
void RenderTextVerticalClipped(const ImVec2& pos_min, const ImVec2& pos_max, const char* text, const char* text_end, const ImVec2* text_size_if_known, const ImVec2& align =  ImVec2(0.0f,0.0f), const ImVec2* clip_min=NULL, const ImVec2* clip_max=NULL,bool rotateCCW=false)    {
    // Hide anything after a '##' string
    const char* text_display_end = FindRenderedTextEnd(text, text_end);
    const int text_len = (int)(text_display_end - text);
    if (text_len == 0)
        return;

    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = GetCurrentWindow();

    // Perform CPU side clipping for single clipped element to avoid using scissor state
    ImVec2 pos = pos_min;
    const ImVec2 text_size = text_size_if_known ? *text_size_if_known : CalcVerticalTextSize(text, text_display_end, false, 0.0f);

    if (!clip_max) clip_max = &pos_max;
    bool need_clipping = (pos.x + text_size.x >= clip_max->x) || (pos.y + text_size.y >= clip_max->y);
    if (!clip_min) clip_min = &pos_min; else need_clipping |= (pos.x < clip_min->x) || (pos.y < clip_min->y);

    // Align
    /*if (align & ImGuiAlign_Center) pos.x = ImMax(pos.x, (pos.x + pos_max.x - text_size.x) * 0.5f);
    else if (align & ImGuiAlign_Right) pos.x = ImMax(pos.x, pos_max.x - text_size.x);
    if (align & ImGuiAlign_VCenter) pos.y = ImMax(pos.y, (pos.y + pos_max.y - text_size.y) * 0.5f);*/

    //if (align & ImGuiAlign_Center) pos.y = ImMax(pos.y, (pos.y + pos_max.y - text_size.y) * 0.5f);
    //else if (align & ImGuiAlign_Right) pos.y = ImMax(pos.y, pos_max.y - text_size.y);
    //if (align & ImGuiAlign_VCenter) pos.x = ImMax(pos.x, (pos.x + pos_max.x - text_size.x) * 0.5f);

    // Align whole block (we should defer that to the better rendering function
    if (align.x > 0.0f) pos.x = ImMax(pos.x, pos.x + (pos_max.x - pos.x - text_size.x) * align.x);
    if (align.y > 0.0f) pos.y = ImMax(pos.y, pos.y + (pos_max.y - pos.y - text_size.y) * align.y);


    if (rotateCCW) pos.y+=text_size.y;
    // Render
    if (need_clipping)
    {
        ImVec4 fine_clip_rect(clip_min->x, clip_min->y, clip_max->x, clip_max->y);
        AddTextVertical(window->DrawList,g.Font, g.FontSize, pos, GetColorU32(ImGuiCol_Text), text, text_display_end, 0.0f, &fine_clip_rect,rotateCCW);
    }
    else
    {
        AddTextVertical(window->DrawList,g.Font, g.FontSize, pos, GetColorU32(ImGuiCol_Text), text, text_display_end, 0.0f, NULL,rotateCCW);
    }
}
}

namespace RevertUpstreamBeginChildCommit   {
// That commit [2016/11/06 (1.50)] broke everything!
static bool OldBeginChild(const char* str_id, const ImVec2& size_arg = ImVec2(0,0), ImGuiWindowFlags extra_flags = 0)    {
    ImGuiWindow* parent_window = ImGui::GetCurrentWindow();
    const ImGuiID id = parent_window->GetID(str_id);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_ChildWindow;

    const ImVec2 content_avail = ImGui::GetContentRegionAvail();
    ImVec2 size = ImFloor(size_arg);
    const int auto_fit_axises = ((size.x == 0.0f) ? 0x01 : 0x00) | ((size.y == 0.0f) ? 0x02 : 0x00);
    if (size.x <= 0.0f)
    {
        //if (size.x == 0.0f) flags |= ImGuiWindowFlags_ChildWindowAutoFitX;
        size.x = ImMax(content_avail.x, 4.0f) - fabsf(size.x); // Arbitrary minimum zero-ish child size of 4.0f (0.0f causing too much issues)
    }
    if (size.y <= 0.0f)
    {
        //if (size.y == 0.0f) flags |= ImGuiWindowFlags_ChildWindowAutoFitY;
        size.y = ImMax(content_avail.y, 4.0f) - fabsf(size.y);
    }
    flags |= extra_flags;

    char title[256];
    ImFormatString(title, IM_ARRAYSIZE(title), "%s.%s", parent_window->Name, str_id);

    ImGui::SetNextWindowSize(size);
    bool ret = ImGui::Begin(title, NULL, flags);

    ImGuiWindow* child_window = ImGui::GetCurrentWindow();
    child_window->ChildId = id;
    child_window->AutoFitChildAxises = auto_fit_axises;
    //if (!(parent_window->Flags & ImGuiWindowFlags_ShowBorders)) child_window->Flags &= ~ImGuiWindowFlags_ShowBorders;

    // Process navigation-in immediately so NavInit can run on first frame
    if (!(flags & ImGuiWindowFlags_NavFlattened) && (child_window->DC.NavLayersActiveMask != 0 || child_window->DC.NavHasScroll) && GImGui->NavActivateId == id)
    {
        ImGui::FocusWindow(child_window);
        ImGui::NavInitWindow(child_window, false);
        ImGui::SetActiveID(id+1, child_window); // Steal ActiveId with a dummy id so that key-press won't activate child item
        GImGui->ActiveIdSource = ImGuiInputSource_Nav;
    }

    return ret;
}
} // namespace RevertUpstreamBeginChildCommit

// TabLabelStyle --------------------------------------------------------------------------------------------------
TabLabelStyle TabLabelStyle::style;
const char* TabLabelStyle::ColorNames[TabLabelStyle::Col_TabLabel_Count] = {
"Col_TabLabel","Col_TabLabelHovered","Col_TabLabelActive","Col_TabLabelBorder","Col_TabLabelText",
"Col_TabLabelSelected","Col_TabLabelSelectedHovered","Col_TabLabelSelectedActive","Col_TabLabelSelectedBorder",
"Col_TabLabelSelectedText","Col_TabLabelCloseButtonHovered","Col_TabLabelCloseButtonActive","Col_TabLabelCloseButtonBorder","Col_TabLabelCloseButtonTextHovered"
};
const char* TabLabelStyle::FontStyleNames[TabLabelStyle::FONT_STYLE_COUNT]={"FONT_STYLE_NORMAL","FONT_STYLE_BOLD","FONT_STYLE_ITALIC","FONT_STYLE_BOLD_ITALIC"};
const char* TabLabelStyle::TabStateNames[TabLabelStyle::TAB_STATE_COUNT]={"TAB_STATE_NORMAL","TAB_STATE_SELECTED","TAB_STATE_MODIFIED","TAB_STATE_SELECTED_MODIFIED"};
const ImFont* TabLabelStyle::ImGuiFonts[TabLabelStyle::FONT_STYLE_COUNT]={NULL,NULL,NULL,NULL};
// These bit operations are probably non-endian independent, but ImGui uses them too and so do I.
inline static ImU32 ColorMergeWithAlpha(ImU32 c,float alphaMult) {
    // I'm not sure how to convert these using IM_COL32_A_MASK...
    ImU32 alpha = ((float)(c>>24))*alphaMult;
    return ((c&0x00FFFFFF)|(alpha<<24));
}
inline static ImU32 ColorDarken(ImU32 c,float value,float optionalAlphaToSet=-1.f) {
    ImVec4 f = ImGui::ColorConvertU32ToFloat4(c);
    f.x-=value;if (f.x<0) f.x=0;else if (f.x>1) f.x=1;
    f.y-=value;if (f.y<0) f.y=0;else if (f.y>1) f.y=1;
    f.z-=value;if (f.z<0) f.z=0;else if (f.z>1) f.z=1;
    if (optionalAlphaToSet>=0 && optionalAlphaToSet<=1) f.w = optionalAlphaToSet;
    return ImGui::ColorConvertFloat4ToU32(f);
}
inline static ImU32 ColorLighten(ImU32 c,float value,float optionalAlphaToSet=-1.f) {return ColorDarken(c,-value,optionalAlphaToSet);}
// Helper inline methods to save some code in TabLabelStyle::ctr() and ResetTabLabelStyle(...)
inline static void TabLabelStyleSetSelectedTabColors(ImGui::TabLabelStyle& style,const ImColor& tabColor,const ImColor& textColor,const ImColor& borderColor)   {
    style.colors[TabLabelStyle::Col_TabLabelSelectedActive] = style.colors[TabLabelStyle::Col_TabLabelSelectedHovered] = style.colors[TabLabelStyle::Col_TabLabelSelected] = tabColor;
    style.colors[TabLabelStyle::Col_TabLabelSelectedText]               = textColor;
    style.colors[TabLabelStyle::Col_TabLabelSelectedBorder]             = borderColor;
}
inline static void TabLabelStyleSetTabColors(ImGui::TabLabelStyle& style,const ImColor& tabColor,const ImColor& tabColorHovered,const ImColor& textColor,const ImColor& borderColor)   {
    style.colors[TabLabelStyle::Col_TabLabel]           = tabColor;
    style.colors[TabLabelStyle::Col_TabLabelHovered]    = style.colors[TabLabelStyle::Col_TabLabelActive] = tabColorHovered;
    style.colors[TabLabelStyle::Col_TabLabelText]       = textColor;
    style.colors[TabLabelStyle::Col_TabLabelBorder]     = borderColor;
}
inline static void TabLabelStyleSetCloseButtonColors(ImGui::TabLabelStyle& style,const ImColor& hoveredBtnColor=ImColor(166,0,11,255),const ImColor& actveBtnColor=ImColor(206,40,51,255),const ImColor* pHoveredBtnTextColor=NULL,const ImColor* pHoveredBtnBorderColor=NULL)   {
    style.colors[TabLabelStyle::Col_TabLabelCloseButtonHovered]       = hoveredBtnColor;
    style.colors[TabLabelStyle::Col_TabLabelCloseButtonActive]        = actveBtnColor;
    style.colors[TabLabelStyle::Col_TabLabelCloseButtonTextHovered]   = (pHoveredBtnTextColor!=NULL) ? (ImU32)(*pHoveredBtnTextColor) : style.colors[TabLabelStyle::Col_TabLabelSelectedText];
    style.colors[TabLabelStyle::Col_TabLabelCloseButtonBorder]        = (pHoveredBtnBorderColor!=NULL) ? (ImU32)(*pHoveredBtnBorderColor) : style.colors[TabLabelStyle::Col_TabLabelSelectedBorder];
}
// End Helper inline methods to save some code in TabLabelStyle::ctr() and ResetTabLabelStyle(...)

TabLabelStyle::TabLabelStyle()    {

    fillColorGradientDeltaIn0_05 = 0.0f;rounding = 6.f;borderWidth = 0.f;
    closeButtonRounding = 0.f;closeButtonBorderWidth = 1.f;closeButtonTextWidth = 2.5f;//3.f;

    TabLabelStyleSetSelectedTabColors(*this,ImColor(0.267f,0.282f,0.396f,1.0f),ImColor(0.925f,0.945f,0.957,1.0f),ImColor(0.090f,0.106f,0.157f,0.000f));
    TabLabelStyleSetTabColors(*this,ImColor(0.161f,0.188f,0.204f,1.f),ImColor(0.239f,0.259f,0.275f,1.f),ImColor(0.549f,0.565f,0.576f,0.784f),ColorDarken(colors[Col_TabLabelSelectedBorder],.0225f));
    TabLabelStyleSetCloseButtonColors(*this);

    //for (int i=0;i<TAB_STATE_COUNT;i++) fontStyles[i] = FONT_STYLE_BOLD;    // looks better for me
    fontStyles[TAB_STATE_NORMAL]            = FONT_STYLE_NORMAL;
    fontStyles[TAB_STATE_SELECTED]          = FONT_STYLE_BOLD;
    fontStyles[TAB_STATE_MODIFIED]          = FONT_STYLE_ITALIC;
    fontStyles[TAB_STATE_SELECTED_MODIFIED] = FONT_STYLE_BOLD_ITALIC;

    tabWindowLabelBackgroundColor        = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    tabWindowLabelShowAreaSeparator      = false;
    tabWindowSplitterColor               = ImVec4(1,1,1,1);
    tabWindowSplitterSize                = 6.f;
}
void ChangeTabLabelStyleColors(TabLabelStyle& style,float satThresholdForInvertingLuminance,float shiftHue)  {
    if (satThresholdForInvertingLuminance>=1.f && shiftHue==0.f) return;
    for (int i = 0; i < TabLabelStyle::Col_TabLabel_Count; i++)	{
	ImVec4 col = ImGui::ColorConvertU32ToFloat4(style.colors[i]);
	float H, S, V;
	ImGui::ColorConvertRGBtoHSV( col.x, col.y, col.z, H, S, V );
	if( S <= satThresholdForInvertingLuminance)  { V = 1.0 - V; }
	if (shiftHue) {H+=shiftHue;if (H>1) H-=1.f;else if (H<0) H+=1.f;}
	ImGui::ColorConvertHSVtoRGB( H, S, V, col.x, col.y, col.z );
	style.colors[i] = ImGui::ColorConvertFloat4ToU32(col);
    }
}
bool TabLabelStyle::EditFast(TabLabelStyle &s)  {
    bool rv = false;
    static bool resetCurrentStyle = false;
    resetCurrentStyle = ImGui::Button("Reset Current Style To: ###TabLabelStyleResetTo");
    ImGui::SameLine();
    static int styleEnumNum = 0;
    ImGui::PushItemWidth(135);
    ImGui::Combo("###TabLabelStyleEnumCombo",&styleEnumNum,ImGui::GetDefaultTabLabelStyleNames(),(int) ImGuiTabLabelStyle_Count,(int) ImGuiTabLabelStyle_Count);
    ImGui::PopItemWidth();
    ImGui::SameLine();
    static float hueShift = 0;
    ImGui::PushItemWidth(50);
    ImGui::DragFloat("HueShift##tablabelstyleShiftHue",&hueShift,.005f,0,1,"%.2f");
    ImGui::PopItemWidth();
    if (hueShift!=0)   {
        ImGui::SameLine();
        if (ImGui::SmallButton("reset##tablabelstyleReset")) {hueShift=0.f;}
    }
    const bool mustInvertColors = ImGui::Button("Invert Colors:##tablabelstyleInvertColors");
    ImGui::SameLine();
    ImGui::PushItemWidth(50);
    static float invertColorThreshold = .1f;
    ImGui::DragFloat("Saturation Threshold##tablabelstyleLumThres",&invertColorThreshold,.005f,0.f,0.5f,"%.2f");
    ImGui::PopItemWidth();
    if (mustInvertColors)  ChangeTabLabelStyleColors(s,invertColorThreshold);
    if (resetCurrentStyle)  {
        ImGui::ResetTabLabelStyle(styleEnumNum,s);
        if (hueShift!=0) ChangeTabLabelStyleColors(s,0.f,hueShift);
        rv = true;
    }
    if (ImGui::Button("Invert Selected Look")) {InvertSelectedLook(s);rv=true;}
    ImGui::SameLine();
    if (ImGui::Button("Lighten Tab Colors")) {TabLabelStyle::LightenBackground(s,0.05f);rv=true;}
    ImGui::SameLine();
    if (ImGui::Button("Darken Tab Colors")) {TabLabelStyle::DarkenBackground(s,0.05f);rv=true;}
    return rv;
}
bool TabLabelStyle::Edit(TabLabelStyle &s)  {
    bool changed = false;
    const float dragSpeed = 0.25f;
    const char prec[] = "%1.1f";
    ImGui::PushID(&s);

    static bool useSimplifiedInterface = true;
    if (ImGui::Button(useSimplifiedInterface ? "Use Complex Interface###TLSInter" : "Use Simple Interface###TLSInter")) useSimplifiedInterface = !useSimplifiedInterface;
    ImGui::Separator();

    if (useSimplifiedInterface)	{
        changed|=EditFast(s);
        ImGui::Separator();
    }

    ImGui::Text("Tab Labels:");
    ImGui::PushItemWidth(50);
    changed|=ImGui::DragFloat("fillColorGradientDeltaIn0_05",&s.fillColorGradientDeltaIn0_05,0.01f,0.f,.5f,"%1.3f");
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s","Zero gradient (render)s much faster\nwhen \"rounding\" is positive.");
    changed|=ImGui::DragFloat("rounding",&s.rounding,dragSpeed,0.0f,16.f,prec);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s","Small values render faster\nbut to really speed up gradients\nset this to zero.");
    changed|=ImGui::DragFloat("borderWidth",&s.borderWidth,.01f,0.f,5.f,"%1.2f");
    ImGui::Spacing();

    if (!useSimplifiedInterface)    {
	changed|=ImGui::DragFloat("closeButtonRounding",&s.closeButtonRounding,dragSpeed,0.0f,16.f,prec);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s","I suggest setting this to zero...");
	changed|=ImGui::DragFloat("closeButtonBorderWidth",&s.closeButtonBorderWidth,.01f,0.f,5.f,"%1.2f");
    }
    changed|=ImGui::DragFloat("closeButtonTextWidth",&s.closeButtonTextWidth,.01f,0.f,5.f,"%1.2f");
    ImGui::Spacing();
    ImGui::PopItemWidth();

    //changed|=ImGui::Checkbox("antialiasing",&s.antialiasing);
    ImGui::Spacing();

    ImGui::Text("Colors:");
    ImGui::PushItemWidth(ImGui::GetWindowWidth()*0.5f);
    for (int item = 0,itemSz=(int)TabLabelStyle::Col_TabLabel_Count;item<itemSz;item++) {
	if (useSimplifiedInterface && (item==Col_TabLabelActive || item==Col_TabLabelSelectedActive
				       || item==Col_TabLabelSelectedHovered)) continue;
	if (item==Col_TabLabelSelected || item==Col_TabLabelCloseButtonHovered) ImGui::Spacing();
	ImVec4 tmp = ImColor(s.colors[item]);
        const bool color_changed = ImGui::ColorEdit4(TabLabelStyle::ColorNames[item],&tmp.x);
	if (color_changed) {
	    s.colors[item] = ImGui::ColorConvertFloat4ToU32(tmp);
	    if (useSimplifiedInterface) {
		switch (item) {
		case Col_TabLabelHovered:
		    s.colors[Col_TabLabelActive] = s.colors[Col_TabLabelHovered];
		break;
		case Col_TabLabelSelected:
		    s.colors[Col_TabLabelSelectedActive] = s.colors[Col_TabLabelSelectedHovered] = s.colors[Col_TabLabelSelected];
		break;
		default:
		break;
		}
	    }
	}
        changed|=color_changed;
    }
    ImGui::PopItemWidth();
    ImGui::Spacing();

    ImGui::PushItemWidth(ImGui::GetWindowWidth()*0.5f);
    if (!useSimplifiedInterface)    {
	ImGui::Text("Fonts (needs TabLabelStyle::ImGuiFonts[TAB_STATE_COUNT]):");
	for (int item = 0,itemSz=(int)TabLabelStyle::TAB_STATE_COUNT;item<itemSz;item++) {
	    changed|=ImGui::Combo(TabStateNames[item],&s.fontStyles[item],&FontStyleNames[0],FONT_STYLE_COUNT);
	}
	ImGui::Spacing();
    }

    if (!useSimplifiedInterface)    {
	ImGui::Text("TabWindow:");
	changed|=ImGui::ColorEdit4("tabWindowLabelBackgroundColor",&s.tabWindowLabelBackgroundColor.x);
	changed|=ImGui::Checkbox("tabWindowLabelShowAreaSeparator",&s.tabWindowLabelShowAreaSeparator);
	changed|=ImGui::ColorEdit4("tabWindowSplitterColor",&s.tabWindowSplitterColor.x);
	ImGui::PushItemWidth(50);
	changed|=ImGui::DragFloat("tabWindowSplitterSize",&s.tabWindowSplitterSize,1,4,16,"%1.0f");
	ImGui::PopItemWidth();
	ImGui::Spacing();
    }
    ImGui::PopItemWidth();

    ImGui::PopID();
    return changed;
}

void TabLabelStyle::InvertSelectedLook(TabLabelStyle &style)    {
    ImU32 tmp(0);TabLabelStyle &s = style;
    tmp = s.colors[Col_TabLabel]; s.colors[Col_TabLabel] = s.colors[Col_TabLabelSelected]; s.colors[Col_TabLabelSelected] = tmp;
    tmp = s.colors[Col_TabLabelHovered]; s.colors[Col_TabLabelHovered] = s.colors[Col_TabLabelSelectedHovered]; s.colors[Col_TabLabelSelectedHovered] = tmp;
    tmp = s.colors[Col_TabLabelActive]; s.colors[Col_TabLabelActive] = s.colors[Col_TabLabelSelectedActive]; s.colors[Col_TabLabelSelectedActive] = tmp;
    tmp = s.colors[Col_TabLabelBorder]; s.colors[Col_TabLabelBorder] = s.colors[Col_TabLabelSelectedBorder]; s.colors[Col_TabLabelSelectedBorder] = tmp;
    tmp = s.colors[Col_TabLabelText]; s.colors[Col_TabLabelText] = s.colors[Col_TabLabelSelectedText]; s.colors[Col_TabLabelSelectedText] = tmp;
}
void TabLabelStyle::ShiftHue(TabLabelStyle &style, float amountIn0_1)   {ChangeTabLabelStyleColors(style,0.f,amountIn0_1);}
void TabLabelStyle::InvertColors(TabLabelStyle &style, float saturationThreshould)  {ChangeTabLabelStyleColors(style,saturationThreshould,0.f);}
void TabLabelStyle::LightenBackground(TabLabelStyle &style, float amount)   {
    TabLabelStyle &s = style;
    s.colors[Col_TabLabel]                  = ColorLighten(s.colors[Col_TabLabel],amount);
    s.colors[Col_TabLabelHovered]           = ColorLighten(s.colors[Col_TabLabelHovered],amount);
    s.colors[Col_TabLabelActive]            = ColorLighten(s.colors[Col_TabLabelActive],amount);
    //s.colors[Col_TabLabelBorder]          = ColorLighten(s.colors[Col_TabLabelBorder],amount);
    //s.colors[Col_TabLabelText]            = ColorLighten(s.colors[Col_TabLabelText],amount);

    s.colors[Col_TabLabelSelected]          = ColorLighten(s.colors[Col_TabLabelSelected],amount);
    s.colors[Col_TabLabelSelectedHovered]   = ColorLighten(s.colors[Col_TabLabelSelectedHovered],amount);
    s.colors[Col_TabLabelSelectedActive]    = ColorLighten(s.colors[Col_TabLabelSelectedActive],amount);
    //s.colors[Col_TabLabelSelectedBorder]  = ColorLighten(s.colors[Col_TabLabelSelectedBorder],amount);
    //s.colors[Col_TabLabelSelectedText]    = ColorLighten(s.colors[Col_TabLabelSelectedText],amount);
}
void TabLabelStyle::DarkenBackground(TabLabelStyle &style, float amount)    {LightenBackground(style,-amount);}

bool ResetTabLabelStyle(int tabLabelStyleEnum,ImGui::TabLabelStyle& style) {    
    if (tabLabelStyleEnum<0 || tabLabelStyleEnum>=ImGuiTabLabelStyle_Count) return false;
    style = TabLabelStyle();
    switch (tabLabelStyleEnum) {
    case ImGuiTabLabelStyle_Dark:
        style.fillColorGradientDeltaIn0_05 = 0.075f;style.rounding = 0.f;style.borderWidth = 1.f;
        TabLabelStyleSetSelectedTabColors(style,ImColor(49,54,58,255),ImColor(210,214,217,255),ImColor(23,27,40,250));
        TabLabelStyleSetTabColors(style,ColorDarken(style.colors[TabLabelStyle::Col_TabLabelSelected],.135f,1.f),ColorLighten(style.colors[TabLabelStyle::Col_TabLabel],.1f,1.f),ImColor(140,144,147,200),ColorDarken(style.colors[TabLabelStyle::Col_TabLabelSelectedBorder],.0225f,1.f));
        TabLabelStyleSetCloseButtonColors(style);
        break;
    case ImGuiTabLabelStyle_Red:
    case ImGuiTabLabelStyle_Green:
    case ImGuiTabLabelStyle_Blue:
    case ImGuiTabLabelStyle_Yellow:
    case ImGuiTabLabelStyle_Orange:
        style.fillColorGradientDeltaIn0_05 = 0.075f;style.rounding = 0.f;style.borderWidth = 1.f;
        // Colors for ImGuiTabLabelStyle_Red here:
        TabLabelStyleSetSelectedTabColors(style,ImColor(0.549f,0.108f,0.071f,1.000f),ImColor(0.863f,1.000f,0.965f,1.000f),ImColor(0.337f,0.125f,0.125f,1.000f));
        TabLabelStyleSetTabColors(style,ImColor(0.337f,0.162f,0.143f,0.981f),ImColor(0.525f,0.206f,0.163f,0.981f),ImColor(0.655f,0.745f,0.718f,0.981f),ImColor(0.537f,0.200f,0.200f,0.881f));
        // Hue shift if necessary here:
        if (tabLabelStyleEnum == ImGuiTabLabelStyle_Green)          TabLabelStyle::ShiftHue(style,0.32f);
        else if (tabLabelStyleEnum == ImGuiTabLabelStyle_Blue)      TabLabelStyle::ShiftHue(style,0.62f);
        else if (tabLabelStyleEnum == ImGuiTabLabelStyle_Yellow)    TabLabelStyle::ShiftHue(style,0.14f);
        else if (tabLabelStyleEnum == ImGuiTabLabelStyle_Orange)    TabLabelStyle::ShiftHue(style,0.08f);
        // set close button colors after shifting hue
        TabLabelStyleSetCloseButtonColors(style);
        break;
    case ImGuiTabLabelStyle_White:  {
        style.fillColorGradientDeltaIn0_05 = 0.0f;style.rounding = 6.f;style.borderWidth = 1.0f;
        TabLabelStyleSetSelectedTabColors(style,ImColor(1.f,1.f,1.f),ImColor(0.059f,0.059f,0.059f,1.f),ImColor(0.090f,0.106f,0.157f,0.706f));
        TabLabelStyleSetTabColors(style,ImColor(0.925f,0.953f,0.969f,1.f),ImColor(1.f,1.f,1.f,1.f),ImColor(0.247f,0.286f,0.294f,0.765f),ImColor(0.067f,0.082f,0.133f,0.353f));
        style.closeButtonBorderWidth = 1.f;style.closeButtonTextWidth = 2.5f;ImColor btc(1.f,1.f,1.f,1.),bbc(0.090f,0.106f,0.157,1.f);
        TabLabelStyleSetCloseButtonColors(style,ImColor(0.651f,0.000f,0.047f,0.490f),ImColor(0.949f,0.f,0.067f,1.f),&btc,&bbc);
    }
        break;
    case ImGuiTabLabelStyle_Foxy:
    case ImGuiTabLabelStyle_FoxyInverse:
        style.fillColorGradientDeltaIn0_05 = 0.f;style.rounding = 6.f;style.borderWidth = 1.f;//0.f
        TabLabelStyleSetSelectedTabColors(style,ImColor(213,212,211,255),ImColor(30,26,53,255),ImColor(136,137,135,255));
        TabLabelStyleSetTabColors(style,ImColor(60,59,55,255),ImColor(104,103,100,255),ImColor(223,219,210,255),ImColor(60,59,55,255));
        TabLabelStyleSetCloseButtonColors(style);style.colors[TabLabelStyle::Col_TabLabelCloseButtonBorder] = style.colors[TabLabelStyle::Col_TabLabelCloseButtonHovered];
        if (tabLabelStyleEnum == ImGuiTabLabelStyle_FoxyInverse) {
            TabLabelStyle::InvertSelectedLook(style);
            style.colors[TabLabelStyle::Col_TabLabelActive] = style.colors[TabLabelStyle::Col_TabLabelHovered] = ImColor(154,153,150,255);
            style.colors[TabLabelStyle::Col_TabLabelSelectedActive] = style.colors[TabLabelStyle::Col_TabLabelSelectedHovered] = style.colors[TabLabelStyle::Col_TabLabelSelected] = style.colors[TabLabelStyle::Col_TabLabelSelected];
        }
        break;
    case ImGuiTabLabelStyle_Tidy:   {
        style.fillColorGradientDeltaIn0_05 = 0.0f;style.rounding = 6.f;style.borderWidth = 1.5f;
        TabLabelStyleSetSelectedTabColors(style,ImColor(0.682f,0.682f,0.682f,0.941f),ImColor(0.000f,0.000f,0.000f,1.000f),ImColor(0.992f,0.992f,0.992f,1.000f));
        TabLabelStyleSetTabColors(style,ImColor(0.212f,0.212f,0.212f,1.000f),ImColor(0.392f,0.392f,0.392f,1.000f),ImColor(0.784f,0.784f,0.784f,1.000f),ImColor(0.541f,0.541f,0.541f,0.588f));
        style.closeButtonBorderWidth = 3.f;style.closeButtonTextWidth = 2.5f;ImColor btc(0.949f,0.949f,0.949f,1.000f),bbc(0.749f,0.749f,0.749f,0.549f);
        TabLabelStyleSetCloseButtonColors(style,ImColor(0.651f,0.000f,0.043f,0.608f),ImColor(0.808f,0.157f,0.200f,0.608f),&btc,&bbc);
    }
        break;
    default:
        break;
    }

    return true;
}
static const char* DefaultTabLabelStyleNames[ImGuiTabLabelStyle_Count]={"Default","Dark","Red","Green","Blue","Yellow","Orange","White","Tidy","Foxy","FoxyInverse"};
const char** GetDefaultTabLabelStyleNames() {return &DefaultTabLabelStyleNames[0];}


#if (defined(IMGUIHELPER_H_) && !defined(NO_IMGUIHELPER_SERIALIZATION))
#ifndef NO_IMGUIHELPER_SERIALIZATION_SAVE
#include "../imguihelper/imguihelper.h"
bool TabLabelStyle::Save(const TabLabelStyle &style, ImGuiHelper::Serializer& s) {
    if (!s.isValid()) return false;

    s.save(ImGui::FT_FLOAT,&style.fillColorGradientDeltaIn0_05,"fillColorGradientDeltaIn0_05");
    s.save(ImGui::FT_FLOAT,&style.rounding,"rounding");
    s.save(ImGui::FT_FLOAT,&style.borderWidth,"borderWidth");

    s.save(ImGui::FT_FLOAT,&style.closeButtonRounding,"closeButtonRounding");
    s.save(ImGui::FT_FLOAT,&style.closeButtonBorderWidth,"closeButtonBorderWidth");
    s.save(ImGui::FT_FLOAT,&style.closeButtonTextWidth,"closeButtonTextWidth");

    //s.save(&style.antialiasing,"antialiasing");

    ImVec4 tmpColor(1,1,1,1);
    for (int i=0;i<Col_TabLabel_Count;i++)    {
	tmpColor = ImColor(style.colors[i]);s.save(ImGui::FT_COLOR,&tmpColor.x,ColorNames[i],4);
    }

    for (int i=0;i<TAB_STATE_COUNT;i++)    {
	s.save(&style.fontStyles[i],TabStateNames[i]);
    }

    s.save(ImGui::FT_COLOR,&style.tabWindowLabelBackgroundColor.x,"tabWindowLabelBackgroundColor",4);
    s.save(&style.tabWindowLabelShowAreaSeparator,"tabWindowLabelShowAreaSeparator");
    s.save(ImGui::FT_COLOR,&style.tabWindowSplitterColor.x,"tabWindowSplitterColor",4);
    s.save(ImGui::FT_FLOAT,&style.tabWindowSplitterSize,"tabWindowSplitterSize");

    return true;
}
#endif //NO_IMGUIHELPER_SERIALIZATION_SAVE
#ifndef NO_IMGUIHELPER_SERIALIZATION_LOAD
#include "../imguihelper/imguihelper.h"
static bool TabLabelStyleParser(ImGuiHelper::FieldType ft,int /*numArrayElements*/,void* pValue,const char* name,void* userPtr)    {
    TabLabelStyle& s = *((TabLabelStyle*) userPtr);
    ImVec4& tmp = *((ImVec4*) pValue);  // we cast it soon to float for now...
    switch (ft) {
    case ImGui::FT_FLOAT:
	if (strcmp(name,"fillColorGradientDeltaIn0_05")==0)		s.fillColorGradientDeltaIn0_05 = tmp.x;
	else if (strcmp(name,"rounding")==0)				s.rounding = tmp.x;
	else if (strcmp(name,"borderWidth")==0)				s.borderWidth = tmp.x;
	else if (strcmp(name,"closeButtonRounding")==0)			s.closeButtonRounding = tmp.x;
	else if (strcmp(name,"closeButtonBorderWidth")==0)		s.closeButtonBorderWidth = tmp.x;
	else if (strcmp(name,"closeButtonTextWidth")==0)		s.closeButtonTextWidth = tmp.x;
	else if (strcmp(name,"tabWindowSplitterSize")==0)		{s.tabWindowSplitterSize = tmp.x;return true;}	// Returning true at the end allows queuing serialized elements in a single file
    break;
    case ImGui::FT_INT:
	for (int i=0;i<TabLabelStyle::TAB_STATE_COUNT;i++)  {
	    if (strcmp(name,TabLabelStyle::TabStateNames[i])==0)	{s.fontStyles[i] = *((int*)pValue);if (s.fontStyles[i]<0 || s.fontStyles[i]>3) s.fontStyles[i]=0;break;}
	}
    break;
    case ImGui::FT_BOOL:
    //if (strcmp(name,"antialiasing")==0)				s.antialiasing = *((bool*)pValue);
    //else
    if (strcmp(name,"tabWindowLabelShowAreaSeparator")==0)	s.tabWindowLabelShowAreaSeparator = *((bool*)pValue);
    break;
    case ImGui::FT_COLOR:
	if (strcmp(name,"tabWindowLabelBackgroundColor")==0)		    s.tabWindowLabelBackgroundColor = ImColor(tmp);
	else if (strcmp(name,"tabWindowSplitterColor")==0)		    s.tabWindowSplitterColor = ImColor(tmp);
	else {
	    for (int i=0;i<TabLabelStyle::Col_TabLabel_Count;i++)  {
		if (strcmp(name,TabLabelStyle::ColorNames[i])==0)	    {
		    s.colors[i] = ImColor(tmp);break;
		}
	    }
	}
    break;
    default:
    // TODO: check
    break;
    }
    return false;
}
bool TabLabelStyle::Load(TabLabelStyle &style,ImGuiHelper::Deserializer& d, const char ** pOptionalBufferStart)  {
    if (!d.isValid()) return false;
    const char* amount = pOptionalBufferStart ? (*pOptionalBufferStart) : 0;
    amount = d.parse(TabLabelStyleParser,(void*)&style,amount);
    if (pOptionalBufferStart) *pOptionalBufferStart=amount;
    return true;
}
#endif //NO_IMGUIHELPER_SERIALIZATION_LOAD
#endif //NO_IMGUIHELPER_SERIALIZATION

const TabLabelStyle& TabLabelStyle::GetMergedWithWindowAlpha()    {
    // Actually here I'm just using GImGui->Style.Alpha.
    // I guess there's some per-window alpha I have to multiply...
    static TabLabelStyle S;
    static int frameCnt=-1;    
    if (frameCnt!=ImGui::GetFrameCount())   {
        frameCnt=ImGui::GetFrameCount();
    const float alpha = GImGui->Style.Alpha * GImGui->Style.Colors[ImGuiCol_WindowBg].w;
        S = TabLabelStyle::style;
    for (int i=0;i<Col_TabLabel_Count;i++) S.colors[i] = ColorMergeWithAlpha(style.colors[i],alpha);
	S.tabWindowLabelBackgroundColor.w*=alpha;
	S.tabWindowSplitterColor.w*=alpha;
    }
    return S;
}
inline const TabLabelStyle& TabLabelStyleGetMergedWithAlphaForOverlayUsage()    {
    static TabLabelStyle S;static int frameCnt=-1;static const float alpha = 0.75f;
    if (frameCnt!=ImGui::GetFrameCount())   {
        frameCnt=ImGui::GetFrameCount();
        S = TabLabelStyle::style;
	for (int i=0;i<TabLabelStyle::Col_TabLabel_Count;i++) S.colors[i] = ColorMergeWithAlpha(TabLabelStyle::style.colors[i],alpha);
        S.tabWindowLabelBackgroundColor.w*=alpha;
        S.tabWindowSplitterColor.w*=alpha;
    }
    return S;
}
//----------------------------------------------------------------------------------------------------------------------

//=======================================================================================
// Main method to draw the tab label
// The TabLabelStyle used by this method won't be merged with the Window Alpha (please provide a pOptionalStyleToUseIn using TabLabelStyle::GetMergedWithWindowAlpha() if needed).
static bool TabButton(const char *label, bool selected, bool *pCloseButtonPressedOut=NULL, const char* textOverrideIn=NULL, ImVec2 *pJustReturnItsSizeHereOut=NULL, const TabLabelStyle* pOptionalStyleToUseIn=NULL,ImFont *fontOverride=NULL, ImVec2 *pOptionalJustDrawTabButtonGraphicsUnderMouseWithThisOffset=NULL, ImDrawList *drawListOverride=NULL,bool privateReuseLastCalculatedLabelSizeDoNotUse = false,bool forceActiveColorLook = false)  {
    // Based on ImGui::ButtonEx(...)
    bool *pHoveredOut = NULL;           // removed from args (can be queried from outside)
    bool *pCloseButtonHovered = NULL;   // removed from args (who cares if the close button is hovered?)
    const int flags = 0;                // what's this ?
    const bool hasCloseButton = pCloseButtonHovered || pCloseButtonPressedOut;

    const bool isFakeControl = pOptionalJustDrawTabButtonGraphicsUnderMouseWithThisOffset || pJustReturnItsSizeHereOut;

    ImGuiWindow* window = GetCurrentWindow();
    if (window && window->SkipItems && !isFakeControl)  return false;

    //ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = ImGui::GetStyle();
    const TabLabelStyle& tabStyle = pOptionalStyleToUseIn ? *pOptionalStyleToUseIn : TabLabelStyle::Get();
    const ImGuiID id = isFakeControl ? 0 : window->GetID(label);
    if (textOverrideIn) label = textOverrideIn;

    if (!fontOverride) fontOverride = (ImFont*) (selected ? TabLabelStyle::ImGuiFonts[tabStyle.fontStyles[TabLabelStyle::TAB_STATE_SELECTED]] : TabLabelStyle::ImGuiFonts[tabStyle.fontStyles[TabLabelStyle::TAB_STATE_NORMAL]]);
    if (fontOverride) ImGui::PushFont(fontOverride);
    static ImVec2 staticLabelSize(0,0);
    ImVec2 label_size(0,0);
    if (!privateReuseLastCalculatedLabelSizeDoNotUse) label_size = staticLabelSize = ImGui::CalcTextSize(label, NULL, true);
    else label_size = staticLabelSize;

    ImVec2 pos = window ? window->DC.CursorPos : ImVec2(0,0);
    if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrLineTextBaseOffset)    pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
    ImVec2 size(label_size.x + (style.FramePadding.x+tabStyle.borderWidth) * 2.0f, label_size.y + (style.FramePadding.y+tabStyle.borderWidth) * 2.0f);
    float btnWidth = label_size.y*0.75f,btnSpacingX = label_size.y*0.25f;
    float extraWidthForBtn = hasCloseButton ? (btnSpacingX*2.f+btnWidth) : 0;
    if (hasCloseButton) size.x+=extraWidthForBtn;
    if (pJustReturnItsSizeHereOut) {*pJustReturnItsSizeHereOut=size;if (fontOverride) ImGui::PopFont();return false;}

    const ImRect bb(pOptionalJustDrawTabButtonGraphicsUnderMouseWithThisOffset ? *pOptionalJustDrawTabButtonGraphicsUnderMouseWithThisOffset : pos,
                    (pOptionalJustDrawTabButtonGraphicsUnderMouseWithThisOffset ? *pOptionalJustDrawTabButtonGraphicsUnderMouseWithThisOffset : pos) + size);
    if (!pOptionalJustDrawTabButtonGraphicsUnderMouseWithThisOffset) {
        ItemSize(bb, style.FramePadding.y);
        if (!ItemAdd(bb, id)) {if (fontOverride) ImGui::PopFont();return false;}
    }

    //if (window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat) flags |= ImGuiButtonFlags_Repeat;    // What's this ?
    bool hovered=false, held=false;
    bool pressed = isFakeControl ? false : ButtonBehavior(bb, id, &hovered, &held, flags);
    bool btnHovered = false;
    bool btnPressed = false;
    ImVec2 startBtn(0,0),endBtn(0,0);
    if (hasCloseButton)    {
        startBtn = ImVec2(bb.Max.x-extraWidthForBtn+btnSpacingX*0.5f,bb.Min.y+(size.y-btnWidth)*0.5f);
        endBtn = ImVec2(startBtn.x+btnWidth,startBtn.y+btnWidth);
        if (!isFakeControl) {
            btnHovered = hovered && ImGui::IsMouseHoveringRect(startBtn,endBtn);
            btnPressed = pressed && btnHovered;
            if (btnPressed) pressed = false;
            if (pCloseButtonHovered) *pCloseButtonHovered = btnHovered;
            if (pCloseButtonPressedOut) * pCloseButtonPressedOut = btnPressed;
        }
    }
    if (pHoveredOut) *pHoveredOut = hovered && !btnHovered;  // We may choose not to return "hovered" when the close btn is hovered.
    if (forceActiveColorLook) {hovered = held = true;}

    // Render

    const ImU32 col = (hovered && !btnHovered && held) ? tabStyle.colors[selected ? TabLabelStyle::Col_TabLabelSelectedActive : TabLabelStyle::Col_TabLabelActive] : (hovered) ? tabStyle.colors[selected ? TabLabelStyle::Col_TabLabelSelectedHovered : TabLabelStyle::Col_TabLabelHovered] : tabStyle.colors[selected ? TabLabelStyle::Col_TabLabelSelected : TabLabelStyle::Col_TabLabel];
    const ImU32 colText = tabStyle.colors[selected ? TabLabelStyle::Col_TabLabelSelectedText : TabLabelStyle::Col_TabLabelText];

    if (!drawListOverride) drawListOverride = window->DrawList;

    // Canvas
    DrawListHelper::ImDrawListAddRectWithVerticalGradient(drawListOverride,bb.Min, bb.Max,col,(selected || hovered || held)?tabStyle.fillColorGradientDeltaIn0_05:(-tabStyle.fillColorGradientDeltaIn0_05),tabStyle.colors[selected ? TabLabelStyle::Col_TabLabelSelectedBorder : TabLabelStyle::Col_TabLabelBorder],tabStyle.rounding,1|2,tabStyle.borderWidth);

    // Text
    ImGui::PushStyleColor(ImGuiCol_Text,ImGui::ColorConvertU32ToFloat4(colText));
    if (!pOptionalJustDrawTabButtonGraphicsUnderMouseWithThisOffset)  RenderTextClipped(bb.Min,ImVec2(bb.Max.x-extraWidthForBtn,bb.Max.y), label, NULL, &label_size, ImVec2(0.5f,0.5f));
    else    {
        ImVec2 textPos(bb.Min.x+(bb.Max.x-bb.Min.x-label_size.x-extraWidthForBtn)*0.5f,bb.Min.y+(bb.Max.y-bb.Min.y-label_size.y)*0.5f);
        drawListOverride->AddText(textPos,colText,label);
    }
    ImGui::PopStyleColor();



    //fprintf(stderr,"bb.Min=%d,%d bb.Max=%d,%d label_size=%d,%d extraWidthForBtn=%d\n",(int)bb.Min.x,(int)bb.Min.y,(int)bb.Max.x,(int)bb.Max.y,(int)label_size.x,(int)label_size.y,(int)extraWidthForBtn);
    if (hasCloseButton && (hovered || selected)) {
    const ImU32 col = (held && btnHovered) ? tabStyle.colors[TabLabelStyle::Col_TabLabelCloseButtonActive] : btnHovered ? tabStyle.colors[TabLabelStyle::Col_TabLabelCloseButtonHovered] : 0;
    if (btnHovered) DrawListHelper::ImDrawListAddRect(drawListOverride,startBtn, endBtn, col,tabStyle.colors[TabLabelStyle::Col_TabLabelCloseButtonBorder],tabStyle.closeButtonRounding,0x0F,tabStyle.closeButtonBorderWidth);

        const float cross_extent = (btnWidth * 0.5f * 0.7071f);// - 1.0f;
        const ImVec2 center((startBtn.x+endBtn.x)*0.5f,(startBtn.y+endBtn.y)*0.5f);
        const ImU32 cross_col = tabStyle.colors[(btnHovered) ? TabLabelStyle::Col_TabLabelCloseButtonTextHovered : selected ? TabLabelStyle::Col_TabLabelSelectedText : TabLabelStyle::Col_TabLabelText];//btnHovered ? 0xFFFF0000 : ImGui::GetColorU32(ImGuiCol_Text);
        drawListOverride->AddLine(center + ImVec2(+cross_extent,+cross_extent), center + ImVec2(-cross_extent,-cross_extent), cross_col,tabStyle.closeButtonTextWidth);
        drawListOverride->AddLine(center + ImVec2(+cross_extent,-cross_extent), center + ImVec2(-cross_extent,+cross_extent), cross_col,tabStyle.closeButtonTextWidth);

    }
    if (fontOverride) ImGui::PopFont();

    return pressed;
}
//========================================================================================
// Main code starts here


void TabWindow::TabLabel::DestroyTabLabel(TabWindow::TabLabel*& tab)  {
    if (TabWindow::TabLabelDeletingCb) TabWindow::TabLabelDeletingCb(tab);
    tab->~TabLabel();
    ImGui::MemFree(tab);
    tab=NULL;
}

struct TabWindowNode  {
    friend class TabLabel;
    friend class TabWindow;
    friend struct TabWindowDragData;

    ImVector<TabWindow::TabLabel* > tabs;   // only in leaf nodes
    TabWindow::TabLabel* selectedTab;
    TabWindowNode *parent;   // (reference)
    TabWindowNode *child[2];  // (owned)
    char* name;         // (owned)
    float splitterPerc; // in [0,1]
    bool horizontal;
    ImVec2 allTabsSize;
    int  numClosableTabs;
    TabWindowNode() {tabs.clear();selectedTab=NULL;parent=NULL;for (int i=0;i<2;i++) child[i]=NULL;name=NULL;
           horizontal=false;splitterPerc=0.5f;allTabsSize.x=allTabsSize.y=0.f;numClosableTabs=0;}
    ~TabWindowNode() {
        clear();
        if (name) {ImGui::MemFree(name);name=NULL;}
    }
    inline bool isLeafNode() const {return (!child[0] && !child[1]);}
    void clear()  {
        for (int i=0;i<2;i++) {
            TabWindowNode*& ch = child[i];
            if (ch) {
                ch->clear();  // delete child nodes too
                ch->~TabWindowNode();
                ImGui::MemFree(ch);
                ch=NULL;
            }
        }
        for (int i=0,isz=tabs.size();i<isz;i++) {
            TabWindow::TabLabel*& tab = tabs[i];
            TabWindow::TabLabel::DestroyTabLabel(tab);
        }
        tabs.clear();
    }
    TabWindowNode *addTabLabel(TabWindow::TabLabel *tab, int childPosLTRB=-1, int pos=-1)     {
        IM_ASSERT(tab);
        IM_ASSERT(this->isLeafNode());
        IM_ASSERT(!parent || (!parent->isLeafNode() && parent->child[0] && parent->child[1] && parent->tabs.size()==0));
        if (childPosLTRB==-1)   {
            if (pos<0 || pos>tabs.size()) pos=tabs.size();
            tabs.push_back(tab);
            for (int i=tabs.size()-2;i>=pos;--i) tabs[i+1] = tabs[i];
            tabs[pos] = tab;
            return this;
        }
        IM_ASSERT(childPosLTRB>=0 && childPosLTRB<4);
        horizontal = (childPosLTRB==1 || childPosLTRB==3);
        splitterPerc = 0.5f;
        const bool spFirst = (childPosLTRB==0 || childPosLTRB==1);
        // create the two child nodes
        for (int i=0;i<2;i++)   {
            TabWindowNode* ch = child[i];
            ch = (TabWindowNode*) ImGui::MemAlloc(sizeof(TabWindowNode));
            IM_PLACEMENT_NEW(ch) TabWindowNode();
            child[i] = ch;
            ch->parent = this;
        }
        assignChildNames(false);

        // We must move tabs to child[]:
        TabWindowNode* ch = spFirst ? child[1] : child[0];
        ch->tabs.resize(tabs.size());
        for (int i=0,isz=tabs.size();i<isz;i++) {
            TabWindow::TabLabel* tab = tabs[i];
            ch->tabs[i] = tab;
        }
        tabs.clear();
        ch->selectedTab = selectedTab;
        selectedTab = NULL;
        // We must insert tab
        ch = spFirst ? child[0] : child[1];
        ch->selectedTab = tab;
        return ch->addTabLabel(tab,-1,pos);
    }
    TabWindowNode* findTabLabel(TabWindow::TabLabel* tab,bool recursive=false)  {
        if (!tab) return NULL;
        if (recursive) {
            TabWindowNode * n = NULL;
            for (int i=0;i<2;i++)
                if (child[i] && (n=child[i]->findTabLabel(tab,true))) return n;
        }
        for (int i=0,isz=tabs.size();i<isz;i++)
            if (tabs[i]==tab) return this;
        return NULL;
    }
    TabWindow::TabLabel* findTabLabelFromUserPtr(void* value,TabWindowNode** pOptionalParentNodeOut=NULL)  {
        TabWindow::TabLabel* tab = NULL;
        for (int i=0;i<2;i++)
            if (child[i] && (tab=child[i]->findTabLabelFromUserPtr(value,pOptionalParentNodeOut))!=NULL) return tab;
        for (int i=0,isz=tabs.size();i<isz;i++)
            if (tabs[i]->userPtr==value) {
                if (pOptionalParentNodeOut) *pOptionalParentNodeOut = this;
                return tabs[i];
            }
        return NULL;
    }
    TabWindow::TabLabel* findTabLabelFromUserText(const char* value,TabWindowNode** pOptionalParentNodeOut=NULL)  {
        TabWindow::TabLabel* tab = NULL;
        for (int i=0;i<2;i++)
            if (child[i] && (tab=child[i]->findTabLabelFromUserText(value,pOptionalParentNodeOut))!=NULL) return tab;
        for (int i=0,isz=tabs.size();i<isz;i++)
            if (tabs[i]->userText && strcmp(tabs[i]->userText,value)==0) {
                if (pOptionalParentNodeOut) *pOptionalParentNodeOut = this;
                return tabs[i];
            }
        return NULL;
    }
    TabWindow::TabLabel* findTabLabelFromTooltip(const char* value,TabWindowNode** pOptionalParentNodeOut=NULL)  {
        TabWindow::TabLabel* tab = NULL;
        for (int i=0;i<2;i++)
            if (child[i] && (tab=child[i]->findTabLabelFromTooltip(value,pOptionalParentNodeOut))!=NULL) return tab;
        for (int i=0,isz=tabs.size();i<isz;i++)
            if (strcmp(tabs[i]->tooltip,value)==0) {
                if (pOptionalParentNodeOut) *pOptionalParentNodeOut = this;
                return tabs[i];
            }
        return NULL;
    }
    TabWindow::TabLabel* findTabLabelFromLabel(const char* value,TabWindowNode** pOptionalParentNodeOut=NULL)  {
        TabWindow::TabLabel* tab = NULL;
        for (int i=0;i<2;i++)
            if (child[i] && (tab=child[i]->findTabLabelFromLabel(value,pOptionalParentNodeOut))!=NULL) return tab;
        for (int i=0,isz=tabs.size();i<isz;i++)
            if (tabs[i]->matchLabel(value)) {
                if (pOptionalParentNodeOut) *pOptionalParentNodeOut = this;
                return tabs[i];
            }
        return NULL;
    }
    bool removeTabLabel(TabWindow::TabLabel* tab,bool recursive=false,TabWindowNode** pOptionalActiveTabNodeToChange=NULL,bool dontDeleteTabLabel=false)  {
        if (!tab) return false;
        if (recursive) {
            for (int i=0;i<2;i++)   {
                if (child[i] && child[i]->removeTabLabel(tab,true,pOptionalActiveTabNodeToChange,dontDeleteTabLabel)) return true;
            }
        }
        IM_ASSERT(tab);
        IM_ASSERT(tabs.size()>0 ? this->isLeafNode() : true);
        for (int i=0;i<tabs.size();i++) {
            if (tabs[i]==tab) {
                if (selectedTab == tab) selectedTab = NULL;
                if (!dontDeleteTabLabel) TabWindow::TabLabel::DestroyTabLabel(tabs[i]);
                for (int j=i;j<tabs.size()-1;j++) tabs[j] = tabs[j+1];
                tabs.pop_back();
                if (tabs.size()==0 && parent) {
                    // We must merge this with parent
                    TabWindowNode* parent = this->parent;
                    IM_ASSERT(parent->child[0] && parent->child[1]);
                    IM_ASSERT(parent->child[0]==this || parent->child[1]==this);
                    IM_ASSERT(parent->child[0]!=parent->child[1]);

                    int id = parent->child[0]==this ? 0 : 1;
                    // delete parent->child[id]: it's empty (Hey! that's me! Am I allowed delete myself?)
                    {
                        TabWindowNode* ch = parent->child[id];
                        IM_ASSERT(ch==this);
                        IM_ASSERT(ch->isLeafNode());
                        parent->child[id] = NULL;
                        if (pOptionalActiveTabNodeToChange && *pOptionalActiveTabNodeToChange==ch) *pOptionalActiveTabNodeToChange=parent;
                        IM_ASSERT(ch->tabs.size()==0);
                        // We defer deleting it at the bottom of this method for extended safety
                    }
                    // merge the other child with parent
                    id = (id == 1) ? 0 : 1;// other parent child
                    {
                        TabWindowNode* ch = parent->child[id];
                        if (ch->isLeafNode())   {
                            if (pOptionalActiveTabNodeToChange && *pOptionalActiveTabNodeToChange==ch) *pOptionalActiveTabNodeToChange=parent;
                            IM_ASSERT(parent->tabs.size()==0);
                            parent->tabs.resize(ch->tabs.size());
                            for (int i=0,isz=ch->tabs.size();i<isz;i++) {
                                parent->tabs[i] = ch->tabs[i];
                            }
                            ch->tabs.clear();
                            parent->selectedTab = ch->selectedTab;
                            parent->splitterPerc = 0.5f;

                            parent->child[id] = NULL;
                        }
                        else {
                            IM_ASSERT(ch->tabs.size()==0);
                            IM_ASSERT(parent->tabs.size()==0);

                            // We must replace "parent" with "ch" and then delete "parent"
                            // Nope: it's better to "deep clone "ch" to "parent" and delete "ch"

                            if (pOptionalActiveTabNodeToChange && *pOptionalActiveTabNodeToChange==ch) *pOptionalActiveTabNodeToChange=parent;

                            if (ch->name) {ImGui::MemFree(ch->name);ch->name=NULL;}
                            parent->child[0] = ch->child[0];ch->child[0]=NULL;
                            parent->child[1] = ch->child[1];ch->child[1]=NULL;ch->parent=NULL;
                            parent->child[0]->parent = parent->child[1]->parent = parent;
                            parent->horizontal = ch->horizontal;
                            parent->selectedTab = ch->selectedTab;
                            parent->splitterPerc = ch->splitterPerc;
                            parent->assignChildNames(true);

                        }

                        // delete the other child
                        ch->~TabWindowNode();
                        ImGui::MemFree(ch);
                        ch = NULL;
                        // delete me
                        ch = this;
                        ch->~TabWindowNode();
                        ImGui::MemFree(ch);
                    }



                }
                return true;
            }
        }
        return false;
    }
    bool isEmpty(bool recursive=false) {
        if (tabs.size()!=0) return false;
        if (recursive) {
            for (int i=0;i<2;i++)
                if (child[i] && !child[i]->isEmpty(true)) return false;
        }
        return true;
    }
    TabWindowNode* getFirstLeaftNode() {return isLeafNode() ? this : child[0]->getFirstLeaftNode();}
    TabWindowNode* getRootNode() {return parent ? parent->getRootNode() : this;}
    bool mergeToParent(TabWindowNode** pOptionalActiveTabNodeToChange=NULL) {
	if (!isLeafNode() || !parent) return false;
	ImVector<TabWindow::TabLabel*> nodetabs;int sz=0;
	TabWindowNode* parent = this->parent;
	while ((sz=tabs.size())>0)   {
	    TabWindow::TabLabel* tab = tabs[0];
	    nodetabs.push_back(tab);
	    removeTabLabel(tab,false,pOptionalActiveTabNodeToChange,true);
	    if (sz==1) break;
	}
	// "this" it's invalid now: after having removed all its TabLabels the node we're in has been deleted...
	parent = parent->getFirstLeaftNode();
	for (int i=0,isz = nodetabs.size();i<isz;i++)   {
	    parent->addTabLabel(nodetabs[i]);
	}
	return true;
    }
    void mergeEmptyLeafNodes(TabWindowNode** pOptionalActiveTabNodeToChange=NULL)   {
	for (int i=0;i<2;i++) {
	    if (child[i])  child[i]->mergeEmptyLeafNodes(pOptionalActiveTabNodeToChange);
	}
	if (isLeafNode() && tabs.size()==0) mergeToParent(pOptionalActiveTabNodeToChange);
    }

    void setName(const char* lbl)  {
        if (name) {ImGui::MemFree(name);name=NULL;}
        const char e = '\0';if (!lbl) lbl=&e;
        const int sz = strlen(lbl)+1;
        name = (char*) ImGui::MemAlloc(sz+1);strcpy(name,lbl);
    }
    void assignChildNames(bool recursive=false)  {
        const int sz = strlen(name)+8;
        for (int i=0;i<2;i++) {
            TabWindowNode* ch = child[i];
            if (!ch) continue;
            if (ch->name) {ImGui::MemFree(ch->name);ch->name=NULL;}
            ch->name = (char*) ImGui::MemAlloc(sz);
            strcpy(ch->name,name);
            strcat(ch->name,".child");
            sprintf(&ch->name[sz-2],"%d",i);
            ch->name[sz-1]='\0';
            if (recursive) ch->assignChildNames(true);
        }
    }
    void getTabLabels(ImVector<TabWindow::TabLabel*>& tabsOut,bool onlyClosableTabs=false,bool onlyModifiedTabs=false)	{
        for (int i=0;i<2;i++)   {
            if (child[i]) child[i]->getTabLabels(tabsOut,onlyClosableTabs,onlyModifiedTabs);
        }
        IM_ASSERT(tabs.size()>0 ? this->isLeafNode() : true);
        for (int i=0,isz=tabs.size();i<isz;i++) {
            TabWindow::TabLabel* tab = tabs[i];
            if ((!onlyClosableTabs || tab->isClosable()) && (!onlyModifiedTabs || tab->getModified())) {
                tabsOut.push_back(tab);
                //fprintf(stderr,"%s\n",tab->getLabel());
            }
        }
    }

    void render(const ImVec2& windowSize,struct MyTabWindowHelperStruct *ptr);

#if (defined(IMGUIHELPER_H_) && !defined(NO_IMGUIHELPER_SERIALIZATION))
#ifndef NO_IMGUIHELPER_SERIALIZATION_SAVE
    void serialize(ImGuiHelper::Serializer& s,TabWindow* tabWindow) {
        if (name) s.save(name,"name");
        s.save(&splitterPerc,"splitterPerc");
        s.save(&horizontal,"horizontal");
        if (tabWindow->activeNode==this) {bool a = true;s.save(&a,"isActiveNode");}
        const bool isLeafNode = this->isLeafNode();
        s.save(&isLeafNode,"isLeafNode");
        const int tabSize = tabs.size();s.save(&tabSize,"numTabs");
        IM_ASSERT(tabSize>0 ? isLeafNode : true);
        if (!isLeafNode) {
            for (int i=0;i<2;i++)   child[i]->serialize(s,tabWindow);
        }
        else {
            for (int i=0;i<tabSize;i++) {
                TabWindow::TabLabel& tl = *tabs[i];
                if (tl.getLabel()) s.save(tl.getLabel(),"label",(tl.getModified() && strlen(tl.getLabel())>0) ? (strlen(tl.getLabel())-1) : -1);
                if (tl.tooltip && strlen(tl.tooltip)>0) s.save(tl.getTooltip(),"tooltip");
                s.save(&tl.closable,"closable");
                s.save(&tl.draggable,"draggable");
                if (selectedTab==&tl) {bool a = true;s.save(&a,"selected");}
                if (tl.userText && strlen(tl.userText)>0) s.save(tl.getUserText(),"userText");
                s.save(&tl.userInt,"userInt");
                s.save(&tl.wndFlags,"wndFlags");
            }
        }
    }
#endif //NO_IMGUIHELPER_SERIALIZATION_SAVE
#ifndef NO_IMGUIHELPER_SERIALIZATION_LOAD
    struct ParseCallbackStruct {
        TabWindowNode* node;
        bool isLeafNode;
        int numTabs;
        bool isActiveNode;
    };
    static bool ParseCallback(ImGuiHelper::FieldType /*ft*/,int /*numArrayElements*/,void* pValue,const char* name,void* userPtr)    {
        ParseCallbackStruct& cbs = *((ParseCallbackStruct*)userPtr);
        TabWindowNode* n = cbs.node;
        if (strcmp(name,"name")==0) {
            n->setName((const char*)pValue);
            //fprintf(stderr,"\"%s\"\n",n->name);
        }
        else if (strcmp(name,"splitterPerc")==0) n->splitterPerc = *((float*)pValue);
        else if (strcmp(name,"horizontal")==0) n->horizontal = *((bool*)pValue);
        else if (strcmp(name,"isActiveNode")==0) cbs.isActiveNode = *((bool*)pValue);
        else if (strcmp(name,"isLeafNode")==0) cbs.isLeafNode = *((bool*)pValue);
        else if (strcmp(name,"numTabs")==0) {cbs.numTabs = *((int*)pValue);return true;}
        return false;
    }
    struct ParseTabLabelCallbackStruct {
        TabWindow::TabLabel* tab;
        bool isSelected;
    };
    static bool ParseTabLabelCallback(ImGuiHelper::FieldType /*ft*/,int /*numArrayElements*/,void* pValue,const char* name,void* userPtr)    {
        ParseTabLabelCallbackStruct& tls = *((ParseTabLabelCallbackStruct*)userPtr);
        TabWindow::TabLabel& tab = *tls.tab;
        if (strcmp(name,"label")==0) {
            tab.setLabel((const char*)pValue);
            //fprintf(stderr,"\"%s\"\n",tab.label);
        }
        else if (strcmp(name,"tooltip")==0) tab.setTooltip((const char*)pValue);
        else if (strcmp(name,"closable")==0) tab.closable = *((bool*)pValue);
        else if (strcmp(name,"draggable")==0) tab.draggable = *((bool*)pValue);
        else if (strcmp(name,"selected")==0) tls.isSelected = *((bool*)pValue);
        else if (strcmp(name,"userText")==0) tab.setUserText((const char*)pValue);
        else if (strcmp(name,"userInt")==0) tab.userInt = *((int*)pValue);
        else if (strcmp(name,"wndFlags")==0) {tab.wndFlags = *((int*)pValue);return true;}
        return false;
    }
    void deserialize(ImGuiHelper::Deserializer& d,TabWindowNode* parent,const char*& amount,TabWindow* tabWindow)  {
        ParseCallbackStruct cbs;cbs.node=this;cbs.isLeafNode=true;cbs.numTabs=0;cbs.isActiveNode=false;
        amount = d.parse(ParseCallback,(void*)&cbs,amount);
        this->parent = parent;
        if (cbs.isActiveNode && tabWindow) {
            tabWindow->activeNode = this;
            //IM_ASSERT(cbs.isLeafNode);    // mmmh, this gets hit sometimes...
        }
        IM_ASSERT(cbs.numTabs>0 ? cbs.isLeafNode : true);
        if (!cbs.isLeafNode) {
            for (int i=0;i<2;i++)   {
                TabWindowNode* n = (TabWindowNode*) ImGui::MemAlloc(sizeof(TabWindowNode));
                IM_PLACEMENT_NEW(n) TabWindowNode();
                n->deserialize(d,this,amount,tabWindow);
                this->child[i] = n;
            }
        }
        else {
            for (int i=0;i<cbs.numTabs;i++) {
                TabWindow::TabLabel tl;
                ParseTabLabelCallbackStruct tls;tls.tab=&tl;tls.isSelected = false;
                amount = d.parse(ParseTabLabelCallback,(void*)&tls,amount);
                TabWindow::TabLabel* tab = tabWindow->createTabLabel(tl.getLabel(),tl.getTooltip(),tl.isClosable(),tl.isDraggable(),NULL,tl.userText,tl.userInt,tl.wndFlags);
                if (tab) {
                    this->tabs.push_back(tab);
                    if (tls.isSelected) this->selectedTab = tab;
                }
            }
        }
        IM_ASSERT(this->tabs.size()>0 ? this->isLeafNode() : true);
        IM_ASSERT(!this->isLeafNode() ? this->tabs.size()==0 : true);
    }
#endif //NO_IMGUIHELPER_SERIALIZATION_LOAD
#endif //NO_IMGUIHELPER_SERIALIZATION

};

namespace TabWindowDefaultCallbacks {
void TabLabelGroupPopupMenuProvider(ImVector<ImGui::TabWindow::TabLabel*>& tabs,ImGui::TabWindow& parent,ImGui::TabWindowNode* tabNode,void*) {
    const int numTabsToClose = parent.getNumClosableTabs(tabNode);  // or parent.getNumTabs(tabNode)
    const bool isMergeble = parent.isMergeble(tabNode);
    if (numTabsToClose || isMergeble)  {
        ImGui::PushStyleColor(ImGuiCol_WindowBg,ImGui::ColorConvertU32ToFloat4(ImGui::TabLabelStyle::Get().colors[ImGui::TabLabelStyle::Col_TabLabel]));
        ImGui::PushStyleColor(ImGuiCol_Text,ImGui::ColorConvertU32ToFloat4(ImGui::TabLabelStyle::Get().colors[ImGui::TabLabelStyle::Col_TabLabelText]));
        if (ImGui::BeginPopup(ImGui::TabWindow::GetTabLabelGroupPopupMenuName()))   {
            //ImGui::Text("TabLabel Group Menu");
            //ImGui::Separator();
            if (isMergeble && ImGui::MenuItem("Merge")) parent.merge(tabNode); // Warning: this invalidates "tabNode" after the call
            if (numTabsToClose && ImGui::MenuItem("Close all")) {
                for (int i=0,isz=tabs.size();i<isz;i++) {
                    ImGui::TabWindow::TabLabel* tab = tabs[i];
                    if (tab->isClosable())  // otherwise even non-closable tabs will be closed [skip if using parent.getNumTabs(tabNode) above]
                    {
			//parent.removeTabLabel(tab);
			tab->mustCloseNextFrame = true;  // alternative way... this asks for closing
                    }
                }
            }
            ImGui::EndPopup();
        }
        ImGui::PopStyleColor(2);
    }
}
void TabLabelPopupMenuProvider(ImGui::TabWindow::TabLabel* tab,ImGui::TabWindow& parent,void*) {
    const bool savable = tab && /*tab->isClosable() &&*/ tab->getModified();
    if (savable && ImGui::BeginPopup(ImGui::TabWindow::GetTabLabelPopupMenuName()))   {
        ImGui::PushID(tab);
        //ImGui::Text("\"%.*s\" Menu",(int)(strlen(tab->getLabel())-(tab->getModified()?1:0)),tab->getLabel());
        //ImGui::Separator();
        if (ImGui::MenuItem("Save")) {
            bool ok = false;
            if (TabWindow::TabLabelSaveCb) ok = TabWindow::TabLabelSaveCb(tab,parent,NULL);
            else ok = tab->saveAs(NULL);
            if (ok) tab->setModified(false);
        }
        //if (tab->closable && ImGui::MenuItem("Close")) tab->mustCloseNextFrame = true;
        ImGui::PopID();
        ImGui::EndPopup();
    }

}

}   // TabWindowDefaultCallbacks

struct TabWindowDragData {
    TabWindow::TabLabel* draggingTabSrc;
    TabWindowNode* draggingTabNodeSrc;
    ImGuiWindow* draggingTabImGuiWindowSrc;
    TabWindow* draggingTabWindowSrc;
    ImVec2 draggingTabSrcSize;
    ImVec2 draggingTabSrcOffset;
    bool draggingTabSrcIsSelected;

    TabWindow::TabLabel* draggingTabDst;
    TabWindowNode* draggingTabNodeDst;
    ImGuiWindow* draggingTabImGuiWindowDst;
    TabWindow* draggingTabWindowDst;

    TabWindowDragData() {reset();}
    void resetDraggingSrc() {
        draggingTabSrc = NULL;
        draggingTabNodeSrc = NULL;
        draggingTabImGuiWindowSrc = NULL;
        draggingTabWindowSrc = NULL;
        draggingTabSrcSize = draggingTabSrcOffset = ImVec2(0,0);
        draggingTabSrcIsSelected = false;
    }
    void resetDraggingDst() {
        draggingTabDst = NULL;
        draggingTabNodeDst = NULL;
        draggingTabImGuiWindowDst = NULL;
        draggingTabWindowDst = NULL;
    }
    inline void reset() {resetDraggingSrc();resetDraggingDst();}
    inline bool isDraggingSrcValid() const {
        return (draggingTabSrc && draggingTabNodeSrc && draggingTabImGuiWindowSrc);
    }
    inline bool isDraggingDstValid() const {
        return (draggingTabDst && draggingTabNodeDst && draggingTabImGuiWindowDst);
    }
    inline int findDraggingSrcIndex(const TabWindow::TabLabel* tab=NULL) const {
        if (!tab) tab = draggingTabSrc;
        for (int i=0,isz=draggingTabNodeSrc->tabs.size();i<isz;i++) {
            if (draggingTabNodeSrc->tabs[i] == tab) return i;
        }
        return -1;
    }
    inline int findDraggingDstIndex(const TabWindow::TabLabel* tab=NULL) const {
        if (!tab) tab = draggingTabDst;
        for (int i=0,isz=draggingTabNodeDst->tabs.size();i<isz;i++) {
            if (draggingTabNodeDst->tabs[i] == tab) return i;
        }
        return -1;
    }
    inline static TabWindowNode* FindTabNodeByName(TabWindowNode* firstNode,const char* name,int numCharsToMatch=-1) {
        if ((numCharsToMatch==-1 && strcmp(firstNode->name,name)==0)
            || (strncmp(firstNode->name,name,numCharsToMatch)==0)) return firstNode;
        TabWindowNode* rv = NULL;
        for (int i=0;i<2;i++)   {
            TabWindowNode* ch = firstNode->child[i];
            if (ch && (rv=FindTabNodeByName(ch,name,numCharsToMatch))) return rv;
        }
        return NULL;
    }

    inline void drawDragButton(ImDrawList* drawList,const ImVec2& wp,const ImVec2& mp)   {
        const TabLabelStyle& tabStyle = TabLabelStyleGetMergedWithAlphaForOverlayUsage();
        ImVec2 start(wp.x+mp.x-draggingTabSrcOffset.x-draggingTabSrcSize.x*0.5f,wp.y+mp.y-draggingTabSrcOffset.y-draggingTabSrcSize.y*0.5f);
        bool mustCloseTab = false;
        const ImFont* fontOverride = (draggingTabSrcIsSelected ? (draggingTabSrc->getModified() ? TabLabelStyle::ImGuiFonts[tabStyle.fontStyles[TabLabelStyle::TAB_STATE_SELECTED_MODIFIED]] : TabLabelStyle::ImGuiFonts[tabStyle.fontStyles[TabLabelStyle::TAB_STATE_SELECTED]]) :
        (draggingTabSrc->getModified() ? TabLabelStyle::ImGuiFonts[tabStyle.fontStyles[TabLabelStyle::TAB_STATE_MODIFIED]] : TabLabelStyle::ImGuiFonts[tabStyle.fontStyles[TabLabelStyle::TAB_STATE_NORMAL]]));
        if (!fontOverride) {
            if (draggingTabSrcIsSelected) fontOverride = TabLabelStyle::ImGuiFonts[tabStyle.fontStyles[TabLabelStyle::TAB_STATE_SELECTED]];
            else if (draggingTabSrc->getModified())  fontOverride = TabLabelStyle::ImGuiFonts[tabStyle.fontStyles[TabLabelStyle::TAB_STATE_MODIFIED]];
        }
        ImGui::TabButton(NULL,draggingTabSrcIsSelected,draggingTabSrc->closable ? &mustCloseTab : NULL,draggingTabSrc->getLabel(),NULL,&tabStyle,(ImFont*)fontOverride,&start,drawList,false,true);
    }
    inline void drawProhibitionSign(ImDrawList* drawList,const ImVec2& wp,const ImVec2& pos,float size,float alpha=0.5f)   {
        ImVec2 start(wp.x+pos.x-size*0.5f,wp.y+pos.y-size*0.5f);
        const ImVec2 end(start.x+size,start.y+size);
        const ImVec4 color(1.f,1.f,1.f,alpha);
        drawList->AddImage(TabWindow::DockPanelIconTextureID,start,end,ImVec2(0.5f,0.75f),ImVec2(0.75f,1.f),ImGui::ColorConvertFloat4ToU32(color));
   }

};
static TabWindowDragData gDragData;
struct MyTabWindowHelperStruct {
    bool isRMBclicked;
    static bool isMouseDragging;
    static bool isMouseDraggingJustStarted;
    static bool LockedDragging; // better dragging experience when trying to drag non-draggable tab labels
    bool isASplitterActive;
    static TabWindow::TabLabel* tabLabelPopup;
    static bool tabLabelPopupChanged;
    static TabWindow* tabLabelPopupTabWindow;   // used by both tabLabelPopup and tabLabelGroupPopup
    static ImVector<TabWindow::TabLabel*> tabLabelGroupPopup;
    static bool tabLabelGroupPopupChanged;
    static TabWindowNode* tabLabelGroupPopupNode;

    TabWindow* tabWindow;
    bool allowExchangeTabLabels;

    static ImVector<TabWindow::TabLabel*> TabsToClose;
    static ImVector<TabWindow*> TabsToCloseParents;

    static ImVector<TabWindow::TabLabel*> TabsToAskForClosing;
    static ImVector<TabWindow*> TabsToAskForClosingParents;
    static bool TabsToAskForClosingIsUsedJustToSaveTheseTabs;
    static bool TabsToAskForClosingDontAllowCancel;
    static bool MustOpenAskForClosingPopup;


    ImVec2 itemSpacing;
    float childBorderSize;

    ImVec4 splitterColor;
    ImVec4 splitterColorHovered;
    ImVec4 splitterColorActive;

    float textHeightWithSpacing;
    bool isWindowHovered;

    ImGuiWindowFlags flags;

    MyTabWindowHelperStruct(TabWindow* _tabWindow) {
        isMouseDragging = ImGui::IsMouseDragging(0,3.f);
        isMouseDraggingJustStarted = isMouseDragging && (ImGui::GetIO().MouseDownDuration[0] < 0.35f);// ImGui::GetIO().MouseDown[0] does not work!
        isRMBclicked = ImGui::IsMouseClicked(1);
        isASplitterActive = false;
        tabWindow = _tabWindow;
        allowExchangeTabLabels = !gDragData.draggingTabSrc || (gDragData.draggingTabWindowSrc && gDragData.draggingTabWindowSrc->canExchangeTabLabelsWith(tabWindow));
        //mustOpenAskForClosingPopup = false;

        ImGuiStyle& style = ImGui::GetStyle();
        itemSpacing =   style.ItemSpacing;
        childBorderSize = style.ChildBorderSize;

        const TabLabelStyle& ts = TabLabelStyle::Get();
        splitterColor           = ts.tabWindowSplitterColor;  splitterColor.w *= 0.4f;
        splitterColorHovered    = ts.tabWindowSplitterColor;  splitterColorHovered.w *= 0.55f;
        splitterColorActive     = ts.tabWindowSplitterColor;  splitterColorActive.w *= 0.7f;

        storeStyleVars();

        textHeightWithSpacing = ImGui::GetTextLineHeightWithSpacing();

        isWindowHovered = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

        flags = TabWindow::ExtraWindowFlags;// | ((ImGui::GetCurrentWindow()->Flags&ImGuiWindowFlags_ShowBorders) ? ImGuiWindowFlags_ShowBorders : 0);
    }
    ~MyTabWindowHelperStruct() {
	restoreStyleVars();
    /*if (mustOpenAskForClosingPopup) {
        ImGuiContext& g = *GImGui; while (g.OpenPopupStack.size() > 0) g.OpenPopupStack.pop_back();   // Close all existing context-menus
	    ImGui::OpenPopup(ImGui::TabWindow::GetTabLabelAskForDeletionModalWindowName());
    }*/
    }
    inline void storeStyleVars() {
        ImGuiStyle& style = ImGui::GetStyle();
        style.ItemSpacing = ImVec2(1,1);
        style.ChildBorderSize = 0.f;
    }
    inline void restoreStyleVars() {
        ImGuiStyle& style = ImGui::GetStyle();
        style.ItemSpacing = itemSpacing;
        style.ChildBorderSize = childBorderSize;
    }

    inline static void ResetTabsToClose() {
	TabsToClose.clear();TabsToCloseParents.clear();
    }
    inline static void ResetTabsToAskForClosing() {
	TabsToAskForClosing.clear();TabsToAskForClosingParents.clear();
    TabsToAskForClosingIsUsedJustToSaveTheseTabs=false;
    TabsToAskForClosingDontAllowCancel=false;
    }
};
TabWindow::TabLabel* MyTabWindowHelperStruct::tabLabelPopup = NULL;
ImVector<TabWindow::TabLabel*> MyTabWindowHelperStruct::tabLabelGroupPopup;
TabWindow* MyTabWindowHelperStruct::tabLabelPopupTabWindow = NULL;
bool  MyTabWindowHelperStruct::tabLabelPopupChanged = false;
bool  MyTabWindowHelperStruct::tabLabelGroupPopupChanged = false;
TabWindowNode*  MyTabWindowHelperStruct::tabLabelGroupPopupNode = NULL;
bool MyTabWindowHelperStruct::isMouseDragging = false;
bool MyTabWindowHelperStruct::isMouseDraggingJustStarted = false;
bool MyTabWindowHelperStruct::LockedDragging = false;
ImVector<TabWindow::TabLabel*> MyTabWindowHelperStruct::TabsToClose;
ImVector<TabWindow*> MyTabWindowHelperStruct::TabsToCloseParents;
ImVector<TabWindow::TabLabel*> MyTabWindowHelperStruct::TabsToAskForClosing;
ImVector<TabWindow*> MyTabWindowHelperStruct::TabsToAskForClosingParents;
bool MyTabWindowHelperStruct::TabsToAskForClosingIsUsedJustToSaveTheseTabs=false;
bool MyTabWindowHelperStruct::TabsToAskForClosingDontAllowCancel=false;
bool MyTabWindowHelperStruct::MustOpenAskForClosingPopup=false;
TabWindow::TabLabelCallback TabWindow::WindowContentDrawerCb=NULL;
void* TabWindow::WindowContentDrawerUserPtr=NULL;
TabWindow::TabLabelCallback TabWindow::TabLabelPopupMenuDrawerCb=&TabWindowDefaultCallbacks::TabLabelPopupMenuProvider;
void* TabWindow::TabLabelPopupMenuDrawerUserPtr=NULL;
//TabWindow::TabLabelClosingCallback TabWindow::TabLabelClosingCb=NULL;
//void* TabWindow::TabLabelClosingUserPtr=NULL;
TabWindow::TabLabelDeletingCallback TabWindow::TabLabelDeletingCb=NULL;
TabWindow::TabLabelGroupPopupMenuCallback TabWindow::TabLabelGroupPopupMenuDrawerCb=&TabWindowDefaultCallbacks::TabLabelGroupPopupMenuProvider;
void* TabWindow::TabLabelGroupPopupMenuDrawerUserPtr=NULL;
TabWindow::TabLabelFactoryCallback TabWindow::TabLabelFactoryCb=NULL;
TabWindow::TabLabelFileCallback TabWindow::TabLabelSaveCb=NULL;
ImGuiWindowFlags TabWindow::ExtraWindowFlags = 0;


void TabWindowNode::render(const ImVec2 &windowSize, MyTabWindowHelperStruct *ptr)
{   
    MyTabWindowHelperStruct& mhs = *ptr;
    const TabLabelStyle& tabStyle = TabLabelStyle::GetMergedWithWindowAlpha();  // Or just Get() ?
    ImGuiStyle& style = ImGui::GetStyle();
    const ImVec4 colorChildWindowBg = style.Colors[ImGuiCol_ChildBg];
    const float splitterSize = tabStyle.tabWindowSplitterSize;
    bool splitterActive = false;
    static ImVec4 colorTransparent(0,0,0,0);

    IM_ASSERT(name);
    if (child[0])   {
        IM_ASSERT(child[1]);
        IM_ASSERT(tabs.size()==0);
        const float minSplitSize = 10;  // If size is smaller, the child won't be displayed
        style.Colors[ImGuiCol_ChildBg] = colorTransparent;
        if (ImGui::RevertUpstreamBeginChildCommit::OldBeginChild(name,windowSize,ImGuiWindowFlags_NoScrollbar))   {
            style.Colors[ImGuiCol_ChildBg] = colorChildWindowBg;
            ImVec2 ws = windowSize;
            float splitterPercToPixels = 0.f,splitterDelta = 0.f;
            if (horizontal && ws.y>splitterSize && ws.x>minSplitSize) {
                ws.y-=splitterSize;
                splitterPercToPixels = ws.y*splitterPerc;
                if (splitterPercToPixels>minSplitSize) child[0]->render(ImVec2(ws.x,splitterPercToPixels),ptr);
                // Horizontal Splitter ------------------------------------------
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));
                ImGui::PushStyleColor(ImGuiCol_Button,mhs.splitterColor);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered,mhs.splitterColorHovered);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,mhs.splitterColorActive);
                ImGui::PushID(this);

                ImGui::Button("##splitter0", ImVec2(ws.x,splitterSize));
                splitterActive = !mhs.isASplitterActive && ImGui::IsItemActive();
                if (splitterActive || ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);

                mhs.isASplitterActive |= splitterActive;
                if (splitterActive)  splitterDelta = ImGui::GetIO().MouseDelta.y;
                else splitterDelta = 0.f;
                if (splitterActive)  {
                    float& h = splitterPercToPixels;
                    const float minh = splitterSize;
                    const float maxh = ws.y-splitterSize - mhs.textHeightWithSpacing;//20;   Is this correct ?       // Warning: 20.f is hard-coded!
                    if (h+splitterDelta>maxh)           splitterDelta = (h!=maxh) ? (maxh-h) : 0.f;
                    else if (h+splitterDelta<minh)      splitterDelta = (h!=minh) ? (minh-h) : 0.f;
                    h+=splitterDelta;
                    splitterPerc = splitterPercToPixels/ws.y;
                }
                ImGui::PopID();
                ImGui::PopStyleColor(3);
                ImGui::PopStyleVar();
                //------------------------------------------------------
                if (ws.y-splitterPercToPixels>minSplitSize) child[1]->render(ImVec2(ws.x,ws.y-splitterPercToPixels),ptr);
            }
            else if (!horizontal && ws.x>splitterSize && ws.y>minSplitSize) {
                ws.x-=splitterSize;
                splitterPercToPixels = ws.x*splitterPerc;
                if (splitterPercToPixels>minSplitSize) child[0]->render(ImVec2(splitterPercToPixels,ws.y),ptr);
                // Vertical Splitter ------------------------------------------
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));
                ImGui::PushStyleColor(ImGuiCol_Button,mhs.splitterColor);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered,mhs.splitterColorHovered);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,mhs.splitterColorActive);
                ImGui::PushID(this);
                ImGui::SameLine(0,0);

                ImGui::Button("##splitter1", ImVec2(splitterSize,ws.y));
                splitterActive = !mhs.isASplitterActive && ImGui::IsItemActive();
                if (splitterActive || ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

                mhs.isASplitterActive |= splitterActive;
                if (splitterActive)  splitterDelta = ImGui::GetIO().MouseDelta.x;
                else splitterDelta = 0.f;
                if (splitterActive)  {
                    float& w = splitterPercToPixels;
                    const float minw = splitterSize;
                    const float maxw = ws.x-splitterSize;
                    if (w + splitterDelta>maxw)         splitterDelta = (w!=maxw) ? (maxw-w) : 0.f;
                    else if (w + splitterDelta<minw)    splitterDelta = (w!=minw) ? (minw-w) : 0.f;
                    w+=splitterDelta;
                    splitterPerc = splitterPercToPixels/ws.x;
                }
                ImGui::SameLine(0,0);
                ImGui::PopID();
                ImGui::PopStyleColor(3);
                ImGui::PopStyleVar();
                //------------------------------------------------------
                if (ws.x-splitterPercToPixels>minSplitSize) child[1]->render(ImVec2(ws.x-splitterPercToPixels,ws.y),ptr);
            }
            //else {/* Window too tiny: better not to draw it, otherwise the splitters overlap and may cause bad stuff */}
        }
        else style.Colors[ImGuiCol_ChildBg] = colorChildWindowBg;
        ImGui::EndChild();  // name
        return;
    }

    // Leaf Node
    IM_ASSERT(!child[1]);


    //TabWindow::TabLabel* hoveredTab = NULL;
    //----------------------------------------------------------------
    {
        style.Colors[ImGuiCol_ChildBg] = colorTransparent;
        if (ImGui::RevertUpstreamBeginChildCommit::OldBeginChild(name,windowSize,ImGuiWindowFlags_NoScrollbar)) {
            if (tabStyle.tabWindowLabelBackgroundColor.w!=0)    {
                ImGuiWindow* window = ImGui::GetCurrentWindow();
                window->DrawList->AddRectFilled(window->Pos, window->Pos+ImVec2(windowSize.x,allTabsSize.y), GetColorU32(tabStyle.tabWindowLabelBackgroundColor), 0);
            }
            style.Colors[ImGuiCol_ChildBg] = colorChildWindowBg;
            ImGuiContext& g = *GImGui;
            TabWindowDragData& dd = gDragData;
            const ImFont* fontOverride = NULL;


            ImGui::Spacing();	    // Hack to remove when I'll find out why the top border of the tab labels gets clipped out.
            ImGui::BeginGroup();
            const int numTabs = tabs.size();
            if (numTabs>0 && !selectedTab) selectedTab = tabs[0];

            float windowWidth = 0.f,sumX=0.f;
            windowWidth = windowSize.x;//ImGui::GetWindowWidth();// - style.WindowPadding.x;// - (ImGui::GetScrollMaxY()>0 ? style.ScrollbarSize : 0.f);
            TabWindow::TabLabel* newSelectedTab = selectedTab;
            numClosableTabs = 0;
            ImVec2 tabButtonSz(0,0);bool mustCloseTab = false;bool canUseSizeOptimization = false;bool isAItemHovered = false;
            const bool isDraggingCorrectly = mhs.isMouseDragging && !mhs.LockedDragging && !mhs.isASplitterActive;
            bool selection_changed = false;
            for (int i = 0; i < numTabs; i++)
            {
                TabWindow::TabLabel& tab = *tabs[i];
                if (tab.closable) ++numClosableTabs;

                fontOverride = ((selectedTab == &tab) ? (tab.getModified() ? TabLabelStyle::ImGuiFonts[tabStyle.fontStyles[TabLabelStyle::TAB_STATE_SELECTED_MODIFIED]] : TabLabelStyle::ImGuiFonts[tabStyle.fontStyles[TabLabelStyle::TAB_STATE_SELECTED]]) :
                    (tab.getModified() ? TabLabelStyle::ImGuiFonts[tabStyle.fontStyles[TabLabelStyle::TAB_STATE_MODIFIED]] : TabLabelStyle::ImGuiFonts[tabStyle.fontStyles[TabLabelStyle::TAB_STATE_NORMAL]]));
                if (!fontOverride) {
                    if (selectedTab == &tab) fontOverride = TabLabelStyle::ImGuiFonts[tabStyle.fontStyles[TabLabelStyle::TAB_STATE_SELECTED]];
                    else if (tab.getModified())  fontOverride = TabLabelStyle::ImGuiFonts[tabStyle.fontStyles[TabLabelStyle::TAB_STATE_MODIFIED]];
                }

                if (sumX > 0.f) {
                    sumX+=style.ItemSpacing.x;   // Maybe we can skip it if we use SameLine(0,0) below
                    ImGui::TabButton(NULL,selectedTab == &tab,tab.closable ? &mustCloseTab : NULL,tab.getLabel(),&tabButtonSz,&tabStyle,(ImFont*)fontOverride);
                    sumX+=tabButtonSz.x;
                    if (sumX>windowWidth) sumX = 0.f;
                    else ImGui::SameLine();
                    canUseSizeOptimization = true;
                }
                else canUseSizeOptimization = false;

                // Draw the button
                mustCloseTab = false;
                ImGui::PushID(&tab);   // otherwise two tabs with the same name would clash.
                if (tab.mustSelectNextFrame || ImGui::TabButton("",(selectedTab == &tab),tab.closable ? &mustCloseTab : NULL,tab.getLabel(),NULL,&tabStyle,(ImFont*)fontOverride,NULL,NULL,canUseSizeOptimization))   {
                    selection_changed = (selectedTab != &tab);
                    newSelectedTab = &tab;
                    tab.mustSelectNextFrame = false;
                }
                ImGui::PopID();

                if (sumX==0.f) sumX = style.WindowPadding.x + ImGui::GetItemRectSize().x; // First element of a line

                if (tab.mustCloseNextFrame || mustCloseTab) {
                    tab.mustCloseNextFrame = false;
                    if (!tab.getModified())	{
                        mhs.TabsToClose.push_back(&tab);
                        mhs.TabsToCloseParents.push_back(mhs.tabWindow);
                    }
                    else {
                        mhs.TabsToAskForClosing.push_back(&tab);
                        mhs.TabsToAskForClosingParents.push_back(mhs.tabWindow);
                        mhs.MustOpenAskForClosingPopup = true;
                    }
                }
                else if (ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly)) {
                    isAItemHovered = true;
                    //hoveredTab = &tab;
                    if (tab.tooltip && mhs.isWindowHovered && strlen(tab.tooltip)>0 && (&tab!=mhs.tabLabelPopup || GImGui->OpenPopupStack.size()==0) )  ImGui::SetTooltip("%s",tab.tooltip);

					ImGuiWindow* root_window = g.HoveredWindow ? g.HoveredWindow->RootWindow : NULL;
                    if (isDraggingCorrectly) {
                        if (mhs.isMouseDraggingJustStarted)  {
                            if (!dd.draggingTabSrc) {
                                if (mhs.isWindowHovered)    {
                                    if (!tab.draggable) mhs.LockedDragging = true;
                                    else {
                                        dd.draggingTabSrc = &tab;
                                        dd.draggingTabNodeSrc = this;
                                        dd.draggingTabImGuiWindowSrc = g.HoveredWindow;
                                        dd.draggingTabWindowSrc = mhs.tabWindow;
                                        dd.draggingTabSrcIsSelected = (selectedTab == &tab);

                                        dd.draggingTabSrcSize = ImGui::GetItemRectSize();
                                        const ImVec2& mp = ImGui::GetIO().MousePos;
                                        const ImVec2 draggingTabCursorPos = ImGui::GetCursorPos();
                                        dd.draggingTabSrcOffset=ImVec2(
                                                    mp.x+dd.draggingTabSrcSize.x*0.5f-sumX+ImGui::GetScrollX(),
                                                    mp.y+dd.draggingTabSrcSize.y*0.5f-draggingTabCursorPos.y+ImGui::GetScrollY()
                                                    );

                                        //fprintf(stderr,"Hovered Start Window:%s\n",g.HoveredWindow ? g.HoveredWindow->Name : "NULL");
                                    }
                                }
                            }
                            else if (dd.draggingTabSrc && (!tab.draggable || !mhs.allowExchangeTabLabels)) {
                                // Prohibition sign-------
                                const ImVec2& itemSize = ImGui::GetItemRectSize();
                                const ImVec2 itemPos =ImVec2(
                                            sumX-itemSize.x*0.5f-ImGui::GetScrollX(),
                                            ImGui::GetCursorPos().y-itemSize.y*0.5f-ImGui::GetScrollY()
                                            );
                                ImDrawList* drawList = ImGui::GetWindowDrawList();  // main problem is that the sign is covered by the dragging tab (even if the latter is semi-transparent...)
                                const ImVec2 wp = g.HoveredWindow->Pos;
                                dd.drawProhibitionSign(drawList,wp,itemPos,dd.draggingTabSrcSize.y*1.2f);
                            }
                        }
                    }
                    else if (dd.draggingTabSrc && dd.draggingTabSrc!=&tab && root_window && g.CurrentWindow) {
                        // This code should execute only on a drop AFAIK
                        const int len1 = strlen(root_window->Name);
                        const int len2 = strlen(g.CurrentWindow->Name);
                        if (strncmp(root_window->Name,g.CurrentWindow->Name,len1)==0 && (len1<=len2 || g.CurrentWindow->Name[len1]=='.'))    {
                            //fprintf(stderr,"g.HoveredRootWindow=%s g.CurrentWindow=%s\n",g.HoveredRootWindow?g.HoveredRootWindow->Name:"NULL",g.CurrentWindow?g.CurrentWindow->Name:"NULL");
                            dd.draggingTabDst = &tab;
                            dd.draggingTabNodeDst = this;
                            dd.draggingTabImGuiWindowDst = g.HoveredWindow;
                            dd.draggingTabWindowDst = mhs.tabWindow;
                        }
                    }

                    if (mhs.isRMBclicked && mhs.isWindowHovered && !dd.draggingTabSrc) {
                        // select it
                        selection_changed = (selectedTab != &tab);
                        newSelectedTab = &tab;
                        tab.mustSelectNextFrame = false;
                        // see if we need popup menu
                        if (TabWindow::TabLabelPopupMenuDrawerCb)	{
                            mhs.tabLabelPopup = &tab;
                            mhs.tabLabelPopupTabWindow = mhs.tabWindow;
                            mhs.tabLabelPopupChanged = true;
                            // fprintf(stderr,"open popup\n");  // This gets actually called...
                        }
                    }

                }

            }

            selectedTab = newSelectedTab;
            if (selection_changed) mhs.tabWindow->activeNode = this;

            if (tabStyle.tabWindowLabelShowAreaSeparator) {
                const TabLabelStyle::Colors separatorColor = TabLabelStyle::Col_TabLabelText;//TabLabelStyle::Col_TabLabelSelectedBorder
                ImGui::PushStyleColor(ImGuiCol_Separator,ImGui::ColorConvertU32ToFloat4(tabStyle.colors[separatorColor]));
                ImGui::Separator(); // We'd need it higher than just one pixel...
                ImGui::PopStyleColor();
            }
            ImGui::EndGroup();allTabsSize = ImGui::GetItemRectSize();

            // tab label group popup menu trigger
            if (TabWindow::TabLabelGroupPopupMenuDrawerCb && mhs.isRMBclicked && !isAItemHovered)	{
                ImGuiWindow* window = ImGui::GetCurrentWindow();
                if (ImGui::IsMouseHoveringRect(window->Pos, window->Pos+ImVec2(windowSize.x,allTabsSize.y)))   {
                    mhs.tabLabelGroupPopupChanged = true;
                    mhs.tabLabelGroupPopup.clear();
                    mhs.tabLabelPopupTabWindow = mhs.tabWindow;
                    mhs.tabLabelGroupPopupNode = this;
                    for (int i = 0; i < numTabs; i++) {
                        TabWindow::TabLabel* tab = tabs[i];
                        mhs.tabLabelGroupPopup.push_back(tab);	// should I check against mhs.TabsToClose ?
                    }
                }
            }

            //----------------------------------------------------------------
            mhs.restoreStyleVars();     // needs matching
            const ImGuiWindowFlags childFlags = mhs.flags | (selectedTab ? selectedTab->wndFlags : 0);
            if (ImGui::RevertUpstreamBeginChildCommit::OldBeginChild("user",ImVec2(0,0),childFlags))    {
                /*if (childFlags&ImGuiWindowFlags_ShowBorders) {
                    // This kind of handling the ImGuiWindowFlags_ShowBorders flag on its own is necessary to achieve what we want
                    GImGui->CurrentWindow->Flags|=childFlags;//Changed from ImGui::GetCurrentWindow()-> (faster)
                }*/
                if (/*selectedTab &&*/ TabWindow::WindowContentDrawerCb) {
                    TabWindow::WindowContentDrawerCb(selectedTab,*mhs.tabWindow,TabWindow::WindowContentDrawerUserPtr);
                }
                else {
                    if (selectedTab) selectedTab->render();
                    else {
                        ImGui::Text("EMPTY TAB LABEL DOCKING SPACE.");
                        ImGui::Text("PLEASE DRAG AND DROP TAB LABELS HERE!");
                        ImGui::Separator();
                        ImGui::Spacing();
                        ImGui::TextWrapped("And please use ImGui::TabWindow::SetWindowContentDrawerCallback(...) to set a callback to modify this text.");}
                }
            }
            ImGui::EndChild();  // user
            mhs.storeStyleVars();
        }
        else style.Colors[ImGuiCol_ChildBg] = colorChildWindowBg;
        ImGui::EndChild();  // "name"
    }
    //----------------------------------------------------------------



}


bool TabWindow::ModalDialogSaveDisplay(const char* dialogName,ImVector<TabWindow::TabLabel*>& TabsToAskFor,ImVector<TabWindow*>& TabsToAskForParents,
bool closeTabsAfterSaving,bool allowCancel,bool * pMustCloseDialogOut,const char* btnDoNotSaveName,const char* btnSaveName,const char* btnCancelName,
const char* dialogTitleLine1,const char* dialogTitleLine2) {
    IM_ASSERT(dialogName && TabsToAskFor.size()>0);
    bool mustCloseDialog = false;
    const bool open = ImGui::BeginPopupModal(dialogName, NULL, ImGuiWindowFlags_AlwaysAutoResize);
    if (open)  {
    if (dialogTitleLine1) ImGui::Text("%s",dialogTitleLine1);
    if (dialogTitleLine2) ImGui::Text("%s",dialogTitleLine2);
    ImGui::Separator();
    const int sz = TabsToAskFor.size();
    ImGui::BeginChild("List",ImVec2(0,150),true);
    for (int i=0;i<sz;i++) {
        TabWindow::TabLabel* tabLabel = TabsToAskFor[i];
        ImGui::Text("%.*s",(int)strlen(tabLabel->getLabel())-(tabLabel->getModified() ? 1 : 0),tabLabel->getLabel());
    }
    ImGui::EndChild();
    ImGui::Separator();
    //const ImGuiStyle& style = ImGui::GetStyle();
    //ImGui::PushItemWidth(ImGui::GetWindowWidth()-(allowCancel ? ImGui::CalcTextSize(btnCancelName).x : 0)-ImGui::CalcTextSize(btnDoNotSaveName).x-ImGui::CalcTextSize(btnSaveName).x-2.f*style.ItemSpacing.x-6.f*style.FramePadding.x);
    bool cancel = false;
    if (allowCancel) {cancel = ImGui::Button(btnCancelName);ImGui::SameLine();}
    const bool doNotSave = ImGui::Button(btnDoNotSaveName);ImGui::SameLine();
    const bool save = ImGui::Button(btnSaveName);
    //ImGui::PopItemWidth();
    if (cancel) mustCloseDialog = true;
    else if (doNotSave || save) {
        for (int i=0;i<sz;i++) {
        TabWindow::TabLabel* tabLabel = TabsToAskFor[i];
        TabWindow* tabWindow = TabsToAskForParents[i];

        if (save)   {
            bool ok = false;
            if (TabWindow::TabLabelSaveCb) ok = TabWindow::TabLabelSaveCb(tabLabel,*tabWindow,NULL);   // can't we check return value ?
            else ok = tabLabel->saveAs(NULL);
        }

        if (closeTabsAfterSaving)   {
            if (MyTabWindowHelperStruct::tabLabelPopup == tabLabel) MyTabWindowHelperStruct::tabLabelPopup = NULL;
            if (gDragData.draggingTabSrc == tabLabel) gDragData.resetDraggingSrc();
            if (gDragData.draggingTabDst == tabLabel) gDragData.resetDraggingDst();
            for (int j=0;j<MyTabWindowHelperStruct::tabLabelGroupPopup.size();j++)  {
            if (MyTabWindowHelperStruct::tabLabelGroupPopup[j]==tabLabel)	{
                TabWindow::TabLabel* tmp = MyTabWindowHelperStruct::tabLabelGroupPopup[MyTabWindowHelperStruct::tabLabelGroupPopup.size()-1];
                MyTabWindowHelperStruct::tabLabelGroupPopup[MyTabWindowHelperStruct::tabLabelGroupPopup.size()-1] = MyTabWindowHelperStruct::tabLabelGroupPopup[j];
                MyTabWindowHelperStruct::tabLabelGroupPopup[j] = tmp;
                MyTabWindowHelperStruct::tabLabelGroupPopup.pop_back();
                j--;
            }
            }

            if (!tabWindow->mainNode->removeTabLabel(tabLabel,true,&tabWindow->activeNode))   {
            fprintf(stderr,"Error: Can't delete TabLabel: \"%s\"\n",tabLabel->getLabel());
            }
        }
        }
        mustCloseDialog = true;
    }
    ImGui::EndPopup();
    if (mustCloseDialog) {ImGuiContext& g = *GImGui; while (g.OpenPopupStack.size() > 0) g.OpenPopupStack.pop_back();}   // Close all existing context-menus
    //if (cancel) mustCloseDialog = false;
    }
    if (pMustCloseDialogOut) *pMustCloseDialogOut = mustCloseDialog;
    return open;
}

void TabWindow::render()
{
    IM_ASSERT(ImGui::GetCurrentWindow());   // Call me inside a window

    if (!init) {init=true;}
    if (!activeNode) activeNode = mainNode->getFirstLeaftNode();

    ImVec2 windowSize = ImGui::GetWindowSize();
    windowSize.x-=/*2.f**/ImGui::GetStyle().WindowPadding.x;	// It should be 2.f*ImGui::GetStyle().WindowPadding.x, but ImGui::GetStyle().WindowPadding.x seems to work better...
    windowSize.y-=(2.f*ImGui::GetStyle().WindowPadding.y+ImGui::GetCurrentWindow()->TitleBarHeight());
    TabWindowDragData& dd = gDragData;

    static int frameCnt = -1;
    static bool lastFrameNoDragTabLabelHasBeenDrawn = true;
    ImGuiContext& g = *GImGui;
    if (frameCnt!=g.FrameCount) {
        frameCnt=g.FrameCount;
        const bool mustDrawDraggedTabLabel = (!g.HoveredWindow || lastFrameNoDragTabLabelHasBeenDrawn) && dd.draggingTabSrc;
        lastFrameNoDragTabLabelHasBeenDrawn = true;
        //--------------------------------------------------------------
        // Some "static" actions here:
        //--------------------------------------------------------------
        // 1) Close Tabs
        //--------------------------------------------------------------
        if (MyTabWindowHelperStruct::TabsToClose.size()>0)   {
            const int sz = MyTabWindowHelperStruct::TabsToClose.size();
	    for (int i=0;i<sz;i++) {
		TabLabel* tabLabel = MyTabWindowHelperStruct::TabsToClose[i];
                    TabWindow* tabWindow = MyTabWindowHelperStruct::TabsToCloseParents[i];

                    if (MyTabWindowHelperStruct::tabLabelPopup == tabLabel) MyTabWindowHelperStruct::tabLabelPopup = NULL;
                    if (dd.draggingTabSrc == tabLabel) dd.resetDraggingSrc();
                    if (dd.draggingTabDst == tabLabel) dd.resetDraggingDst();
                    for (int j=0;j<MyTabWindowHelperStruct::tabLabelGroupPopup.size();j++)  {
                        if (MyTabWindowHelperStruct::tabLabelGroupPopup[j]==tabLabel)	{
                            TabLabel* tmp = MyTabWindowHelperStruct::tabLabelGroupPopup[MyTabWindowHelperStruct::tabLabelGroupPopup.size()-1];
                            MyTabWindowHelperStruct::tabLabelGroupPopup[MyTabWindowHelperStruct::tabLabelGroupPopup.size()-1] = MyTabWindowHelperStruct::tabLabelGroupPopup[j];
                            MyTabWindowHelperStruct::tabLabelGroupPopup[j] = tmp;
                            MyTabWindowHelperStruct::tabLabelGroupPopup.pop_back();
                            j--;
                        }
                    }
		    if (!tabWindow->mainNode->removeTabLabel(tabLabel,true,&tabWindow->activeNode))   {
			fprintf(stderr,"Error: Can't delete TabLabel: \"%s\"\n",tabLabel->getLabel());
		    }
            }
	    MyTabWindowHelperStruct::ResetTabsToClose();
	}

    if (MyTabWindowHelperStruct::TabsToAskForClosing.size()>0)   {
        //fprintf(stderr,"Ok: %d\n",MyTabWindowHelperStruct::TabsToAskForClosing.size());
        if (MyTabWindowHelperStruct::MustOpenAskForClosingPopup)    {
            ImGuiContext& g = *GImGui; while (g.OpenPopupStack.size() > 0) g.OpenPopupStack.pop_back();   // Close all existing context-menus
            ImGui::OpenPopup(TabWindow::GetTabLabelAskForDeletionModalWindowName());
            MyTabWindowHelperStruct::MustOpenAskForClosingPopup = false;
        }
        bool mustCloseDialog = false;
        if (!ModalDialogSaveDisplay(GetTabLabelAskForDeletionModalWindowName(),MyTabWindowHelperStruct::TabsToAskForClosing,MyTabWindowHelperStruct::TabsToAskForClosingParents,
                    !MyTabWindowHelperStruct::TabsToAskForClosingIsUsedJustToSaveTheseTabs,!MyTabWindowHelperStruct::TabsToAskForClosingDontAllowCancel,&mustCloseDialog))
            MyTabWindowHelperStruct::ResetTabsToAskForClosing();
        if (mustCloseDialog) MyTabWindowHelperStruct::ResetTabsToAskForClosing();

    }
        // 2) Display Tab Menu ------------------------------------------
        if (TabLabelPopupMenuDrawerCb && MyTabWindowHelperStruct::tabLabelPopup) {
            if (MyTabWindowHelperStruct::tabLabelPopupChanged) {
                MyTabWindowHelperStruct::tabLabelPopupChanged = false;
                ImGuiContext& g = *GImGui; while (g.OpenPopupStack.size() > 0) g.OpenPopupStack.pop_back();   // Close all existing context-menus
                ImGui::OpenPopup(TabWindow::GetTabLabelPopupMenuName());
            }
            TabLabelPopupMenuDrawerCb(MyTabWindowHelperStruct::tabLabelPopup,*MyTabWindowHelperStruct::tabLabelPopupTabWindow,TabLabelPopupMenuDrawerUserPtr);
        }
        if (TabLabelGroupPopupMenuDrawerCb && MyTabWindowHelperStruct::tabLabelGroupPopup.size()>0)    {
            if (MyTabWindowHelperStruct::tabLabelGroupPopupChanged) {
                MyTabWindowHelperStruct::tabLabelGroupPopupChanged = false;
                ImGuiContext& g = *GImGui; while (g.OpenPopupStack.size() > 0) g.OpenPopupStack.pop_back();   // Close all existing context-menus
                ImGui::OpenPopup(TabWindow::GetTabLabelGroupPopupMenuName());
            }
            TabLabelGroupPopupMenuDrawerCb(MyTabWindowHelperStruct::tabLabelGroupPopup,*MyTabWindowHelperStruct::tabLabelPopupTabWindow,MyTabWindowHelperStruct::tabLabelGroupPopupNode,TabLabelGroupPopupMenuDrawerUserPtr);
        }
        // 3) Display dragging button only if no hover window is present (otherwise we need to draw something under it before, see below)
    if (mustDrawDraggedTabLabel)  {
	    if (dd.draggingTabWindowSrc->isIsolated()) {
		dd.resetDraggingSrc();
		MyTabWindowHelperStruct::LockedDragging = true;	// consume one click before restrt dragging again
	    }
	    else {
		const ImVec2& mp = ImGui::GetIO().MousePos;
		const ImVec2 wp = dd.draggingTabImGuiWindowSrc->Pos;
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		drawList->PushClipRectFullScreen(); // New
		dd.drawDragButton(drawList,wp,mp);
	    }
        }
        //----------------------------------------------------------------
        gDragData.resetDraggingDst();
        //----------------------------------------------------------------
    }

    MyTabWindowHelperStruct mhs(this);
    mainNode->render(windowSize,&mhs);
    static const ImGuiWindow* HoveredCorrectChildWindow = NULL;

    // Draw dragging stuff and Apply drag logic -------------------------------------------
	ImGuiWindow* root_window = g.HoveredWindow ? g.HoveredWindow->RootWindow : NULL;
    if (root_window==ImGui::GetCurrentWindow())
    {
        ImGuiStyle& style = ImGui::GetStyle();
        int hoversInt = 0;  // 1 = center, 3 = center-top, 4 = center-right, 5 = center-bottom, 2 = center-left,

        // Draw tab label while mouse drags it
        if (dd.draggingTabSrc) {
            IM_ASSERT(dd.draggingTabImGuiWindowSrc);
            const ImVec2& mp = ImGui::GetIO().MousePos;
            const ImVec2 wp = dd.draggingTabImGuiWindowSrc->Pos;
            ImDrawList* drawList = ImGui::GetForegroundDrawList();    

            const ImGuiWindow* hoveredWindow = g.HoveredWindow;
            //const ImGuiWindow* hoveredRootWindow = g.HoveredRootWindow;
            int hoveredWindowNameSz = 0;
            //-------------------------
            const char* match = NULL;
            // Window -----------------
            if (hoveredWindow && hoveredWindow!=dd.draggingTabImGuiWindowSrc
                    && (hoveredWindowNameSz=strlen(hoveredWindow->Name))>4 &&
                    //strcmp(&hoveredWindow->Name[hoveredWindowNameSz-4],"user")==0
                    (match=strstr(hoveredWindow->Name,"user"))
                    //&& strncmp(g.ActiveIdWindow->Name,hoveredWindow->Name,hoveredWindowNameSz-5)!=0 // works for g.ActiveIdWindow or g.FocusedWindow
                    )
            {
                const int matchLen = match-hoveredWindow->Name+4;
                if (matchLen==hoveredWindowNameSz) {
                    HoveredCorrectChildWindow = hoveredWindow;
                    //ImGui::SetTooltip("good: \"%s\"",hoveredWindow->Name);
                }
                else {
                    if (HoveredCorrectChildWindow && strncmp(HoveredCorrectChildWindow->Name,hoveredWindow->Name,matchLen)==0 && (int)strlen(HoveredCorrectChildWindow->Name)==matchLen) {
                        //ImGui::SetTooltip("good (reused): \"%s\" for \"%s\"",HoveredCorrectChildWindow->Name,hoveredWindow->Name);
                    }
                    else {
                        HoveredCorrectChildWindow = NULL;
                        for (int i=0,isz=g.Windows.size();i<isz;i++)    {
                            const ImGuiWindow* wnd = g.Windows[i];
                            if (strncmp(wnd->Name,hoveredWindow->Name,matchLen)==0 && (int)strlen(wnd->Name)==matchLen) {
                                HoveredCorrectChildWindow = wnd;
                                break;
                            }
                        }
                        //if (HoveredCorrectChildWindow)  ImGui::SetTooltip("recalculated: \"%s\" for \"%s\"",HoveredCorrectChildWindow->Name,hoveredWindow->Name);
                        //else ImGui::SetTooltip("bad: \"%s\"",hoveredWindow->Name);
                    }
                }
                if (HoveredCorrectChildWindow)  {
                    hoveredWindow = HoveredCorrectChildWindow;  // Mandatory
                    //---------------------------------------------

                    // Background
                    const ImVec2 wp = hoveredWindow->Pos;
                    const ImVec2 ws = hoveredWindow->Size;
                    ImVec2 start(wp.x,wp.y);
                    ImVec2 end(start.x+ws.x,start.y+ws.y);
                    const float draggedBtnAlpha = 0.35f;
                    const ImVec4& bgColor = style.Colors[ImGuiCol_TitleBg];
                    drawList->AddRectFilled(start,end,ImColor(bgColor.x,bgColor.y,bgColor.z,bgColor.w*draggedBtnAlpha),style.FrameRounding);

                    // central quad
                    const float defaultQuadAlpha = 0.75f;
                    const ImTextureID tid = DockPanelIconTextureID;
                    ImU32 quadCol = ImColor(1.f,1.f,1.f,defaultQuadAlpha);
                    ImU32 quadColHovered = ImColor(0.5f,0.5f,1.f,1.f);
                    const float minDim = ws.x < ws.y ? ws.x : ws.y;
                    const float MIN_SIZE = 87.5f;
                    const float MAX_SIZE = 200.5f;
                    float centralQuadDim = minDim*0.45f;
                    if	    (MIN_SIZE>0 && centralQuadDim<MIN_SIZE) centralQuadDim = MIN_SIZE;
                    else if (MAX_SIZE>0 && centralQuadDim>MAX_SIZE) centralQuadDim = MAX_SIZE;
                    ImVec2 uv0,uv1;bool hovers;

                    if (dd.draggingTabWindowSrc->canExchangeTabLabelsWith(this))	{
                        const float singleQuadDim = centralQuadDim*0.3333333334f;
                        // central quad top
                        uv0=ImVec2(0.22916f,0.f);uv1=ImVec2(0.45834f,0.22916f);
                        start.x = wp.x + (ws.x-singleQuadDim)*0.5f;
                        start.y = wp.y + (ws.y-singleQuadDim)*0.5f-singleQuadDim;
                        end.x = start.x+singleQuadDim;
                        end.y = start.y+singleQuadDim;
                        hovers = ImGui::IsMouseHoveringRect(start,end,false);
                        if (hovers) hoversInt = 3;
                        drawList->AddImage(tid,start,end,uv0,uv1,hovers ? quadColHovered : quadCol);
                        // central quad right
                        uv0=ImVec2(0.45834f,0.22916f);uv1=ImVec2(0.6875f,0.45834f);
                        start.x = wp.x + (ws.x-singleQuadDim)*0.5f + singleQuadDim;
                        start.y = wp.y + (ws.y-singleQuadDim)*0.5f;
                        end.x = start.x+singleQuadDim;
                        end.y = start.y+singleQuadDim;
                        hovers = ImGui::IsMouseHoveringRect(start,end,false);
                        if (hovers) hoversInt = 4;
                        drawList->AddImage(tid,start,end,uv0,uv1,hovers ? quadColHovered : quadCol);
                        // central quad bottom
                        uv0=ImVec2(0.22916f,0.45834f);uv1=ImVec2(0.45834f,0.6875f);
                        start.x = wp.x + (ws.x-singleQuadDim)*0.5f;
                        start.y = wp.y + (ws.y-singleQuadDim)*0.5f+singleQuadDim;
                        end.x = start.x+singleQuadDim;
                        end.y = start.y+singleQuadDim;
                        hovers = ImGui::IsMouseHoveringRect(start,end,false);
                        if (hovers) hoversInt = 5;
                        drawList->AddImage(tid,start,end,uv0,uv1,hovers ? quadColHovered : quadCol);
                        // central quad left
                        uv0=ImVec2(0.0f,0.22916f);uv1=ImVec2(0.22916f,0.45834f);
                        start.x = wp.x + (ws.x-singleQuadDim)*0.5f - singleQuadDim;
                        start.y = wp.y + (ws.y-singleQuadDim)*0.5f;
                        end.x = start.x+singleQuadDim;
                        end.y = start.y+singleQuadDim;
                        hovers = ImGui::IsMouseHoveringRect(start,end,false);
                        if (hovers) hoversInt = 2;
                        drawList->AddImage(tid,start,end,uv0,uv1,hovers ? quadColHovered : quadCol);
                        // central quad center
                        uv0=ImVec2(0.22916f,0.22916f);uv1=ImVec2(0.45834f,0.45834f);
                        start.x = wp.x + (ws.x-singleQuadDim)*0.5f;
                        start.y = wp.y + (ws.y-singleQuadDim)*0.5f;
                        end.x = start.x+singleQuadDim;
                        end.y = start.y+singleQuadDim;
                        hovers = //hoversInt==0;
                                ImGui::IsMouseHoveringRect(start,end,false);
                        if (hovers) hoversInt = 1;
                        drawList->AddImage(tid,start,end,uv0,uv1,hovers ? quadColHovered : quadCol);
                        // Refinement: draw remaining 4 inert quads
                        uv0=ImVec2(0.f,0.f);uv1=ImVec2(0.22916f,0.22916f);
                        start.x = wp.x + (ws.x-singleQuadDim)*0.5f - singleQuadDim;
                        start.y = wp.y + (ws.y-singleQuadDim)*0.5f - singleQuadDim;
                        end.x = start.x+singleQuadDim;end.y = start.y+singleQuadDim;
                        drawList->AddImage(tid,start,end,uv0,uv1,quadCol);
                        uv0=ImVec2(0.45834f,0.f);uv1=ImVec2(0.6875f,0.22916f);
                        start.x = wp.x + (ws.x-singleQuadDim)*0.5f + singleQuadDim;
                        start.y = wp.y + (ws.y-singleQuadDim)*0.5f - singleQuadDim;
                        end.x = start.x+singleQuadDim;end.y = start.y+singleQuadDim;
                        drawList->AddImage(tid,start,end,uv0,uv1,quadCol);
                        uv0=ImVec2(0.f,0.45834f);uv1=ImVec2(0.22916f,0.6875f);
                        start.x = wp.x + (ws.x-singleQuadDim)*0.5f - singleQuadDim;
                        start.y = wp.y + (ws.y-singleQuadDim)*0.5f + singleQuadDim;
                        end.x = start.x+singleQuadDim;end.y = start.y+singleQuadDim;
                        drawList->AddImage(tid,start,end,uv0,uv1,quadCol);
                        uv0=ImVec2(0.45834f,0.45834f);uv1=ImVec2(0.6875f,0.6875f);
                        start.x = wp.x + (ws.x-singleQuadDim)*0.5f + singleQuadDim;
                        start.y = wp.y + (ws.y-singleQuadDim)*0.5f + singleQuadDim;
                        end.x = start.x+singleQuadDim;end.y = start.y+singleQuadDim;
                        drawList->AddImage(tid,start,end,uv0,uv1,quadCol);
                    }
                    else {
                        hoversInt = 0;
                        uv0=ImVec2(0.5f,0.75f);uv1=ImVec2(0.75f,1.f);
                        start.x = wp.x + (ws.x-centralQuadDim)*0.5f;
                        start.y = wp.y + (ws.y-centralQuadDim)*0.5f;
                        end.x = start.x+centralQuadDim;end.y = start.y+centralQuadDim;
                        drawList->AddImage(tid,start,end,uv0,uv1,quadCol);
                    }

                }
            }
            else HoveredCorrectChildWindow = NULL;
            // Button -----------------
            dd.drawDragButton(drawList,wp,mp);
            lastFrameNoDragTabLabelHasBeenDrawn = false;
            // -------------------------------------------------------------------
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
        }

        // Drop tab label onto another
        if (dd.draggingTabDst && dd.draggingTabDst->draggable) {
            // swap draggingTabSrc and draggingTabDst
            IM_ASSERT(dd.isDraggingSrcValid());
            IM_ASSERT(dd.isDraggingDstValid());
            IM_ASSERT(dd.draggingTabSrc!=dd.draggingTabDst);

            if (dd.draggingTabWindowSrc->canExchangeTabLabelsWith(this))    {

                if (dd.draggingTabNodeSrc!=dd.draggingTabNodeDst) {
                    bool srcWasSelected = dd.draggingTabNodeSrc->selectedTab == dd.draggingTabSrc;
                    bool dstWasSelected = dd.draggingTabNodeDst->selectedTab == dd.draggingTabDst;
                    if (srcWasSelected) dd.draggingTabNodeSrc->selectedTab = dd.draggingTabDst;
                    if (dstWasSelected) dd.draggingTabNodeDst->selectedTab = dd.draggingTabSrc;
                }

                const int iSrc = dd.findDraggingSrcIndex();
                IM_ASSERT(iSrc>=0);
                const int iDst = dd.findDraggingDstIndex();
                IM_ASSERT(iDst>=0);
                dd.draggingTabNodeDst->tabs[iDst] = dd.draggingTabSrc;
                dd.draggingTabNodeSrc->tabs[iSrc] = dd.draggingTabDst;

                dd.reset();
                //fprintf(stderr,"Drop tab label onto another\n");
            }
        }

        // Reset draggingTabIndex if necessary
        if (!MyTabWindowHelperStruct::isMouseDragging) {
            if (hoversInt && HoveredCorrectChildWindow && dd.draggingTabSrc && dd.draggingTabImGuiWindowSrc && dd.draggingTabImGuiWindowSrc!=g.HoveredWindow && dd.draggingTabImGuiWindowSrc!=HoveredCorrectChildWindow)
            {
                // Drop tab label onto a window portion
                int nameSz = strlen(HoveredCorrectChildWindow->Name);
                static const char trailString[] = ".user";
                static const int trailStringSz = (int) strlen(trailString);
                IM_ASSERT(nameSz>=trailStringSz);
                IM_ASSERT(strcmp(&HoveredCorrectChildWindow->Name[nameSz-trailStringSz],trailString)==0);
                const char* startMatchCh = strstr(HoveredCorrectChildWindow->Name,".##main"),*startMatchCh2 = NULL;
                if (startMatchCh)   {
                    while ((startMatchCh2 = strstr(&HoveredCorrectChildWindow->Name[(int)(startMatchCh-HoveredCorrectChildWindow->Name)+7],".##main"))) {
                        startMatchCh = startMatchCh2;
                    }
                }
                const int startMatchIndex = startMatchCh ? ((int)(startMatchCh-HoveredCorrectChildWindow->Name)+1) : 0;
                IM_ASSERT(nameSz>=trailStringSz-startMatchIndex);

                ImVector<char> tmp;tmp.resize(nameSz);
                strncpy(&tmp[0],&HoveredCorrectChildWindow->Name[startMatchIndex],nameSz-trailStringSz-startMatchIndex);
                tmp[nameSz-trailStringSz-startMatchIndex]='\0';
                //fprintf(stderr,"\"%s\"\n",&tmp[0]);
                dd.draggingTabNodeDst = TabWindowDragData::FindTabNodeByName(mainNode,&tmp[0]);

		//fprintf(stderr,"Item: \"%s\" dragged to window:\"%s\" at pos: %d\n",dd.draggingTabSrc->label,HoveredCorrectChildWindow ? HoveredCorrectChildWindow->Name : "NULL",hoversInt);
                //if (dd.draggingTabNodeDst)  fprintf(stderr,"dd.draggingTabNodeDst->tabs.size()=%d\n",(int)dd.draggingTabNodeDst->tabs.size());
                //else fprintf(stderr,"No dd.draggingTabNodeDst.\n");
                //TODO: move dd.draggingTabSrc and delete the src node if empty------------
		// How can I find dd.draggingTabNodeDst from HoveredCorrectChildWindow?
                // I must strip ".HorizontalStrip.content.user" and then seek TabNode::Name
                //-------------------------------------------------------------------------
                if (dd.draggingTabNodeDst) {
                    if (hoversInt!=1 && dd.draggingTabNodeDst->tabs.size()==0) hoversInt=1;
                    if (!(dd.draggingTabNodeDst==dd.draggingTabNodeSrc && (dd.draggingTabNodeDst->tabs.size()==0 || hoversInt==1))) {
                        // We must:

                        // 1) remove dd.draggingTabSrc from dd.draggingTabNodeSrc
                        if (!dd.draggingTabNodeSrc->removeTabLabel(dd.draggingTabSrc,false,&dd.draggingTabNodeDst,true))   {
                            //fprintf(stderr,"Error: !dd.draggingTabNodeSrc->removeTabLabel(dd.draggingTabSrc,false,&activeNode,true): \"%s\"\n",dd.draggingTabSrc->getLabel());
                        }
                        // 2) append if to dd.draggingTabNodeDst
                        activeNode = dd.draggingTabNodeDst->addTabLabel(dd.draggingTabSrc,hoversInt==1 ? -1 : hoversInt-2);
                        //----------------------------------------------------
                    }
                    //else fprintf(stderr,"Do nothing.\n");
                }

                dd.resetDraggingDst();
            }
            if (dd.draggingTabSrc) dd.resetDraggingSrc();
            MyTabWindowHelperStruct::LockedDragging = false;
        }
    }

}

void TabWindow::clearNodes() {
    if (mainNode)   {
        mainNode->~TabWindowNode();
        ImGui::MemFree(mainNode);
        mainNode=NULL;
    }
    activeNode = NULL;
}
void TabWindow::clear() {mainNode->clear();activeNode=mainNode;}


TabWindow::TabWindow() {
    mainNode = (TabWindowNode*) ImGui::MemAlloc(sizeof(TabWindowNode));
    IM_PLACEMENT_NEW(mainNode) TabWindowNode();
    mainNode->name = (char*) ImGui::MemAlloc(7);strcpy(mainNode->name,"##main");
    activeNode=mainNode;
    init=false;
    userPtr=NULL;
    isolatedMode=false;

}
TabWindow::~TabWindow() {clearNodes();}

void TabWindow::excludeTabWindow(TabWindow &tw)	{
    for (int i=0,isz=tabWindowsToExclude.size();i<isz;i++)  {
	if (&tw == tabWindowsToExclude[i]) return;
    }
    tabWindowsToExclude.push_back(&tw);
    for (int i=0,isz=tw.tabWindowsToExclude.size();i<isz;i++)  {
	if (this == tw.tabWindowsToExclude[i]) return;
    }
    tw.tabWindowsToExclude.push_back(this);
}
void TabWindow::includeTabWindow(TabWindow &tw)	{
    for (int i=0,isz=tabWindowsToExclude.size();i<isz;i++)  {
	if (&tw == tabWindowsToExclude[i]) {
	    TabWindow* tmp = tabWindowsToExclude[isz-1];
	    tabWindowsToExclude[isz-1] = tabWindowsToExclude[i];
	    tabWindowsToExclude[i] = tmp;
	    tabWindowsToExclude.pop_back();
	    break;
	}
    }
    for (int i=0,isz=tw.tabWindowsToExclude.size();i<isz;i++)  {
	if (this == tw.tabWindowsToExclude[i]) {
	    TabWindow* tmp = tw.tabWindowsToExclude[isz-1];
	    tw.tabWindowsToExclude[isz-1] = tw.tabWindowsToExclude[i];
	    tw.tabWindowsToExclude[i] = tmp;
	    tw.tabWindowsToExclude.pop_back();
	    break;
	}
    }
}
bool TabWindow::canExchangeTabLabelsWith(TabWindow *tw)	{
    if ((tw!=this && (isolatedMode || tw->isIsolated())) || !tw) return false;
    for (int i=0,isz=tabWindowsToExclude.size();i<isz;i++)  {
	if (tw == tabWindowsToExclude[i]) return false;
    }
    return true;
}

bool TabWindow::isMergeble(TabWindowNode *node)    {
    return (node && node->isLeafNode() && node->parent);
}
int TabWindow::getNumTabs(TabWindowNode *node)  {
    return node ? node->tabs.size() : 0;
}
int TabWindow::getNumClosableTabs(TabWindowNode *node)    {
    return (node && node->tabs.size()>0) ? node->numClosableTabs : 0;
}
bool TabWindow::merge(TabWindowNode *node) {
    if (!node || !isMergeble(node)) return false;
    {TabWindowNode* n = node;while (n->parent) n=n->parent;if (n!=mainNode) return false;} // checks if tabNode belongs to this TabWindow

    if (!node->mergeToParent(&activeNode)) return false;
    node = NULL;    // it's invalid now

    /*TabWindowNode* parent = node->parent;
    ImVector<TabLabel*> tabs;int sz=0;
    while ((sz=node->tabs.size())>0)   {
        TabLabel* tab = node->tabs[0];
        tabs.push_back(tab);
        node->removeTabLabel(tab,false,&activeNode,true);
        if (sz==1) break;
    }
    node = NULL;    // it's invalid now
    parent = parent->getFirstLeaftNode();
    for (int i=0,isz = tabs.size();i<isz;i++)   {
        parent->addTabLabel(tabs[i]);
    }*/

    return true;
}

void TabWindow::GetAllTabLabels(TabWindow *pTabWindowsIn, int numTabWindowsIn, ImVector<TabWindow::TabLabel *> &tabsOut, ImVector<TabWindow *> &parentsOut,bool onlyClosableTabs,bool onlyModifiedTabs) {
    IM_ASSERT(pTabWindowsIn && numTabWindowsIn>0);
    tabsOut.clear();parentsOut.clear();
    for (int i=0;i<numTabWindowsIn;i++)	{
    TabWindow& tw = pTabWindowsIn[i];
    const int startTabs = tabsOut.size();
    tw.mainNode->getTabLabels(tabsOut,onlyClosableTabs,onlyModifiedTabs);
    if (startTabs!=tabsOut.size())	{
        parentsOut.resize(tabsOut.size());
        for (int j=startTabs,jsz=tabsOut.size();j<jsz;j++)	{
        parentsOut[j] = &tw;
        }
    }
    }
}
bool TabWindow::CloseTabLabelsHelper(ImVector<TabWindow::TabLabel *> &tabs, ImVector<TabWindow *> &parents,bool saveAll, bool askForSaving, bool allowCancelDialog,bool dontCloseTabs)    {
    bool mustStartDialog = false;
    if (tabs.size()==0) return false;
    IM_ASSERT(tabs.size()==parents.size());
    for (int i=0,isz=tabs.size();i<isz;i++) {
        TabWindow::TabLabel *tab = tabs[i];
        TabWindow* tw = parents[i];
        IM_ASSERT(tab && tw);
        if (saveAll) {
            if (tab->modified)  {
                if (askForSaving)   {
                    if (!mustStartDialog) {
                        MyTabWindowHelperStruct::ResetTabsToAskForClosing();
                        MyTabWindowHelperStruct::TabsToAskForClosingDontAllowCancel = !allowCancelDialog;
                        MyTabWindowHelperStruct::TabsToAskForClosingIsUsedJustToSaveTheseTabs = dontCloseTabs;
                        mustStartDialog = true;
                    }
                    MyTabWindowHelperStruct::TabsToAskForClosing.push_back(tab);
                    MyTabWindowHelperStruct::TabsToAskForClosingParents.push_back(tw);
                }
                else {
                    bool ok = false;
                    if (TabWindow::TabLabelSaveCb) ok = TabWindow::TabLabelSaveCb(tab,*tw,NULL);   // can't we check return value ?
                    else ok = tab->saveAs(NULL);
                }
            }
        }
        if (!dontCloseTabs) {
            if (!mustStartDialog || (tab!=MyTabWindowHelperStruct::TabsToAskForClosing[MyTabWindowHelperStruct::TabsToAskForClosing.size()-1]))	{
                // delete tab
                //fprintf(stderr,"Deleting tab: %s\n",tab->getLabel());
                tw->mainNode->removeTabLabel(tab,true,&tw->activeNode,false);
            }
        }
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    if (mustStartDialog)    {
        MyTabWindowHelperStruct::MustOpenAskForClosingPopup = true;
        //fprintf(stderr,"Ok: %d\n",MyTabWindowHelperStruct::TabsToAskForClosing.size());
    }
    return mustStartDialog;
}
bool TabWindow::AreSomeDialogsOpen()	{return (GImGui->OpenPopupStack.size() > 0);}

void TabWindow::getAllTabLabels(ImVector<TabWindow::TabLabel *> &tabsOut, bool onlyClosableTabs, bool onlyModifiedTabs)	{tabsOut.clear();mainNode->getTabLabels(tabsOut,onlyClosableTabs,onlyModifiedTabs);}
void TabWindow::saveAll(ImVector<TabWindow::TabLabel *> *ptabs)	{
    ImVector<TabWindow::TabLabel *> tabs;
    if (!ptabs) {
    getAllTabLabels(tabs,false,true);
    ptabs = &tabs;
    }
    for (int i=0,isz=ptabs->size();i<isz;i++)	{
    TabWindow::TabLabel *tab = (*ptabs)[i];
    bool ok = false;
    if (TabWindow::TabLabelSaveCb) ok = TabWindow::TabLabelSaveCb(tab,*this,NULL);   // can't we check return value ?
    else ok = tab->saveAs(NULL);
    }
}
bool TabWindow::startCloseAllDialog(ImVector<TabWindow::TabLabel *> *ptabs, bool allowCancelDialog)  {
    ImVector<TabWindow::TabLabel *> tabs;
    ImVector<TabWindow *> tws;
    if (!ptabs) {
        getAllTabLabels(tabs,false,false);
        ptabs = &tabs;
    }
    tws.resize(ptabs->size());
    for (int i=0,isz=ptabs->size();i<isz;i++)	{tws[i]=this;}
    return StartCloseAllDialog(*ptabs,tws,allowCancelDialog);
}



//-------------------------------------------------------------------------------
#if (defined(IMGUIHELPER_H_) && !defined(NO_IMGUIHELPER_SERIALIZATION))
#ifndef NO_IMGUIHELPER_SERIALIZATION_SAVE
bool TabWindow::save(ImGuiHelper::Serializer &s)    {
    if (!s.isValid()) return false;
    mainNode->serialize(s,this);
    return true;
}
bool TabWindow::save(const char* filename)  {
    ImGuiHelper::Serializer s(filename);    
    return save(s);
}
bool TabWindow::Save(const char *filename, TabWindow *pTabWindows, int numTabWindows)   {
    IM_ASSERT(pTabWindows && numTabWindows>0);
    ImGuiHelper::Serializer s(filename);
    bool ok = true;
    for (int i=0;i<numTabWindows;i++)   {
        ok|=pTabWindows[i].save(s);
    }
    return ok;
}
#endif //NO_IMGUIHELPER_SERIALIZATION_SAVE
#ifndef NO_IMGUIHELPER_SERIALIZATION_LOAD
bool TabWindow::load(ImGuiHelper::Deserializer &d, const char **pOptionalBufferStart) {
    if (!d.isValid()) return false;
    this->clear();      // Well, shouldn't we ask for modified unclosed tab labels here ?

    const char* amount = pOptionalBufferStart ? (*pOptionalBufferStart) : 0;
    mainNode->deserialize(d,NULL,amount,this);
    if (pOptionalBufferStart) *pOptionalBufferStart = amount;

    mainNode->mergeEmptyLeafNodes(&activeNode);	// optional, but should fix hierarchy problems
    if (!activeNode) activeNode=mainNode;
    if (!activeNode->isLeafNode()) activeNode = activeNode->getFirstLeaftNode();
    IM_ASSERT(activeNode && activeNode->isLeafNode());

    return true;
}
bool TabWindow::load(const char* filename)  {
    ImGuiHelper::Deserializer d(filename);
    return load(d);
}
bool TabWindow::Load(const char *filename, TabWindow *pTabWindows, int numTabWindows)   {
    IM_ASSERT(pTabWindows && numTabWindows>0);
    for (int i=0;i<numTabWindows;i++)   {
        pTabWindows[i].clear(); // Well, shouldn't we ask for modified unclosed tab labels here ?
    }
    ImGuiHelper::Deserializer d(filename);
    const char* amount = 0; bool ok = true;
    for (int i=0;i<numTabWindows;i++)   {
        ok|=pTabWindows[i].load(d,&amount);
    }
    return ok;
}
#endif //NO_IMGUIHELPER_SERIALIZATION_LOAD
#endif //NO_IMGUIHELPER_SERIALIZATION
//--------------------------------------------------------------------------------

TabWindow::TabLabel *TabWindow::createTabLabel(const char *label, const char *tooltip, bool closable, bool draggable, void *userPtr, const char *userText, int userInt, int ImGuiWindowFlagsForContent) {
    TabLabel* tab = NULL;
    if (TabLabelFactoryCb)  tab = TabLabelFactoryCb(*this,label,tooltip,closable,draggable,userPtr,userText,userInt,ImGuiWindowFlagsForContent);
    else    {
        tab = (TabLabel*) ImGui::MemAlloc(sizeof(TabLabel));
        IM_PLACEMENT_NEW(tab) TabLabel(label,tooltip,closable,draggable);
        tab->userPtr = userPtr;
        tab->setUserText(userText);
        tab->userInt =userInt;
        tab->wndFlags = ImGuiWindowFlagsForContent;
    }
    return tab;
}
TabWindow::TabLabel *TabWindow::addTabLabel(const char *label, const char *tooltip,bool closable, bool draggable, void *userPtr, const char *userText,int userInt,int ImGuiWindowFlagsForContent) {
    TabLabel* tab = createTabLabel(label,tooltip,closable,draggable,userPtr,userText,userInt,ImGuiWindowFlagsForContent);
    if (!activeNode) activeNode = mainNode->getFirstLeaftNode();
    if (tab) activeNode = activeNode->addTabLabel(tab);
    return tab;
}
TabWindow::TabLabel *TabWindow::addTabLabel(TabLabel *tabLabel, bool checkIfAlreadyPresent) {
    if (!tabLabel) return NULL;
    if (checkIfAlreadyPresent && mainNode->findTabLabel(tabLabel,true)) return tabLabel;
    if (!activeNode) activeNode = mainNode->getFirstLeaftNode();
    activeNode = activeNode->addTabLabel(tabLabel);
    return tabLabel;
}
bool TabWindow::removeTabLabel(TabWindow::TabLabel *tab) {
    if (!tab) return false;
    if (!mainNode->removeTabLabel(tab,true,&activeNode)) {
        fprintf(stderr,"Error: cannot remove TabLabel: \"%s\"\n",tab->getLabel());
        return false;
    }
    return true;
}

TabWindow::TabLabel *TabWindow::findTabLabelFromLabel(const char *label) const  {
    return mainNode->findTabLabelFromLabel(label);
}
TabWindow::TabLabel *TabWindow::findTabLabelFromTooltip(const char *tooltip) const  {
    return mainNode->findTabLabelFromTooltip(tooltip);
}
TabWindow::TabLabel *TabWindow::findTabLabelFromUserPtr(void *userPtr) const    {
    return mainNode->findTabLabelFromUserPtr(userPtr);
}
TabWindow::TabLabel *TabWindow::findTabLabelFromUserText(const char *userText) const    {
    return mainNode->findTabLabelFromUserText(userText);
}
TabWindow::TabLabel *TabWindow::FindTabLabelFromLabel(const char *label, const TabWindow *pTabWindows, int numTabWindows, int *pOptionalTabWindowIndexOut)  {
    IM_ASSERT(pTabWindows && numTabWindows>0);
    TabLabel* rv = NULL;
    for (int i=0;i<numTabWindows;i++)   {
        if ((rv = pTabWindows[i].findTabLabelFromLabel(label))) {
            if (pOptionalTabWindowIndexOut) *pOptionalTabWindowIndexOut=i;
            return rv;
        }
    }
    if (pOptionalTabWindowIndexOut) *pOptionalTabWindowIndexOut=-1;
    return NULL;
}
TabWindow::TabLabel *TabWindow::FindTabLabelFromTooltip(const char *tooltip, const TabWindow *pTabWindows, int numTabWindows, int *pOptionalTabWindowIndexOut)    {
    IM_ASSERT(pTabWindows && numTabWindows>0);
    TabLabel* rv = NULL;
    for (int i=0;i<numTabWindows;i++)   {
        if ((rv = pTabWindows[i].findTabLabelFromTooltip(tooltip))) {
            if (pOptionalTabWindowIndexOut) *pOptionalTabWindowIndexOut=i;
            return rv;
        }
    }
    if (pOptionalTabWindowIndexOut) *pOptionalTabWindowIndexOut=-1;
    return NULL;
}
TabWindow::TabLabel *TabWindow::FindTabLabelFromUserPtr(void *userPtr, const TabWindow *pTabWindows, int numTabWindows, int *pOptionalTabWindowIndexOut)    {
    IM_ASSERT(pTabWindows && numTabWindows>0);
    TabLabel* rv = NULL;
    for (int i=0;i<numTabWindows;i++)   {
        if ((rv = pTabWindows[i].findTabLabelFromUserPtr(userPtr))) {
            if (pOptionalTabWindowIndexOut) *pOptionalTabWindowIndexOut=i;
            return rv;
        }
    }
    if (pOptionalTabWindowIndexOut) *pOptionalTabWindowIndexOut=-1;
    return NULL;
}
TabWindow::TabLabel *TabWindow::FindTabLabelFromUserText(const char *userText, const TabWindow *pTabWindows, int numTabWindows, int *pOptionalTabWindowIndexOut)    {
    IM_ASSERT(pTabWindows && numTabWindows>0);
    TabLabel* rv = NULL;
    for (int i=0;i<numTabWindows;i++)   {
        if ((rv = pTabWindows[i].findTabLabelFromUserText(userText))) {
            if (pOptionalTabWindowIndexOut) *pOptionalTabWindowIndexOut=i;
            return rv;
        }
    }
    if (pOptionalTabWindowIndexOut) *pOptionalTabWindowIndexOut=-1;
    return NULL;
}




// Based on the code by krys-spectralpixel (https://github.com/krys-spectralpixel), posted here: https://github.com/ocornut/imgui/issues/261
bool TabLabels(int numTabs, const char** tabLabels, int& selectedIndex, const char** tabLabelTooltips, bool wrapMode, int *pOptionalHoveredIndex, int* pOptionalItemOrdering, bool allowTabReorder, bool allowTabClosing, int *pOptionalClosedTabIndex, int *pOptionalClosedTabIndexInsideItemOrdering) {
    ImGuiStyle& style = ImGui::GetStyle();
    const TabLabelStyle& tabStyle = TabLabelStyle::GetMergedWithWindowAlpha();

    const ImVec2 itemSpacing =  style.ItemSpacing;
	style.ItemSpacing.x =       0;
    style.ItemSpacing.y =       1;

    if (numTabs>0 && (selectedIndex<0 || selectedIndex>=numTabs)) {
        if (!pOptionalItemOrdering)  selectedIndex = 0;
        else selectedIndex = -1;
    }
    if (pOptionalHoveredIndex) *pOptionalHoveredIndex = -1;
    if (pOptionalClosedTabIndex) *pOptionalClosedTabIndex = -1;
    if (pOptionalClosedTabIndexInsideItemOrdering) *pOptionalClosedTabIndexInsideItemOrdering = -1;

    float windowWidth = 0.f,sumX=0.f;
    if (wrapMode) windowWidth = ImGui::GetWindowWidth() - style.WindowPadding.x - (ImGui::GetScrollMaxY()>0 ? style.ScrollbarSize : 0.f);

    static int draggingTabIndex = -1;int draggingTabTargetIndex = -1;   // These are indices inside pOptionalItemOrdering
    static bool draggingTabWasSelected = false;
    static ImVec2 draggingTabSize(0,0);
    static ImVec2 draggingTabOffset(0,0);
    static bool draggingLocked = false;

    const bool isRMBclicked = ImGui::IsMouseClicked(1);
    const bool isMouseDragging = ImGui::IsMouseDragging(0,3.f);
    const bool isMouseDraggingJustStarted = isMouseDragging && (ImGui::GetIO().MouseDownDuration[0] < 0.35f);// ImGui::GetIO().MouseDown[0] does not work!
    int justClosedTabIndex = -1,newSelectedIndex = selectedIndex;

    ImVec2 startGroupCursorPos = ImGui::GetCursorPos();
    ImGui::BeginGroup();
    ImVec2 tabButtonSz(0,0);bool mustCloseTab = false;bool canUseSizeOptimization = false;
    const bool isWindowHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
    bool selection_changed = false;bool noButtonDrawn = true;
    for (int j = 0,i; j < numTabs; j++)
    {
        i = pOptionalItemOrdering ? pOptionalItemOrdering[j] : j;
        if (i==-1) continue;

        if (!wrapMode) {if (!noButtonDrawn) ImGui::SameLine();canUseSizeOptimization=false;}
        else if (sumX > 0.f) {
            sumX+=style.ItemSpacing.x;   // Maybe we can skip it if we use SameLine(0,0) below
            ImGui::TabButton(tabLabels[i],(i == selectedIndex),allowTabClosing ? &mustCloseTab : NULL,NULL,&tabButtonSz,&tabStyle);
            sumX+=tabButtonSz.x;
            if (sumX>windowWidth) sumX = 0.f;
            else ImGui::SameLine();
            canUseSizeOptimization = true;
        }
        else canUseSizeOptimization = false;

        // Draw the button
        ImGui::PushID(i);   // otherwise two tabs with the same name would clash.
        if (ImGui::TabButton(tabLabels[i],i == selectedIndex,allowTabClosing ? &mustCloseTab : NULL,NULL,NULL,&tabStyle,NULL,NULL,NULL,canUseSizeOptimization))   {
            selection_changed = (selectedIndex!=i);
            newSelectedIndex = i;
        }
        ImGui::PopID();
        noButtonDrawn = false;

        if (wrapMode) {
            if (sumX==0.f) sumX = style.WindowPadding.x + ImGui::GetItemRectSize().x; // First element of a line
        }
        else if (isMouseDragging && allowTabReorder && pOptionalItemOrdering) {
            // We still need sumX
            if (sumX==0.f) sumX = style.WindowPadding.x + ImGui::GetItemRectSize().x; // First element of a line
            else sumX+=style.ItemSpacing.x + ImGui::GetItemRectSize().x;

        }

        if (isWindowHovered && ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly) && !mustCloseTab) {
            if (pOptionalHoveredIndex) *pOptionalHoveredIndex = i;
            if (tabLabelTooltips && !isRMBclicked && tabLabelTooltips[i] && strlen(tabLabelTooltips[i])>0)  ImGui::SetTooltip("%s",tabLabelTooltips[i]);

            if (pOptionalItemOrdering)  {
                if (allowTabReorder)  {
                    if (isMouseDragging) {
                        if (draggingTabIndex==-1 && !draggingLocked && isMouseDraggingJustStarted) {
                            draggingTabIndex = j;
                            draggingTabWasSelected = (i == selectedIndex);
                            draggingTabSize = ImGui::GetItemRectSize();
                            const ImVec2& mp = ImGui::GetIO().MousePos;
                            const ImVec2 draggingTabCursorPos = ImGui::GetCursorPos();
                            draggingTabOffset=ImVec2(
                                        mp.x+draggingTabSize.x*0.5f-sumX+ImGui::GetScrollX(),
                                        mp.y+draggingTabSize.y*0.5f-draggingTabCursorPos.y+ImGui::GetScrollY()
                                        );

                        }
                    }
                    else if (draggingTabIndex>=0 && draggingTabIndex<numTabs && draggingTabIndex!=j){
                        draggingTabTargetIndex = j; // For some odd reasons this seems to get called only when draggingTabIndex < i ! (Probably during mouse dragging ImGui owns the mouse someway and sometimes ImGui::IsItemHovered() is not getting called)
                    }
                }
            }
        }
        if (mustCloseTab)   {
            justClosedTabIndex = i;
            if (pOptionalClosedTabIndex) *pOptionalClosedTabIndex = i;
            if (pOptionalClosedTabIndexInsideItemOrdering) *pOptionalClosedTabIndexInsideItemOrdering = j;
			if (pOptionalItemOrdering) pOptionalItemOrdering[j] = -1;
        }

    }
    selectedIndex = newSelectedIndex;
    ImGui::EndGroup();
    ImVec2 groupSize = ImGui::GetItemRectSize();

    // Draw tab label while mouse drags it
	if (draggingTabIndex>=0 && draggingTabIndex<numTabs && pOptionalItemOrdering[draggingTabIndex] != -1) {
        const ImVec2 wp = ImGui::GetWindowPos();
        startGroupCursorPos.x+=wp.x;
        startGroupCursorPos.y+=wp.y;
        startGroupCursorPos.x-=ImGui::GetScrollX();
        startGroupCursorPos.y-=ImGui::GetScrollY();
        const float deltaY = ImGui::GetTextLineHeightWithSpacing()*2.5f;
        startGroupCursorPos.y-=deltaY;
        groupSize.y+=2.f*deltaY;
        //if (ImGui::IsMouseHoveringRect(startGroupCursorPos,startGroupCursorPos+groupSize))  {
            const ImVec2& mp = ImGui::GetIO().MousePos;
            ImVec2 start(wp.x+mp.x-draggingTabOffset.x-draggingTabSize.x*0.5f,mp.y);
            //const ImVec2 end(start.x+draggingTabSize.x,start.y+draggingTabSize.y);
            ImDrawList* drawList = ImGui::GetForegroundDrawList();
            const TabLabelStyle& tabStyle = TabLabelStyleGetMergedWithAlphaForOverlayUsage();
            ImFont* fontOverride = (ImFont*) (draggingTabWasSelected ? TabLabelStyle::ImGuiFonts[tabStyle.fontStyles[TabLabelStyle::TAB_STATE_SELECTED]] : TabLabelStyle::ImGuiFonts[tabStyle.fontStyles[TabLabelStyle::TAB_STATE_NORMAL]]);
            ImGui::TabButton(tabLabels[pOptionalItemOrdering[draggingTabIndex]],draggingTabWasSelected,allowTabClosing ? &mustCloseTab : NULL,NULL,NULL,&tabStyle,fontOverride,&start,drawList,false,true);
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);

            if (TabWindow::DockPanelIconTextureID)	{
                // Optional: draw prohibition sign when dragging too far (you can remove this if you want)
                startGroupCursorPos.y+=deltaY*.5f;
                groupSize.y-=deltaY;
                if (!ImGui::IsMouseHoveringRect(startGroupCursorPos,startGroupCursorPos+groupSize))  {
                    const float signWidth = draggingTabSize.y*1.25f;
                    start.x+=(draggingTabSize.x-signWidth)*0.5f;
                    start.y+=(draggingTabSize.y-signWidth)*0.5f;
                    const ImVec2 end(start.x+signWidth,start.y+signWidth);
                    const ImVec4 color(1.f,1.f,1.f,0.85f);
                    drawList->AddImage(TabWindow::DockPanelIconTextureID,start,end,ImVec2(0.5f,0.75f),ImVec2(0.75f,1.f),ImGui::ColorConvertFloat4ToU32(color));
                }
            }
			//}
			//else {
				//draggingTabIndex = -1;draggingTabTargetIndex=-1;
				//draggingLocked = true;// consume one mouse release
				//}
    }

    // Drop tab label
    if (draggingTabTargetIndex!=-1) {
        // swap draggingTabIndex and draggingTabTargetIndex in pOptionalItemOrdering
        const int tmp = pOptionalItemOrdering[draggingTabTargetIndex];
        pOptionalItemOrdering[draggingTabTargetIndex] = pOptionalItemOrdering[draggingTabIndex];
        pOptionalItemOrdering[draggingTabIndex] = tmp;
        //fprintf(stderr,"%d %d\n",draggingTabIndex,draggingTabTargetIndex);
        draggingTabTargetIndex = draggingTabIndex = -1;
    }

    // Reset draggingTabIndex if necessary
    if (!isMouseDragging) {draggingTabIndex = -1;draggingLocked=false;}

    // Change selected tab when user closes the selected tab
    if (selectedIndex == justClosedTabIndex && selectedIndex>=0)    {
        selectedIndex = -1;
        for (int j = 0,i; j < numTabs; j++) {
            i = pOptionalItemOrdering ? pOptionalItemOrdering[j] : j;
            if (i==-1) continue;
            selectedIndex = i;
            break;
        }
    }

    // Restore the style
    style.ItemSpacing =                     itemSpacing;

    return selection_changed;
}

//-------------------------------------------------------------------------------
#   if (defined(IMGUIHELPER_H_) && !defined(NO_IMGUIHELPER_SERIALIZATION))
#       ifndef NO_IMGUIHELPER_SERIALIZATION_SAVE
        bool TabLabelsSave(ImGuiHelper::Serializer& s,int selectedIndex,const int* pOptionalItemOrdering,int numTabs)   {
            if (!s.isValid()) return false;
            if (numTabs<0 || !pOptionalItemOrdering) numTabs=0;
            s.save(&numTabs,"TabLabelsNumTabs");
            s.save(&selectedIndex,"TabLabelsSelectedIndex");
            for (int i=0;i<numTabs;i+=4) {
                int num = numTabs-i;if (num>4) num=4;
                s.save(&pOptionalItemOrdering[i],"TabLabelsOrdering",num);
            }
            return true;
        }
        bool TabLabelsSave(const char* filename,int selectedIndex,const int* pOptionalItemOrdering,int numTabs) {
            ImGuiHelper::Serializer s(filename);
            return TabLabelsSave(s,selectedIndex,pOptionalItemOrdering,numTabs);
        }
#       endif //NO_IMGUIHELPER_SERIALIZATION_SAVE
#       ifndef NO_IMGUIHELPER_SERIALIZATION_LOAD
        struct TabLabelsParser {
            int* pSelectedIndex;int* pOptionalItemOrdering;int numTabs,numSavedTabs,cnt;
            TabLabelsParser(int* _pSelectedIndex,int* _pOptionalItemOrdering,int _numTabs) : pSelectedIndex(_pSelectedIndex),pOptionalItemOrdering(_pOptionalItemOrdering),numTabs(_numTabs),numSavedTabs(0),cnt(0) {}
            static bool Parse(ImGuiHelper::FieldType /*ft*/,int numArrayElements,void* pValue,const char* name,void* userPtr) {
                TabLabelsParser& P = *((TabLabelsParser*) userPtr);
                const int* pValueInt = (const int*) pValue;
                if (strcmp(name,"TabLabelsNumTabs")==0) P.numSavedTabs = *pValueInt;
                else if (strcmp(name,"TabLabelsSelectedIndex")==0)  {
                    if (P.pSelectedIndex) *P.pSelectedIndex = *pValueInt;
                    if (P.numSavedTabs==0) return true;
                }
                else if (strcmp(name,"TabLabelsOrdering")==0) {
                    for (int i=0;i<numArrayElements;i++) {
                        if (P.numSavedTabs>0 && P.pOptionalItemOrdering && P.cnt<P.numTabs)   P.pOptionalItemOrdering[P.cnt] = pValueInt[i];
                        P.cnt++;
                    }
                    if (P.cnt==P.numSavedTabs) return true;
                }
                return false;
            }
        };
        bool TabLabelsLoad(ImGuiHelper::Deserializer& d,int* pSelectedIndex,int* pOptionalItemOrdering,int numTabs,const char ** pOptionalBufferStart)  {
            if (!d.isValid()) return false;
            const char* amount = pOptionalBufferStart ? (*pOptionalBufferStart) : 0;
            TabLabelsParser parser(pSelectedIndex,pOptionalItemOrdering,numTabs);
            amount = d.parse(&TabLabelsParser::Parse,(void*)&parser,amount);
            if (pOptionalBufferStart) *pOptionalBufferStart = amount;
            return true;
        }
        bool TabLabelsLoad(const char* filename,int* pSelectedIndex,int* pOptionalItemOrdering,int numTabs) {
            ImGuiHelper::Deserializer d(filename);
            return TabLabelsLoad(d,pSelectedIndex,pOptionalItemOrdering,numTabs);
        }
#       endif //NO_IMGUIHELPER_SERIALIZATION_LOAD
#   endif //NO_IMGUIHELPER_SERIALIZATION
//--------------------------------------------------------------------------------


ImTextureID TabWindow::DockPanelIconTextureID = NULL;
void TabWindow::GetDockPanelIconImageRGBA(ImVector<unsigned char>& rgba_buffer_out, int* w_out, int* h_out) {
    // I have drawn all the icons that compose this image myself.
    // I took inspiration from the icons bundled with: https://github.com/dockpanelsuite/dockpanelsuite (that is MIT licensed).
    // So no copyright issues for this AFAIK.

    const int w = 128;const int h = 128;
    if (w_out) *w_out = w;
    if (h_out) *h_out = h;

    const unsigned int palette[54] = {	// each is a 4 bytes (RGBA) unsigned integer in little-endian encoding
                                        0U,4290098613U,4291413446U,4291282887U,4290624957U,4291743181U,4292138196U,4292467161U,4292598747U,4289686099U,4289625722U,4288176475U,4292203989U,4292664540U,4292795869U,4293624876U,4293232449U,4293302112U,4294626468U,4293256419U,4291201680U,4294760628U,4294760113U,4294827453U,4290678164U,4294828744U,4294503133U,4294901244U,4290815144U,4294306530U,
                                        4293518821U,4292729306U,4292596434U,4294967295U,4290295221U,4292203732U,4290097844U,4294829260U,4291652670U,4294696642U,16777216U,620954113U,1996883201U,2667906306U,1409614848U,3422816012U,3741452830U,4227862605U,4278196610U,4278198180U,4278265018U,4285032038U,4281484145U,4278199226U};

    unsigned char indices[w*h+1] = 	// [width x height (+ end string char)]: each (char-'0'), [skipping (char)92], points to a palette entry
            "00000000000000000000000000000111111111111111111111111111110000000000000000000000000000000000000001111111111111111111111111111100000000000000000000000000000001222333333333333333333333333100000000000000000000000000000000000000014233333333333333333333333331000000000000000000000000000000012335555555555555555555555551000000000000000000000000000000000000000125566666666666666666666666610000000000000000000000000000000123577777777777777777777777710000000000000000000000000000000000000001357888888888888888888888888100000000000000000000000000000001357999999999999999999::;<=>10000000000000000000000000000000000000001368999999999999999999::;<8>1000000000000000000000000000000013579?""?""?""?""?@@@@AAAAAAAAAB:47C100000000000000000000000000000000000000013689?""?""?""?""?@@@@AAAAAAAAAB:2<>10000000000000000000000000000000135"	\
            "79?""?""?""?""?@@@@AAAAAAAAAB:16C100000000000000000000000000000000000000013689?""?""?""?""?@@@@AAAAAAAAAB:46>100000000000000000000000000000001357DBBBBBBBBBBBBBBBBBBBD16C10000000000000000000000000000000000000001368DBBBBBBBBBBBBBBBBBBBD46>100000000000000000000000000000001357DEEEEEEFFFFFFFFFFFFFD16C10000000000000000000000000000000000000001368DEEEEEEFFFFFFFFFFFFFD46>100000000000000000000000000000001357DGGGGGGGGGGGGGGGGGGGH16C10000000000000000000000000000000000000001368DGGGGGGGGGGGGGGGGGGGH46>100000000000000000000000000000001357DIIIIIIIIIIIIIIIIIIIH16C10000000000000000000000000000000000000001368DIIIIIIIIIIIIIIIIIIIH46>100000000000000000000000000000001357DJJJJJJJJJJJJJJJJJJJD16C10000000000000000000000000000000000000001368DJJJJJJJJJJJJJJJJJJJD46>100000000000000000000000000000001357DJJJJJJJJJJJJJJJJJJJD16C1000000"	\
            "0000000000000000000000000000000001368DJJJJJJJJJJJJJJJJJJJD46>100000000000000000000000000000001357HKKKKKKKKKKKKKKKKKKKL16C10000000000000000000000000000000000000001368HKKKKKKKKKKKKKKKKKKKL46>100000000000000000000000000000001357DMNMJNMNMJOMNNMONMNMD16C10000000000000000000000000000000000000001368DMNMJNMNMJOMNNMONMNMD46>100000000000000000000000000000001357:O5ON5OPON5OOONPONPND16C10000000000000000000000000000000000000001368:O5ON5OPON5OOONPONPND46>100000000000000000000000000000001357HQQQQQQQQQQQQQQQQQQQL16C10000000000000000000000000000000000000001368HQQQQQQQQQQQQQQQQQQQL46>100000000000000000000000000000001357HQQQQQQQQQQQQQQQQQQQL16C10000000000000000000000000000000000000001368HQQQQQQQQQQQQQQQQQQQL46>100000000000000000000000000000001357HQQQQQQQQKLKQQQQQQQQL16C100000000000000000000000000000000000000"	\
            "01368HQQQQQQQQKLKQQQQQQQQL46>100000000000000000000000000000001357HQQQQQQQKD;LKQQQQQQQL16C10000000000000000000000000000000000000001368HQQQQQQQKD;LKQQQQQQQL46>100000000000000000000000000000001357HQQQQQQKL;;;LKQQQQQQL16C10000000000000000000000000000000000000001368HQQQQQQKL;;;LKQQQQQQL46>100000000000000000000000000000001357HQQQQQKL99999RKQQQQQL16C10000000000000000000000000000000000000001368HQQQQQKL99999RKQQQQQL46>100000000000000000000000000000001357HQQQQK2:::::::OQQQQQL16C10000000000000000000000000000000000000001368HQQQQK2:::::::OQQQQQL46>100000000000000000000000000000011357HQQQQKOPPPPPPPNQQQQQL16C11000000000000000000000000000000000000001368HQQQQKOPPPPPPPNQQQQQL46>100000000000000000000000000000122357HQQQQQQQQQQQQQQQQQQQL16C>=100000000000000000000000000000000000001368HQQQQQQQQQQQQQQQQQQQL46>100"	\
            "0000000000000000000000000012223S=HQQQQQQQQQQQQQQQQQQQL16CC>710000000000000000000000000000000000001368HQQQQQQQQQQQQQQQQQQQL46>10000000000000000000000000001223357>HQQQQQQQQQQQQQQQQQQQL16CCC>71000000000000000000000000000000000001368HQQQQQQQQQQQQQQQQQQQL46>1000000000000000000000000001223557>C;DDDDDDDDDDDDDDDDDDD916CCCC>7100000000000000000000000000000000001368;DDDDDDDDDDDDDDDDDDD946>100000000000000000000000001223577>CC74111111111111111111147CCCCC>710000000000000000000000000000000001368<244444444444444444442<>10011111111111111111111111122357>CCC>766666666666666666667>CCCCCC>711111111111111111111111100000000013688<6666666666666666666<8>1001222333333333333333333333357>CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC>7533333333333333333333310000000001368>>>>>>>>>>>>>>>>>>>>>>>>1001233555555555555555555555S7>CCCC"	\
            "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC>7S555555555555555555551000000000TTTTTTTTTTTTTTTTTTTTTTTTTTTTT001235777777777777777777777=>CCCCC;;;;99999999999999::::;7>CCCCCCCC>=77777777777777777777100000000000000000000000000000000000000001357999DDDDDDD:DDDDDDDDDDDD97>CC9?""?""?""?""?""?@@@@AAAAAAAAABB:47CCC9DDDDDDDDDDD:DDDDDDD:999<=>100000000T111111111111111111111111111111113579?""?BFGIJJJPKQQQQQQQQQQQD47CC9?""?""?""?""?""?@@@@AAAAAAAAABB:16CCCDQQQQQQQQQQQOMKJJIGEB?""?947C100000000T333333333333333333333333333324113579?""?BFGIJJJ2KQQQQQQQQQQQD16CC:IJPOJOMOOJOJOOJOOMOMOD16CCCDQQQQQQQQQQQ2OKJJUGEB?""?916C100000000T666666666666666666666666666552113579?""?BFGIJJJPKQQQQQQQQQQQD16CC:BBLLBLBLLBLBBLBLBBBBL:16CCCDQQQQQQQQQQQOMKJJIGEB?""?916C100000000T888888888888888888888888888753113579?""?BFGIJJMNKQQQQQQQQQQQD16CC:LBFFFFFFFFFFFFFFFFFFBD16CCCDQQQ"	\
            "QQQQQQQQNJKJJIGEB?""?916C100000000T>8<9DDDDDDDDDDD:DDDDDDD:999863113579?""?BFGIJJJ2KQQQQQQKKQQQD16CC:BFFFFFFFFFFFFFFFFFFFFD16CCCDQQQKKQQQQQQ5OKJJIGEB?""?916C100000000T><2DQQQQQQQQQQQOMKJJIGEB?""?9863113579@@BFGIJJJPKQQQQQKLMQQQD16CC:BFFFFFFFFFFFFFFFFFFEL:16CCCDQQQN2QQQQQQOMKJJIGEB@@916C100000000T>64DQQQQQQQQQQQ2OKJJUGEB?""?986311357V@@BFGIJJJPKQQQQKH:MQQQD16CC:BEEEEEEEEEEEEEEEEEEEBD16CCCDQQQP:LKQQQQPNKJJIGEB@@916C100000000T>64DQQQQQQQQQQQOMKJJIGEB?""?986311357V@@BFGIJJJPKQQQMH9:MQQQD16CCDJEEGGEGGGGGGGGGGGGGGBD16CCCDQQQP:9LKQQQONKJJIGEB@@916C100000000T>64DQQQQQQQQQQQNJKJJIGEB?""?986311357V@@BFGIJJMNKQQM:;9:MQQQD16CC:BGGGGGGGGGGGGGGGGGGGBD16CCCDQQQP:9;DKQQMJKJJIGEB@@916C100000000T>64DQQQKKQQQQQQ5OKJJIGEB?""?9863113579AABFGIJJJ2KQKD;;9:MQQQD16CC:BWWWWWWWWWWWWWWWWWWWFD16CCCDQQQP:9;;LQQ5OKJJIGFBAA916C100000000"	\
            "T>64DQQQN2QQQQQQOMKJJIGEB@@9863113579AABFGIJJJPKQQN:;9:MQQQD16CC:PIIIIIIIIIIIIIIIIIIUL:16CCCDQQQP:9;LKQQONKJJIGFBAA916C100000000T>64DQQQP:LKQQQQPNKJJIGEB@@9863113579AABFGIJJJPKQQQKH9:NQQQD16CC:2IUUUUUUUUUUUUUUUUUJ2D16CCCDQQQP:9LKQQQONKJJIGFBAA916C100000000T>64DQQQP:9LKQQQONKJJIGEB@@9863113579AABFGIJJJPKQQQQKL:NQQQD16CC:UJJJJJJJJJJJJJJJJJJJID16CCCDQQQP:LKQQQQPNKJJIGFBAA916C100000000T>64DQQQP:9;DKQQMJKJJIGEB@@9863113579AABFGIJJMNKQQQQQK2MQQQD16CC:PJJJJJJJJJJJJJJJJJJJLD16CCCDQQQNOKQQQQQNJKJJIGFBAA916C100000000T>64DQQQP:9;;LQQ5OKJJIGFBAA9863113579AABFGIJJJ2KQQQQQQQQQQQD16CC:2JJJJJJJJJJJJJJJJJJJ5D16CCCDQQQQQQQQQQQPOKJJIGFBAA916C100000000T>64DQQQP:9;LKQQONKJJIGFBAA9863113579AABFGIJJJPKQQQQQQQQQQQD16CC:MJJJJJ2UPUIPJIWUPJUW2D16CCCDQQQQQQQQQQQONKJJIGFBAA916C100000000T>64DQQQP:9LKQQQONKJJIGFBAA98631"	\
            "1357:AABFGIJJMOKQQQQQQQQQQQD16CC:PJJJJLO2R22L5R22L52LP:16CCCDQQQQQQQQQQQNMKJJIGFBAA:16C100000000T>64DQQQP:LKQQQQPNKJJIGFBAA986311357:AABFGIJJJPKQQQQQQQQQQQD16CC:PJJJJJN:2224R2;2RLLLR:16CCCDQQQQQQQQQQQPNKJJIGFBAA:16C100000000T>64DQQQNOKQQQQQNJKJJIGFBAA986311357:BBBEGIJJMOKQQQQQQQQQQQD16CC:PMJJJ2NLQQQQQQ:QQQQQQD16CCCDQQQQQQQQQQQNMKJJIGFBBB:16C100000000T>64DQQQQQQQQQQQPOKJJIGFBAA986311357;::DDDDDLL:LLLLLLLLLLLL:16CC:5J4LOLKHQQQQQQ:QQQQQQD16CCC:LLLLLLLLLLLDDLLDDDDDDD916C100000000T>64DQQQQQQQQQQQONKJJIGFBAA986311357<4111111111111111111111147CC9LLLLLLL:LLLLLL;LLLLLL947CCC74111111111111111111111147C100000000T>64DQQQQQQQQQQQNMKJJIGFBAA:86311357=766666666666666666666667>CC>54111111142411142411145>CCC>766666666666666666666667>C100000000T>64DQQQQQQQQQQQPNKJJIGFBAA:86311357>CCCCCCCCCCCCCCCCCCCCCCCCCCC"	\
            "C>766666667=76667=76667>CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC100000000T>64DQQQQQQQQQQQNMKJJIGFBBB:8631111111111111111111111111>CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC11111111111111111111111100000000T>64:LLLLLLLLLLLDDLLDDDDDDD98631000000000000000000000001=>CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC100000000000000000000000000000000T><244444444444444444444442<863100000000000000000000000017>CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC1000000000000000000000000000000000T>8<6666666666666666666666<88631000000000000000000000000017>CCCCC9DDDDDDDDDDDDDDDDDDD:7>CCCCC10000000000000000000000000000000000T>>>>>>>>>>>>>>>>>>>>>>>>>>>86310000000000000000000000000017>CCCCDQQQQQQQQQQQQQQQQQQQL47CCCC100000000000000000000000000000000000T111111111111111111111111111111100000000000000000000000000017>CCCDQQQQQQQQQQQQQQQQQQQL16CCC10000"	\
            "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000017>CCDQQQQQQQQQQQQQQQQQQQL16CC10000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000017>CDQQQQKMNNNNNNNMQQQQQL16C1000000000000000000000000000000000000000TTTTTTTTTTTTTTTTTTTTTTTTTTTTT0000000000000000000000000000000157>DQQQQK2:::::::5QQQQQL16C10000000000000000000000000000000000000001>>>>>>>>>>>>>>>>>>>>>>>>8631000000000000000000000000000000013S=DQQQQQKH99999HKQQQQQL16C10000000000000000000000000000000000000001>8<6666666666666666666<8863100000000000000000000000000000001357DQQQQQQKH;;;HMQQQQQQL16C10000000000000000000000000000000000000001><244444444444444444442<863100000000000000000000000000000001357DQQQQQQQM:;:KQQQQQQQL16C100000000000000000000000000000000000000"	\
            "01>649DDDDDDDDDDDDDDDDDDD:863100000000000000000000000000000001357DQQQQQQQQKDNQQQQQQQQL16C10000000000000000000000000000000000000001>64DQQQQQQQQQQQQQQQQQQQL863100000000000000000000000000000001357DQQQQQQQQQQQQQQQQQQQL16C10000000000000000000000000000000000000001>64DQQQQQQQQQQQQQQQQQQQL863100000000000000000000000000000001357DQQQQQQQQQQQQQQQQQQQL16C10000000000000000000000000000000000000001>64DQQQQQQQQQQQQQQQQQQQL863100000000000000000000000000000001357DKKKKKKKKKKKKKKKKKKKL16C10000000000000000000000000000000000000001>64DQQQQKMNNNNNNNMQQQQQL863100000000000000000000000000000001357:P2ON2P5PN2PPPN2POPOD16C10000000000000000000000000000000000000001>64DQQQQK2:::::::5QQQQQL863100000000000000000000000000000001357DJJJJJJJJJJJJJMJJJJML16C10000000000000000000000000000000000000001>64DQQQQQKH99999HKQQQQQL863100"	\
            "000000000000000000000000000001357DJJJJJJJJJJJJJJJJJJJL16C10000000000000000000000000000000000000001>64DQQQQQQKH;;;HMQQQQQQL863100000000000000000000000000000001357DJJJJJJJJJJJJJJJJJJJL16C10000000000000000000000000000000000000001>64DQQQQQQQM:;:KQQQQQQQL863100000000000000000000000000000001357DIIIIIIIIIIIIIIIIIIIH16C10000000000000000000000000000000000000001>64DQQQQQQQQKDNQQQQQQQQL863100000000000000000000000000000001357DGGGGGGGGGGGGGGGGGGGD16C10000000000000000000000000000000000000001>64DQQQQQQQQQQQQQQQQQQQL863100000000000000000000000000000001357DFFFFFFFFFFFFFFFFFFED16C10000000000000000000000000000000000000001>64DQQQQQQQQQQQQQQQQQQQL863100000000000000000000000000000001357DBBBBBBBBBBBBBBBBBBBD16C10000000000000000000000000000000000000001>64DKKKKKKKKKKKKKKKKKKKL86310000000000000000000000000000000135"	\
            "79?""?""?""?""?@@@@AAAAAAAAAB:16C10000000000000000000000000000000000000001>64:P2ON2P5PN2PPPN2POPOD8631000000000000000000000000000000013579?""?""?""?""?@@@@AAAAAAAAAB:16C10000000000000000000000000000000000000001>64DJJJJJJJJJJJJJMJJJJML8631000000000000000000000000000000013579999999VVV9999999:::;16C10000000000000000000000000000000000000001>64DJJJJJJJJJJJJJJJJJJJL863100000000000000000000000000000001357<4111111111111111111147C10000000000000000000000000000000000000001>64DJJJJJJJJJJJJJJJJJJJL863100000000000000000000000000000001357=766666666666666666667>C10000000000000000000000000000000000000001>64DIIIIIIIIIIIIIIIIIIIH863100000000000000000000000000000001357>CCCCCCCCCCCCCCCCCCCCCCC10000000000000000000000000000000000000001>64DGGGGGGGGGGGGGGGGGGGD8631000000000000000000000000000000011111111111111111111111111111000000"	\
            "0000000000000000000000000000000001>64DFFFFFFFFFFFFFFFFFFED86310000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001>64DBBBBBBBBBBBBBBBBBBBD86310000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001>649?""?""?""?""?@@@@AAAAAAAAAB:86310000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001><29?""?""?""?""?@@@@AAAAAAAAAB:86310000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001>8<9999999VVV9999999:::;86310000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001888888888888888888888888753100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"	\
            "016666666666666666666666665521000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000133333333333333333333333332410000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001111111111111111111111111111100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000XYZ[[[[Z]X00000000000001111111111111111111111111111111T00000000000000000000000000000000000000000000000000000000000000000000000YZ^_`aaaa`_^[Y000000000001368>>>>>>>>>>>>>>>>>>>>>>>>>>>T0000000000000000000000000000000000000000000000000000000000000000000000Z^`bccccccccb`_ZX00000000013688<6666666666666666666666<8>T"	\
            "00000000000000000000000000000000000000000000000000000000000000000000Y[`bccccccccccccb`^Y000000001368<244444444444444444444442<>T0000000000000000000000000000000000000000000000000000000000000000000Y^`cccccccccccccccca^Y00000001368999DDDDDDD:DDDDDDDDDDDD946>T0000000000000000000000000000000000000000LH:::dddddddeee00000000000X^`ccccccccfccccccccca^Y00000013689?""?BFGIJJJPKQQQQQQQQQQQD46>T000000000000000000000000000000000000000072TT::ddddddddd00000000000[`cccccca`___``bcccccca^X0000013689?""?BFGIJJJ2KQQQQQQQQQQQD46>T0000000000000000000000000000000000000000L3TT::dddddddde0000000000]_ccccca_^]YYY][^bcccccc`Z0000013689?""?BFGIJJJPKQQQQQQQQQQQD46>T000000000000000L<L0000000L7L00000000000000TT::ddddddd600000000000^acccca^]00000X[`cccccccb^Y000013689?""?BFGIJJMNKQQQQQQQQQQQD46>T000000000000000H22000000032H0000"	\
            "00000000000L::dddddd000000000000]_cccca^Y00000X[`ccccccccc`Z000013689?""?BFGIJJJ2KQQQQQQKKQQQD46>T000000000000000:TTT00000TTT:000000000000000L::dddddd000000000000[acccc_]00000X[`cccccacccca^000013689@@BFGIJJJPKQQQQQKLMQQQD46>T000000000000000:LLLLLLLLTTT:000000000000000L::dddddd00000000000X^bccc`[00000X[`ccccc`^bcccf_Y0001368V@@BFGIJJJPKQQQQKH:MQQQD46>T000000000000000:::::::::::::000000000000000L::dddddd00000000000Y_cccc_Y0000X[`ccccc`^[`cccc`]0001368V@@BFGIJJJPKQQQMH9:MQQQD46>T000000HHHHHHHHHd:::::::::::d000000000000000L::dddddd00000000000]`cccb^X000X[`ccccc`^Y]`cccc`Z0001368V@@BFGIJJMNKQQM:;9:MQQQD46>T0000d4444442222ddddddddddddd00000000000000TL::dddddddd000000000]`ccca[000X[`ccccc`^Y0Y_cccc`Z00013689AABFGIJJJ2KQKD;;9:MQQQD46>T00e::::HHHHHHHHddddddddddddd000000000000L2TL::dddddddde00000000]"	\
            "`ccca[00X[`ccccc`^Y00Y_cccc`Z00013689AABFGIJJJPKQQN:;9:MQQQD46>T000e:::::::::::ddddddddddddd000000000000<2TL::ddddddddd00000000]`ccca^0X[`ccccc`^Y000Y_cccc`Z00013689AABFGIJJJPKQQQKH9:NQQQD46>T00000e:::::::::ddddddddddddd000000000000LH:::dddddddeee00000000Y_cccb_Y[`ccccc`^Y0000]`cccc`]00013689AABFGIJJJPKQQQQKL:NQQQD46>T000000000000000ddddddddddddd00000000000000000H2H::0000000000000X^bccc`^`ccccc`^Y00000[acccc_Y00013689AABFGIJJMNKQQQQQK2MQQQD46>T000000000000000ddddddddddddd00000000000000000H2H::00000000000000[accca`ccccc`^Y00000]_ccccb^X00013689AABFGIJJJ2KQQQQQQQQQQQD46>T000000000000000eddd00000ddde00000000000000000H2H::00000000000000]`ccccccccc`^Y00000Y^acccc`Z000013689AABFGIJJJPKQQQQQQQQQQQD46>T000000000000000eddd000006dde00000000000000000H2H::00000000000000X^accccccc`^Y00000Y^accccb_Y0000"	\
            "1368:AABFGIJJMOKQQQQQQQQQQQD46>T000000000000000ede0000000ede00000000000000000H4H::000000000000000Z`cccccc`^Y0000YZ_accccc`[000001368:AABFGIJJJPKQQQQQQQQQQQD46>T000000000000000000000000000000000000000000000H4H::0000000000000000[accccca`^[[[^_`bccccca^Y000001368:BBBEGIJJMOKQQQQQQQQQQQD2<>T000000000000000000000000000000000000000000000H4H::0000000000000000Y^accccccbaaaabccccccb_]0000001368;::DDDDDLL:LLLLLLLLLLLL:<8>T000000000000000000000000000000000000000000000H4H::00000000000000000Y^accccccccccccccccb_]00000001357888888888888888888888888888T000000000000000000000000000000000000000000000H4:::000000000000000000Y^`cccccccccccccca_]000000001255666666666666666666666666666T00000000000000000000000000000000000000000000004::e0000000000000000000Y[_acccccccccca`[Y0000000001423333333333333333333333333333T"	\
            "0000000000000000000000000000000000000000000000d::0000000000000000000000][_`aabbba`_^]000000000001111111111111111111111111111111T00000000000000000000000000000000000000000000000:e000000000000000000000000Y][^^^^[ZY00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000e00000000000000000000000000000XX0000000000000000000000000000000000000000000000000";

    // Decode 'indices':
    const unsigned num_indices = sizeof(indices)/sizeof(indices[0])-1;  // or strlen(indices);
    IM_ASSERT(w*h==num_indices);
    for (unsigned i=0;i<num_indices;i++)	{
        unsigned char& c=indices[i];
        if (c>=']') c-='1'; // This skips the backslash char
        else c-='0';
    }

    // Fill 'rgba_buffer_out'
    rgba_buffer_out.resize(w*h*4);
    int idx;unsigned int pal;const unsigned char *pPal;unsigned char *pRGBA=(unsigned char *) &rgba_buffer_out[0];
    for (unsigned i=0,isz=w*h;i<isz;i++)    {
        idx = indices[i];
        pal = palette[idx];

        pPal=(unsigned char*) &pal;

        // debug line:
        //printf("[");for (int c=0;c<4;c++) printf("%X",pPal[c]);printf("]");if (i%wOut==(wOut-1)) {printf("\n");fflush(stdout);}
#               ifndef IM_BIG_ENDIAN
        for (int c=0;c<4;c++) *pRGBA++=*pPal++;     // This works only on little-endian machines
#               else
        for (int c=3;c>=0;--c) *pRGBA++=pPal[c];  // This works only on big-endian machines (maybe)
#               endif
        //}
    }

    // TODO: Add optional code guarded by the definition IMGUITABWINDOW_RGBA_FLIP_Y or something like that is somebody request it
}


} // namespace ImGui



namespace ImGui {

//=======================================================================================
// Main method to draw the tab label
// The TabLabelStyle used by this method won't be merged with the Window Alpha (please provide a pOptionalStyleToUseIn using TabLabelStyle::GetMergedWithWindowAlpha() if needed).
static bool TabButtonVertical(bool rotateCCW,const char *label, bool selected, bool *pCloseButtonPressedOut=NULL, const char* textOverrideIn=NULL, ImVec2 *pJustReturnItsSizeHereOut=NULL, const TabLabelStyle* pOptionalStyleToUseIn=NULL,ImFont *fontOverride=NULL, ImVec2 *pOptionalJustDrawTabButtonGraphicsUnderMouseWithThisOffset=NULL, ImDrawList *drawListOverride=NULL,bool privateReuseLastCalculatedLabelSizeDoNotUse = false,bool forceActiveColorLook = false,bool invertRounding=false)  {
    // Based on ImGui::ButtonEx(...)
    bool *pHoveredOut = NULL;           // removed from args (can be queried from outside)
    bool *pCloseButtonHovered = NULL;   // removed from args (who cares if the close button is hovered?)
    const int flags = 0;                // what's this ?
    const bool hasCloseButton = pCloseButtonHovered || pCloseButtonPressedOut;

    const bool isFakeControl = pOptionalJustDrawTabButtonGraphicsUnderMouseWithThisOffset || pJustReturnItsSizeHereOut;

    ImGuiWindow* window = GetCurrentWindow();
    if (window && window->SkipItems && !isFakeControl)  return false;

    //ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = ImGui::GetStyle();
    const TabLabelStyle& tabStyle = pOptionalStyleToUseIn ? *pOptionalStyleToUseIn : TabLabelStyle::Get();
    const ImGuiID id = isFakeControl ? 0 : window->GetID(label);
    if (textOverrideIn) label = textOverrideIn;

    if (!fontOverride) fontOverride = (ImFont*) (selected ? TabLabelStyle::ImGuiFonts[tabStyle.fontStyles[TabLabelStyle::TAB_STATE_SELECTED]] : TabLabelStyle::ImGuiFonts[tabStyle.fontStyles[TabLabelStyle::TAB_STATE_NORMAL]]);
    if (fontOverride) ImGui::PushFont(fontOverride);
    static ImVec2 staticLabelSize(0,0);
    ImVec2 label_size(0,0);
    if (!privateReuseLastCalculatedLabelSizeDoNotUse) label_size = staticLabelSize = ImGui::VerticalTextHelper::CalcVerticalTextSize(label, NULL, true);
    else label_size = staticLabelSize;

    ImVec2 pos = window ? window->DC.CursorPos : ImVec2(0,0);
    if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrLineTextBaseOffset)    pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
    ImVec2 size(label_size.x + (style.FramePadding.x+tabStyle.borderWidth) * 2.0f, label_size.y + (style.FramePadding.y+tabStyle.borderWidth) * 2.0f);

    float btnSize = label_size.x*0.75f,btnSpacingY = label_size.x*0.25f;
    float extraWidthForBtn = hasCloseButton ? (btnSpacingY*2.f+btnSize) : 0;
    if (hasCloseButton) size.y+=extraWidthForBtn;
    if (pJustReturnItsSizeHereOut) {*pJustReturnItsSizeHereOut=size;if (fontOverride) ImGui::PopFont();return false;}

    const ImRect bb(pOptionalJustDrawTabButtonGraphicsUnderMouseWithThisOffset ? *pOptionalJustDrawTabButtonGraphicsUnderMouseWithThisOffset : pos,
                    (pOptionalJustDrawTabButtonGraphicsUnderMouseWithThisOffset ? *pOptionalJustDrawTabButtonGraphicsUnderMouseWithThisOffset : pos) + size);
    if (!pOptionalJustDrawTabButtonGraphicsUnderMouseWithThisOffset) {
        ItemSize(bb, 0.f);//style.FramePadding.y);
        if (!ItemAdd(bb, id)) {if (fontOverride) ImGui::PopFont();return false;}
    }

    //if (window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat) flags |= ImGuiButtonFlags_Repeat;    // What's this ?
    bool hovered=false, held=false;
    bool pressed = isFakeControl ? false : ButtonBehavior(bb, id, &hovered, &held, flags);
    bool btnHovered = false;
    bool btnPressed = false;
    ImVec2 startBtn(0,0),endBtn(0,0);
    if (hasCloseButton)    {
        //startBtn = ImVec2(bb.Max.x-extraWidthForBtn+btnSpacingY*0.5f,bb.Min.y+(size.y-btnSize)*0.5f);
        //endBtn = ImVec2(startBtn.x+btnSize,startBtn.y+btnSize);
        startBtn = ImVec2(bb.Min.x+(size.x-btnSize)*0.5f,rotateCCW ? (bb.Min.y+btnSpacingY*1.5f/*extraWidthForBtn-btnSpacingY*0.5f*/) : (bb.Max.y-extraWidthForBtn+btnSpacingY*0.5f));
        endBtn = ImVec2(startBtn.x+btnSize,startBtn.y+btnSize);
        if (!isFakeControl) {
            btnHovered = hovered && ImGui::IsMouseHoveringRect(startBtn,endBtn);
            btnPressed = pressed && btnHovered;
            if (btnPressed) pressed = false;
            if (pCloseButtonHovered) *pCloseButtonHovered = btnHovered;
            if (pCloseButtonPressedOut) * pCloseButtonPressedOut = btnPressed;
        }
    }
    if (pHoveredOut) *pHoveredOut = hovered && !btnHovered;  // We may choose not to return "hovered" when the close btn is hovered.
    if (forceActiveColorLook) {hovered = held = true;}

    // Render

    const ImU32 col = (hovered && !btnHovered && held) ? tabStyle.colors[selected ? TabLabelStyle::Col_TabLabelSelectedActive : TabLabelStyle::Col_TabLabelActive] : (hovered && !btnHovered) ? tabStyle.colors[selected ? TabLabelStyle::Col_TabLabelSelectedHovered : TabLabelStyle::Col_TabLabelHovered] : tabStyle.colors[selected ? TabLabelStyle::Col_TabLabelSelected : TabLabelStyle::Col_TabLabel];
    const ImU32 colText = tabStyle.colors[selected ? TabLabelStyle::Col_TabLabelSelectedText : TabLabelStyle::Col_TabLabelText];

    if (!drawListOverride) drawListOverride = window->DrawList;

    // Canvas
    if (rotateCCW) ImGui::DrawListHelper::ImDrawListAddRectWithHorizontalGradient(drawListOverride,bb.Min, bb.Max,col,(selected || hovered || held)?tabStyle.fillColorGradientDeltaIn0_05:(-tabStyle.fillColorGradientDeltaIn0_05),tabStyle.colors[selected ? TabLabelStyle::Col_TabLabelSelectedBorder : TabLabelStyle::Col_TabLabelBorder],tabStyle.rounding,invertRounding ? (2|4) : (1|8),tabStyle.borderWidth);
    else ImGui::DrawListHelper::ImDrawListAddRectWithHorizontalGradient(drawListOverride,bb.Min, bb.Max,col,(selected || hovered || held)?(-tabStyle.fillColorGradientDeltaIn0_05):tabStyle.fillColorGradientDeltaIn0_05,tabStyle.colors[selected ? TabLabelStyle::Col_TabLabelSelectedBorder : TabLabelStyle::Col_TabLabelBorder],tabStyle.rounding,invertRounding ? (1|8) : (2|4),tabStyle.borderWidth);

    // Text
    ImGui::PushStyleColor(ImGuiCol_Text,ImGui::ColorConvertU32ToFloat4(colText));
    if (!pOptionalJustDrawTabButtonGraphicsUnderMouseWithThisOffset)  {
        if (!rotateCCW)
            VerticalTextHelper::RenderTextVerticalClipped(
                        bb.Min,
                        ImVec2(bb.Max.x,bb.Max.y-extraWidthForBtn),//ImVec2(bb.Max.x-extraHeightForBtn,bb.Max.y),
                        label, NULL, &label_size, ImVec2(0.5f,0.5f),NULL,NULL,rotateCCW);
        else
            VerticalTextHelper::RenderTextVerticalClipped(
                        ImVec2(bb.Min.x,bb.Min.y+extraWidthForBtn),
                        bb.Max,
                        label, NULL, &label_size, ImVec2(0.5f,0.5f),NULL,NULL,rotateCCW);
    }
    else    {
        //ImVec2 textPos(bb.Min.x+(bb.Max.x-bb.Min.x-label_size.x-extraHeightForBtn)*0.5f,bb.Min.y+(bb.Max.y-bb.Min.y-label_size.y)*0.5f);
        ImVec2 textPos(bb.Min.x+(bb.Max.x-bb.Min.x-label_size.x)*0.5f,
                       rotateCCW ?
                       (bb.Max.y-(bb.Max.y-bb.Min.y-label_size.y-extraWidthForBtn)*0.5f)
                       :
                       (bb.Min.y+(bb.Max.y-bb.Min.y-label_size.y-extraWidthForBtn)*0.5f)
                       );
        VerticalTextHelper::AddTextVertical(drawListOverride,textPos,colText,label,NULL,rotateCCW);
    }
    ImGui::PopStyleColor();



    //fprintf(stderr,"bb.Min=%d,%d bb.Max=%d,%d label_size=%d,%d extraWidthForBtn=%d\n",(int)bb.Min.x,(int)bb.Min.y,(int)bb.Max.x,(int)bb.Max.y,(int)label_size.x,(int)label_size.y,(int)extraWidthForBtn);
    if (hasCloseButton) {
    const ImU32 col = (held && btnHovered) ? tabStyle.colors[TabLabelStyle::Col_TabLabelCloseButtonActive] : btnHovered ? tabStyle.colors[TabLabelStyle::Col_TabLabelCloseButtonHovered] : 0;
    if (btnHovered) DrawListHelper::ImDrawListAddRect(drawListOverride,startBtn, endBtn, col,tabStyle.colors[TabLabelStyle::Col_TabLabelCloseButtonBorder],tabStyle.closeButtonRounding,0x0F,tabStyle.closeButtonBorderWidth);

        const float cross_extent = (btnSize * 0.5f * 0.7071f);// - 1.0f;
        const ImVec2 center((startBtn.x+endBtn.x)*0.5f,(startBtn.y+endBtn.y)*0.5f);
        const ImU32 cross_col = tabStyle.colors[(btnHovered) ? TabLabelStyle::Col_TabLabelCloseButtonTextHovered : selected ? TabLabelStyle::Col_TabLabelSelectedText : TabLabelStyle::Col_TabLabelText];
        drawListOverride->AddLine(center + ImVec2(+cross_extent,+cross_extent), center + ImVec2(-cross_extent,-cross_extent), cross_col,tabStyle.closeButtonTextWidth);
        drawListOverride->AddLine(center + ImVec2(+cross_extent,-cross_extent), center + ImVec2(-cross_extent,+cross_extent), cross_col,tabStyle.closeButtonTextWidth);

    }
    if (fontOverride) ImGui::PopFont();

    return pressed;
}
//========================================================================================


bool TabLabelsVertical(bool textIsRotatedCCW, int numTabs, const char** tabLabels, int& selectedIndex, const char** tabLabelTooltips, int* pOptionalHoveredIndex, int* pOptionalItemOrdering, bool allowTabReorder, bool allowTabClosing, int* pOptionalClosedTabIndex, int * pOptionalClosedTabIndexInsideItemOrdering, bool invertRounding)    {
    ImGuiStyle& style = ImGui::GetStyle();
    const TabLabelStyle& tabStyle = TabLabelStyle::GetMergedWithWindowAlpha();

    const ImVec2 itemSpacing =  style.ItemSpacing;
    style.ItemSpacing.x =       1;
    style.ItemSpacing.y =       1;

    if (numTabs>0 && (selectedIndex<0 || selectedIndex>=numTabs)) {
        if (!pOptionalItemOrdering)  selectedIndex = 0;
        else selectedIndex = -1;
    }
    if (pOptionalHoveredIndex) *pOptionalHoveredIndex = -1;
    if (pOptionalClosedTabIndex) *pOptionalClosedTabIndex = -1;
    if (pOptionalClosedTabIndexInsideItemOrdering) *pOptionalClosedTabIndexInsideItemOrdering = -1;

    //float sumY=0.f;
    //float windowWidth = 0.f;
    //if (wrapMode) windowWidth = ImGui::GetWindowWidth() - style.WindowPadding.x - (ImGui::GetScrollMaxY()>0 ? style.ScrollbarSize : 0.f);

    static int draggingTabIndex = -1;int draggingTabTargetIndex = -1;   // These are indices inside pOptionalItemOrdering
    static bool draggingTabWasSelected = false;
    static ImVec2 draggingTabSize(0,0);
    static ImVec2 draggingTabOffset(0,0);
    static bool draggingLocked = false;

    const bool isRMBclicked = ImGui::IsMouseClicked(1);
    const bool isMouseDragging = ImGui::IsMouseDragging(0,3.f);
    const bool isMouseDraggingJustStarted = isMouseDragging && (ImGui::GetIO().MouseDownDuration[0] < 0.35f);// ImGui::GetIO().MouseDown[0] does not work!
    int justClosedTabIndex = -1,newSelectedIndex = selectedIndex;

    ImVec2 startGroupCursorPos = ImGui::GetCursorPos();
    ImGui::BeginGroup();
    //ImVec2 tabButtonSz(0,0);
    bool mustCloseTab = false;bool canUseSizeOptimization = false;
    const bool isWindowHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
    bool selection_changed = false;//bool noButtonDrawn = true;
    for (int j = 0,i; j < numTabs; j++)
    {
        i = pOptionalItemOrdering ? pOptionalItemOrdering[j] : j;
        if (i==-1) continue;

        //if (!wrapMode)
        {
            //if (!noButtonDrawn) ImGui::SameLine();
            canUseSizeOptimization=false;
        }
        /*else if (sumX > 0.f) {
            sumX+=style.ItemSpacing.x;   // Maybe we can skip it if we use SameLine(0,0) below
            ImGui::TabButtonVertical(tabLabels[i],(i == selectedIndex),allowTabClosing ? &mustCloseTab : NULL,NULL,&tabButtonSz,&tabStyle);
            sumX+=tabButtonSz.x;
            if (sumX>windowWidth) sumX = 0.f;
            //else ImGui::SameLine();
            canUseSizeOptimization = true;
        }
        else canUseSizeOptimization = false;*/

        // Draw the button
        ImGui::PushID(i);   // otherwise two tabs with the same name would clash.
        if (ImGui::TabButtonVertical(textIsRotatedCCW,tabLabels[i],i == selectedIndex,allowTabClosing ? &mustCloseTab : NULL,NULL,NULL,&tabStyle,NULL,NULL,NULL,canUseSizeOptimization,false,invertRounding))   {
            selection_changed = (selectedIndex!=i);
            newSelectedIndex = i;
        }
        ImGui::PopID();
        //noButtonDrawn = false;

        /*if (wrapMode) {
            if (sumX==0.f) sumX = style.WindowPadding.x + ImGui::GetItemRectSize().x; // First element of a line
        }
        else if (isMouseDragging && allowTabReorder && pOptionalItemOrdering) {
            // We still need sumX
            if (sumY==0.f) sumY = style.WindowPadding.y + ImGui::GetItemRectSize().y; // First element of a line
            else sumY+=style.ItemSpacing.y + ImGui::GetItemRectSize().y;

        }*/

        if (isWindowHovered && ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly) && !mustCloseTab) {
            if (pOptionalHoveredIndex) *pOptionalHoveredIndex = i;
            if (tabLabelTooltips && !isRMBclicked && tabLabelTooltips[i] && strlen(tabLabelTooltips[i])>0)  ImGui::SetTooltip("%s",tabLabelTooltips[i]);

            if (pOptionalItemOrdering)  {
                if (allowTabReorder)  {
                    if (isMouseDragging) {
                        if (draggingTabIndex==-1 && !draggingLocked && isMouseDraggingJustStarted) {
                            draggingTabIndex = j;
                            draggingTabWasSelected = (i == selectedIndex);
                            draggingTabSize = ImGui::GetItemRectSize();
                            const ImVec2& mp = ImGui::GetIO().MousePos;
                            const ImVec2 draggingTabCursorPos = ImGui::GetCursorPos();
                            draggingTabOffset=ImVec2(
                                        mp.x-draggingTabSize.x*0.5f-draggingTabCursorPos.x+ImGui::GetScrollX(),
                                        mp.y+draggingTabSize.y*0.5f-draggingTabCursorPos.y+ImGui::GetScrollY()
                                        );

                        }
                    }
                    else if (draggingTabIndex>=0 && draggingTabIndex<numTabs && draggingTabIndex!=j){
                        draggingTabTargetIndex = j; // For some odd reasons this seems to get called only when draggingTabIndex < i ! (Probably during mouse dragging ImGui owns the mouse someway and sometimes ImGui::IsItemHovered() is not getting called)
                    }
                }
            }
        }
        if (mustCloseTab)   {
            justClosedTabIndex = i;
            if (pOptionalClosedTabIndex) *pOptionalClosedTabIndex = i;
            if (pOptionalClosedTabIndexInsideItemOrdering) *pOptionalClosedTabIndexInsideItemOrdering = j;
			if (pOptionalItemOrdering) pOptionalItemOrdering[j] = -1;
        }

    }
    selectedIndex = newSelectedIndex;
    ImGui::EndGroup();
    ImVec2 groupSize = ImGui::GetItemRectSize();

    // Draw tab label while mouse drags it
    if (draggingTabIndex>=0 && draggingTabIndex<numTabs) {
        const ImVec2 wp = ImGui::GetWindowPos();
        startGroupCursorPos.x+=wp.x;
        startGroupCursorPos.y+=wp.y;
        startGroupCursorPos.x-=ImGui::GetScrollX();
        startGroupCursorPos.y-=ImGui::GetScrollY();
        const float deltaX = groupSize.x;
        startGroupCursorPos.x-=deltaX;
        groupSize.x+=2.f*deltaX;
        if (ImGui::IsMouseHoveringRect(startGroupCursorPos,startGroupCursorPos+groupSize))  {
            const ImVec2& mp = ImGui::GetIO().MousePos;
            ImVec2 start(wp.x+mp.x-draggingTabOffset.x-draggingTabSize.x*0.5f,wp.y+mp.y-draggingTabOffset.y-draggingTabSize.y*0.5f);
            //const ImVec2 end(start.x+draggingTabSize.x,start.y+draggingTabSize.y);
            ImDrawList* drawList = ImGui::GetForegroundDrawList();
            const TabLabelStyle& tabStyle = TabLabelStyleGetMergedWithAlphaForOverlayUsage();
            ImFont* fontOverride = (ImFont*) (draggingTabWasSelected ? TabLabelStyle::ImGuiFonts[tabStyle.fontStyles[TabLabelStyle::TAB_STATE_SELECTED]] : TabLabelStyle::ImGuiFonts[tabStyle.fontStyles[TabLabelStyle::TAB_STATE_NORMAL]]);
            ImGui::TabButtonVertical(textIsRotatedCCW,tabLabels[pOptionalItemOrdering[draggingTabIndex]],draggingTabWasSelected,allowTabClosing ? &mustCloseTab : NULL,NULL,NULL,&tabStyle,fontOverride,&start,drawList,false,true,invertRounding);
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);

            if (TabWindow::DockPanelIconTextureID)	{
                // Optional: draw prohibition sign when dragging too far (you can remove this if you want)
                startGroupCursorPos.x+=deltaX*.5f;
                groupSize.x-=deltaX;
                if (!ImGui::IsMouseHoveringRect(startGroupCursorPos,startGroupCursorPos+groupSize))  {
                    const float signSize = draggingTabSize.x*1.25f;
                    start.x+=(draggingTabSize.x-signSize)*0.5f;
                    start.y+=(draggingTabSize.y-signSize)*0.5f;
                    const ImVec2 end(start.x+signSize,start.y+signSize);
                    const ImVec4 color(1.f,1.f,1.f,0.85f);
                    drawList->AddImage(TabWindow::DockPanelIconTextureID,start,end,ImVec2(0.5f,0.75f),ImVec2(0.75f,1.f),ImGui::ColorConvertFloat4ToU32(color));
                }
            }
        }
        else {
            draggingTabIndex = -1;draggingTabTargetIndex=-1;
            draggingLocked = true;// consume one mouse release
        }
    }

    // Drop tab label
    if (draggingTabTargetIndex!=-1) {
        // swap draggingTabIndex and draggingTabTargetIndex in pOptionalItemOrdering
        const int tmp = pOptionalItemOrdering[draggingTabTargetIndex];
        pOptionalItemOrdering[draggingTabTargetIndex] = pOptionalItemOrdering[draggingTabIndex];
        pOptionalItemOrdering[draggingTabIndex] = tmp;
        //fprintf(stderr,"%d %d\n",draggingTabIndex,draggingTabTargetIndex);
        draggingTabTargetIndex = draggingTabIndex = -1;
    }

    // Reset draggingTabIndex if necessary
    if (!isMouseDragging) {draggingTabIndex = -1;draggingLocked=false;}

    // Change selected tab when user closes the selected tab
    if (selectedIndex == justClosedTabIndex && selectedIndex>=0)    {
        selectedIndex = -1;
        for (int j = 0,i; j < numTabs; j++) {
            i = pOptionalItemOrdering ? pOptionalItemOrdering[j] : j;
            if (i==-1) continue;
            selectedIndex = i;
            break;
        }
    }

    // Restore the style
    style.ItemSpacing =                     itemSpacing;

    return selection_changed;
}

float CalcVerticalTabLabelsWidth()  {
    return  ImGui::GetFontSize() + (ImGui::GetStyle().FramePadding.y+ImGui::TabLabelStyle::Get().borderWidth) * 2.0f;
}

} //namespace ImGui

