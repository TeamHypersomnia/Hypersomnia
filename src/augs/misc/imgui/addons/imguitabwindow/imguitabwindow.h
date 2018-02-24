#ifndef IMGUITABWINDOW_H_
#define IMGUITABWINDOW_H_

#ifndef IMGUI_API
#include <imgui.h>
#endif //IMGUI_API

// This addon is available here:                        https://gist.github.com/Flix01/2cdf1db8d936100628c0
// and is bundled in the "ImGui Addons Branch" here:    https://github.com/Flix01/imgui/tree/2015-10-Addons
// Wiki about the "ImGui Addons Branch" is here:        https://github.com/Flix01/imgui/wiki/ImGui-Addons-Branch-Home


// USAGE:
/*
1) In the main "ImGui loop":
----------------------------
    static bool open = true;
    if (ImGui::Begin("Main", &open, ImVec2(400,600),-1.f,ImGuiWindowFlags_NoScrollbar))  {

        static ImGui::TabWindow tabWindow;
        if (!tabWindow.isInited()) {
            static const char* tabNames[] = {"Render","Layers","Scene","World","Object","Constraints","Modifiers","Data","Material","Texture","Particle","Physics"};
            static const int numTabs = sizeof(tabNames)/sizeof(tabNames[0]);
            static const char* tabTooltips[numTabs] = {"Render Tab Tooltip","Layers Tab Tooltip","Scene Tab Tooltip","","Object Tab Tooltip","","","","","Tired to add tooltips...",""};
            for (int i=0;i<numTabs;i++) {
                tabWindow.addTabLabel(tabNames[i],tabTooltips[i]); // see additional args to prevent a tab from closing and from dragging
            }
        }

        tabWindow.render(); // Must be called inside "its" window (and sets isInited() to false)
    }
    ImGui::End();

    // Optional add other ImGui::Window-TabWindow pairs here

2) At init time:
----------------
    // Callbacks:
    ImGui::TabWindow::SetWindowContentDrawerCallback(&TabContentProvider,NULL); // If not set TabWindowLabel::render() will be called instead (if you prefer extending TabWindowLabel)
    ImGui::TabWindow::SetTabLabelPopupMenuDrawerCallback(&TabLabelPopupMenuProvider,NULL);  // Optional (if you need context-menu)
    //ImGui::TabWindow::SetTabLabelDeletingCallback(&OnTabLabelDeleting); // Optional (to delete your userPts)
    //ImGui::TabWindow::SetTabLabelGroupPopupMenuDrawerCallback(&TabLabelGroupPopupMenuProvider,NULL);  // Optional (if you need context-menu when you right-click on an empty spot in the tab area)


    // Texture Loading (*)
    if (!ImGui::TabWindow::DockPanelIconTextureID)  {
		ImVector<unsigned char> rgba_buffer;int w,h;
        ImGui::TabWindow::GetDockPanelIconImageRGBA(rgba_buffer,&w,&h); // 4 channels, no additional stride between lines, => rgba_buffer.size() = 4*w*h
        ImGui::TabWindow::DockPanelIconTextureID = reinterpret_cast<ImTextureID>(MyTextureFromMemoryRGBAMethod(&rgba_buffer[0],w,h));  // User must turn raw RBGA to texture (using GetDockPanelIconImageRGBA).
    }

    // Optional Style
    //ImGui::TabLabelStyle& tabStyle = ImGui::TabLabelStyle::Get();
    // ... modify tabStyle ...

3) At deinit time:
-------------------
    Free: ImGui::TabWindow::DockPanelIconTextureID if not NULL. (*)

4) Finally, in the global scope (at the top of you code), define the callbacks:
-------------------------------------------------------------------------------
void TabContentProvider(ImGui::TabWindow::TabLabel* tab,ImGui::TabWindow& parent,void* userPtr) {
    // Users will use tab->userPtr here most of the time
    ImGui::Spacing();ImGui::Separator();
    if (tab) ImGui::Text("Here is the content of tab label: \"%s\"\n",tab->getLabel());
    else {ImGui::Text("EMPTY TAB LABEL DOCKING SPACE.");ImGui::Text("PLEASE DRAG AND DROP TAB LABELS HERE!");}
    ImGui::Separator();ImGui::Spacing();
}
// Optional (tab label context-menu)
void TabLabelPopupMenuProvider(ImGui::TabWindow::TabLabel* tab,ImGui::TabWindow& parent,void* userPtr) {
    if (ImGui::BeginPopup(ImGui::TabWindow::GetTabLabelPopupMenuName()))   {
        ImGui::PushID(tab);
        ImGui::Text("\"%.*s\" Menu",(int)(strlen(tab->getLabel())-(tab->getModified()?1:0)),tab->getLabel());
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::MenuItem("Entry 1");
        ImGui::MenuItem("Entry 2");
        ImGui::MenuItem("Entry 3");
        ImGui::MenuItem("Entry 4");
        ImGui::MenuItem("Entry 5");
        ImGui::PopID();
        ImGui::EndPopup();
    }

}
// Optional (delete TabLabel::userPtr when TabLabel gets deleted)
void OnTabLabelDeleting(ImGui::TabWindow::TabLabel* tab) {
    // Use this callback to delete tab->userPtr if you used it
}
// Optional (this is fired when RMB is clicked on an empty spot in the tab area)
void TabLabelGroupPopupMenuProvider(ImVector<ImGui::TabWindow::TabLabel*>& tabs,ImGui::TabWindow& parent,ImGui::TabWindowNode* tabNode,void* userPtr) {
    ImGui::PushStyleColor(ImGuiCol_WindowBg,ImGui::ColorConvertU32ToFloat4(ImGui::TabLabelStyle::Get().colors[ImGui::TabLabelStyle::Col_TabLabel]));
    ImGui::PushStyleColor(ImGuiCol_Text,ImGui::ColorConvertU32ToFloat4(ImGui::TabLabelStyle::Get().colors[ImGui::TabLabelStyle::Col_TabLabelText]));
    if (ImGui::BeginPopup(ImGui::TabWindow::GetTabLabelGroupPopupMenuName()))   {
        ImGui::Text("TabLabel Group Menu");
        ImGui::Separator();
        if (parent.isTabNodeMergeble(tabNode) && ImGui::MenuItem("Merge with parent group")) parent.mergeTabNode(tabNode); // Warning: this invalidates "tabNode" after the call
        if (ImGui::MenuItem("Close all tabs in this group")) {
            for (int i=0,isz=tabs.size();i<isz;i++) {
                ImGui::TabWindow::TabLabel* tab = tabs[i];
                if (tab->isClosable())  // otherwise even non-closable tabs will be closed
                {
                    parent.removeTabLabel(tab);
                    //tab->mustCloseNextFrame = true;  // alternative way... this asks for saving if file is modified
                }
            }
        }
        ImGui::EndPopup();
    }
    ImGui::PopStyleColor(2);
}

TIPS ABOUT TEXTURE LOADING;
(*): -> Texture loading/freeing is mandatory only if you're not using an IMGUI_USE_XXX_BINDING, or if you don't know
        what IMGUI_USE_XXX_BINDING is.
     -> If you prefer loading the texture from an external image, I'll provide it here: https://gist.github.com/Flix01/2cdf1db8d936100628c0
     -> Since internally we use texcoords, we had to choose a single convention for them. That means that it might be necessary for
        some people to flip the image RGBA upside down (we can provide some definitions for that if needed).

ADDITIONAL (OPTIONAL) CALLBACKS:
static void TabWindow::SetTabLabelFactoryCallback(TabLabelFactoryCallback _tabLabelFactoryCb) {TabLabelFactoryCb=_tabLabelFactoryCb;}
     -> If present, fired when calling: TabLabel* addTabLabel(const char* label,const char* tooltip=NULL,bool closable=true,bool draggable=true,void* userPtr=NULL,const char* userText=NULL,int userInt=0,int ImGuiWindowFlagsForContent=0);
        User can use it to append a TabLabel::userPtr or to provide a proper extension from TabWindowLabel.
        In addition, that method is called on layout deserialization, so that the user can load the content after deserialization (label,tooltip,userInt and userText are the TabLabel fields that are serialized/deserialized).
static void TabWindow::SetTabLabelSaveCallback(TabLabelFileCallback _tabLabelSaveCb) {TabLabelSaveCb=_tabLabelSaveCb;}
     -> When a modified file needs to be saved, if set, that method will be called (with NULL argument).
        Otherwise TabLabel::saveAs(NULL) will be called.
    If you don't need to save anything, simply never use TabLabel::setModified(true).
*/



// KNOWN BUGS:
/*
-> If you scale the tab labels (e.g. with CTRL+mouse wheel), the dragged tab is not scaled. (I'm not sure this will be ever fixed).
*/


// BETTER ALTERNATIVES:
/*
There are better alternatives to Imgui::TabWindow:
-> https://github.com/thennequin/ImWindow   [for Window OS]
-> https://github.com/nem0/LumixEngine/blob/master/src/studio_lib/imgui/imgui_dock.inl [lumixengine's Dock] => NOW AVAILABLE as imguidock addon.
Please see: https://github.com/ocornut/imgui/issues for further info
*/


#include <string.h>
#ifdef _MSC_VER
#   ifndef strcasecmp
#       define strcasecmp _stricmp
#   endif // strcasecmp
#   ifndef strncasecmp
#       define strncasecmp _strnicmp
#   endif // strncasecmp
#endif // _MSC_VER


namespace ImGui {

enum ImGuiTabLabelStyleEnum {
    ImGuiTabLabelStyle_Default=0,
    ImGuiTabLabelStyle_Dark,
    ImGuiTabLabelStyle_Red,
    ImGuiTabLabelStyle_Green,
    ImGuiTabLabelStyle_Blue,
    ImGuiTabLabelStyle_Yellow,
    ImGuiTabLabelStyle_Orange,
    ImGuiTabLabelStyle_White,
    ImGuiTabLabelStyle_Tidy,
    ImGuiTabLabelStyle_Foxy,
    ImGuiTabLabelStyle_FoxyInverse,    

    ImGuiTabLabelStyle_Count
};
struct TabLabelStyle {
enum Colors {
    Col_TabLabel = 0,
    Col_TabLabelHovered,
    Col_TabLabelActive,
    Col_TabLabelBorder,
    Col_TabLabelText,

    Col_TabLabelSelected,
    Col_TabLabelSelectedHovered,
    Col_TabLabelSelectedActive,
    Col_TabLabelSelectedBorder,
    Col_TabLabelSelectedText,

    Col_TabLabelCloseButtonHovered,
    Col_TabLabelCloseButtonActive,
    Col_TabLabelCloseButtonBorder,
    Col_TabLabelCloseButtonTextHovered,

    Col_TabLabel_Count
};
ImU32 colors[Col_TabLabel_Count];

float fillColorGradientDeltaIn0_05; // vertical gradient if > 0 (looks nice but it's very slow)
float rounding;
float borderWidth;

float closeButtonRounding;
float closeButtonBorderWidth;
float closeButtonTextWidth;

enum FontStyle {
    FONT_STYLE_NORMAL=0,
    FONT_STYLE_BOLD,
    FONT_STYLE_ITALIC,
    FONT_STYLE_BOLD_ITALIC,
    FONT_STYLE_COUNT
};
enum TabState {
    TAB_STATE_NORMAL,
    TAB_STATE_SELECTED,
    TAB_STATE_MODIFIED,
    TAB_STATE_SELECTED_MODIFIED,
    TAB_STATE_COUNT
};
int fontStyles[TAB_STATE_COUNT];    // Users should add TabLabelStyle::ImGuiFonts to map them for these to work

ImVec4 tabWindowLabelBackgroundColor;
bool tabWindowLabelShowAreaSeparator;
ImVec4 tabWindowSplitterColor;
float tabWindowSplitterSize;

IMGUI_API TabLabelStyle();

void reset() {Reset(*this);}
IMGUI_API static bool Edit(TabLabelStyle& style=TabLabelStyle::Get());
IMGUI_API static bool EditFast(TabLabelStyle &s=TabLabelStyle::Get());
static void Reset(TabLabelStyle& style=TabLabelStyle::Get()) {style = TabLabelStyle();}

// These modify the style: some operation are not loseless!
IMGUI_API static void InvertSelectedLook(TabLabelStyle& style=TabLabelStyle::Get());
IMGUI_API static void ShiftHue(TabLabelStyle& style,float amountIn0_1);
IMGUI_API static void InvertColors(TabLabelStyle& style=TabLabelStyle::Get(),float saturationThreshould=0.1f); // in [0.f,0.5f] AFAIR
IMGUI_API static void LightenBackground(TabLabelStyle& style=TabLabelStyle::Get(),float amount=0.15f);
IMGUI_API static void DarkenBackground(TabLabelStyle& style=TabLabelStyle::Get(),float amount=0.15f);


#if (defined(IMGUIHELPER_H_) && !defined(NO_IMGUIHELPER_SERIALIZATION))
#ifndef NO_IMGUIHELPER_SERIALIZATION_SAVE
IMGUI_API static bool Save(const TabLabelStyle& style,ImGuiHelper::Serializer& s);
static inline bool Save(const TabLabelStyle &style, const char *filename)    {
    ImGuiHelper::Serializer s(filename);
    return Save(style,s);
}
#endif //NO_IMGUIHELPER_SERIALIZATION_SAVE
#ifndef NO_IMGUIHELPER_SERIALIZATION_LOAD
IMGUI_API static bool Load(TabLabelStyle& style, ImGuiHelper::Deserializer& d, const char ** pOptionalBufferStart=NULL);
static inline bool Load(TabLabelStyle& style,const char* filename) {
    ImGuiHelper::Deserializer d(filename);
    return Load(style,d);
}
#endif //NO_IMGUIHELPER_SERIALIZATION_LOAD
#endif //NO_IMGUIHELPER_SERIALIZATION


// Gets the default style instance (same as TabLabelStyle::style)
inline static TabLabelStyle& Get() {return style;}
// Gets a new instance = default style instance blended with ImGui::GetStyle().Alpha
IMGUI_API static const TabLabelStyle& GetMergedWithWindowAlpha();
static TabLabelStyle style;
static const char* ColorNames[Col_TabLabel_Count];
static const char* FontStyleNames[FONT_STYLE_COUNT];
static const char* TabStateNames[TAB_STATE_COUNT];
static const ImFont* ImGuiFonts[FONT_STYLE_COUNT];
};
IMGUI_API bool ResetTabLabelStyle(int tabLabelStyleEnum, TabLabelStyle& style);
IMGUI_API const char** GetDefaultTabLabelStyleNames();   // ImGuiTabLabelStyle_Count names re returned
// satThresholdForInvertingLuminance: in [0,1] if == 0.f luminance is not inverted at all
// shiftHue: in [0,1] if == 0.f hue is not changed at all
IMGUI_API void ChangeTabLabelStyleColors(TabLabelStyle& style,float satThresholdForInvertingLuminance=.1f,float shiftHue=0.f);


class TabWindow {
public:
struct TabLabel {
    friend struct TabWindowNode;
    friend class TabWindow;
    friend struct TabWindowDragData;
private:
    // [Nothing is used as ImGui ID: the ImGui ID is the address of the TabLabel]
    char* label;      // [owned] text displayed by the TabLabel (one more char is allocated to optimize appending an asterisk to it)
    char* tooltip;    // [owned] tooltip displayed by the TabLabel
    char* userText;   // [owned] user text
    bool modified;
protected:
    bool closable;
    bool draggable;
    TabLabel(const char* _label=NULL,const char* _tooltip=NULL,bool _closable=true,bool _draggable=true)    {
        label = tooltip = NULL;
        userPtr = NULL;userText=NULL;userInt=0;
        setLabel(_label);
        setTooltip(_tooltip);
        closable = _closable;
        draggable = _draggable;
        mustCloseNextFrame = false;
        mustSelectNextFrame = false;
        wndFlags = 0;
        modified = false;
    }
    virtual ~TabLabel() {
        if (label) {ImGui::MemFree(label);label=NULL;}
        if (tooltip) {ImGui::MemFree(tooltip);tooltip=NULL;}
        if (userText) {ImGui::MemFree(userText);userText=NULL;}
    }
    IMGUI_API static void DestroyTabLabel(TabLabel*& tab);
public:
    inline const char* getLabel() const {return label;}
    inline bool matchLabel(const char* match) const {return modified ? (strncmp(match,label,strlen(label)-1)==0) : (strcmp(match,label)==0);}
    inline bool matchLabelExtension(const char* matchExtension) const {
        const char* dot = strrchr(label,(int) '.');if (!dot) return false;
        return modified ? (strncasecmp(matchExtension,dot,strlen(dot)-1)==0) : (strcasecmp(matchExtension,dot)==0);
    }
    void setLabel(const char* lbl,bool appendAnAsteriskAndMarkAsModified=false)  {
        if (label) {ImGui::MemFree(label);label=NULL;}
        const char e = '\0';if (!lbl) lbl=&e;
        const int sz = strlen(lbl)+1;       // we allocate one char more (optimization for appending an asterisk)
        label = (char*) ImGui::MemAlloc(sz+1);strcpy(label,lbl);
        if (appendAnAsteriskAndMarkAsModified)  {
            modified = true;strcat(label,"*");
        }
        else modified = false;
    }
    inline bool copyLabelTo(char* buffer, int buffer_size) const {
        int len = modified ? ((int)strlen(label)-1) : (int)strlen(label);
        if (len<0) len=0;bool ok = true;
        if (buffer_size<=len) {len = buffer_size-1;ok=false;}
        strncpy(buffer,label,len);
        buffer[len]='\0';
        return ok;
    }
    inline bool matchTooltip(const char* match) const {return (strcmp(match,tooltip)==0);}
    inline bool matchUserText(const char* match) const {return (strcmp(match,userText)==0);}

    inline bool getModified() const {return modified;}
    inline void setModified(bool flag) {
        if (modified == flag) return;
        modified = flag;int sz = strlen(label);
        if (modified)   {if (sz==0 || label[sz-1]!='*') strcat(label,"*");}
        else            {if (sz>0 && label[sz-1]=='*') label[sz-1]='\0';}
    }
    inline const char* getTooltip() const {return tooltip;}
    void setTooltip(const char* tt)  {
        if (tooltip) {ImGui::MemFree(tooltip);tooltip=NULL;}
        const char e = '\0';if (!tt) tt=&e;
        const int sz = strlen(tt);
        tooltip = (char*) ImGui::MemAlloc(sz+1);strcpy(tooltip,tt);
    }
    void setUserText(const char* _userText)  {
        if (userText) {ImGui::MemFree(userText);userText=NULL;}
        if (_userText)  {
            const int sz = strlen(_userText);
            userText = (char*) ImGui::MemAlloc(sz+1);strcpy(userText,_userText);
        }
    }
    inline const char* getUserText() const {return userText;}
    inline bool isClosable() const {return closable;}
    inline bool isDraggable() const {return draggable;}
    mutable void* userPtr;
    mutable int userInt;
    mutable bool mustCloseNextFrame;
    mutable bool mustSelectNextFrame;
    mutable int wndFlags;               // used for the imgui child window that hosts the tab content

    // This method will be used ONLY if you don't set TabWindow::SetWindowContentDrawerCallback(...),
    // (and you prefer extending from this class)
    virtual void render() {
        ImGui::Text("Here is the content of tab label: \"%s\".",getLabel());
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Text("Please consider using:");
        ImGui::Text("ImGui::TabWindow::SetWindowContentDrawerCallback(...)");
        ImGui::TextWrapped("%s","to set a content for it, or extending TabWindowLabel and implement its render() method.");
    }

    virtual bool saveAs(const char* savePath=NULL) {
        setModified(false);
        return true;
    }
};
protected:
public:
typedef void (*TabLabelCallback)(TabLabel* tabLabel,TabWindow& parent,void* userPtr);
//typedef void (*TabLabelClosingCallback)(const ImVector<TabLabel*>& tabLabels,const ImVector<TabWindow*>& parents,ImVector<bool>& forbidClosingOut,void* userPtr);
typedef void (*TabLabelDeletingCallback)(TabLabel* tabLabel);
typedef void (*TabLabelGroupPopupMenuCallback)(ImVector<TabLabel*>& tabLabels,TabWindow& parent,struct TabWindowNode* node,void* userPtr);
typedef TabLabel* (*TabLabelFactoryCallback)(TabWindow& parent,const char* label,const char* tooltip,bool closable,bool draggable,void* userPtr,const char* userText,int userInt,int ImGuiWindowFlagsForContent);
typedef bool (*TabLabelFileCallback)(TabLabel* tabLabel,TabWindow& parent,const char* filePath);


protected:
struct TabWindowNode* mainNode;   // owned
struct TabWindowNode* activeNode; // reference
ImVector<TabWindow*> tabWindowsToExclude;// can't exchange tab labels with these
bool isolatedMode;  // can't exchange tab labels outside
bool init;
IMGUI_API void clearNodes();

public:
static TabLabelCallback WindowContentDrawerCb;
static void* WindowContentDrawerUserPtr;
static TabLabelCallback TabLabelPopupMenuDrawerCb;
static void* TabLabelPopupMenuDrawerUserPtr;
//static TabLabelClosingCallback TabLabelClosingCb;
//static void* TabLabelClosingUserPtr;
static TabLabelDeletingCallback TabLabelDeletingCb;
static TabLabelGroupPopupMenuCallback TabLabelGroupPopupMenuDrawerCb;
static void* TabLabelGroupPopupMenuDrawerUserPtr;
static TabLabelFactoryCallback TabLabelFactoryCb;
static TabLabelFileCallback TabLabelSaveCb;


public:
IMGUI_API TabWindow();
IMGUI_API ~TabWindow();

// Handy for initialization before calling render() the firsat time
bool isInited() const {return init;}

// Here "label" is NOT used as ImGui ID (you shouldn't worry about it): it's just the text you want to display
// If "TabLabelFactoryCb" is present, it will be used in the following method:
IMGUI_API TabLabel* addTabLabel(const char* label,const char* tooltip=NULL,bool closable=true,bool draggable=true,void* userPtr=NULL,const char* userText=NULL,int userInt=0,int ImGuiWindowFlagsForContent=0);
IMGUI_API TabLabel* addTabLabel(TabLabel* tabLabel,bool checkIfAlreadyPresent=true);  // use it only if you extend TabLabel
IMGUI_API bool removeTabLabel(TabLabel* tab);
IMGUI_API void clear();

// Find methods (untested)
IMGUI_API TabLabel* findTabLabelFromLabel(const char* label) const;   // trimming the last trailing asterisk
IMGUI_API TabLabel* findTabLabelFromTooltip(const char* tooltip) const;
IMGUI_API TabLabel* findTabLabelFromUserPtr(void* userPtr) const;
IMGUI_API TabLabel* findTabLabelFromUserText(const char* userText) const;
IMGUI_API static TabLabel* FindTabLabelFromLabel(const char* label,const TabWindow* pTabWindows,int numTabWindows,int* pOptionalTabWindowIndexOut);   // trimming the last trailing asterisk
IMGUI_API static TabLabel* FindTabLabelFromTooltip(const char* tooltip,const TabWindow* pTabWindows,int numTabWindows,int* pOptionalTabWindowIndexOut);
IMGUI_API static TabLabel* FindTabLabelFromUserPtr(void* userPtr,const TabWindow* pTabWindows,int numTabWindows,int* pOptionalTabWindowIndexOut);
IMGUI_API static TabLabel* FindTabLabelFromUserText(const char* userText,const TabWindow* pTabWindows,int numTabWindows,int* pOptionalTabWindowIndexOut);

// Callbacks
static void SetWindowContentDrawerCallback(TabLabelCallback _windowContentDrawer,void* userPtr=NULL) {
    WindowContentDrawerCb=_windowContentDrawer;
    WindowContentDrawerUserPtr=userPtr;
}
static void SetTabLabelPopupMenuDrawerCallback(TabLabelCallback _tabLabelPopupMenuDrawer,void* userPtr=NULL) {
    TabLabelPopupMenuDrawerCb=_tabLabelPopupMenuDrawer;
    TabLabelPopupMenuDrawerUserPtr=userPtr;
}
inline static const char* GetTabLabelPopupMenuName() {return "TabWindowTabLabelPopupMenu";}
/*static void SetTabLabelClosingCallback(TabLabelClosingCallback _tabLabelClosing,void* userPtr=NULL) {
    TabLabelClosingCb=_tabLabelClosing;
    TabLabelClosingUserPtr=userPtr;
}*/
static void SetTabLabelDeletingCallback(TabLabelDeletingCallback _tabLabelDeleting) {TabLabelDeletingCb=_tabLabelDeleting;}
static void SetTabLabelGroupPopupMenuDrawerCallback(TabLabelGroupPopupMenuCallback _tabLabelGroupPopupMenuDrawer,void* userPtr=NULL) {
    TabLabelGroupPopupMenuDrawerCb=_tabLabelGroupPopupMenuDrawer;
    TabLabelGroupPopupMenuDrawerUserPtr=userPtr;
}
inline static const char* GetTabLabelGroupPopupMenuName() {return "TabWindowTabLabelGroupPopupMenu";}
static void SetTabLabelFactoryCallback(TabLabelFactoryCallback _tabLabelFactoryCb) {TabLabelFactoryCb=_tabLabelFactoryCb;}
static void SetTabLabelSaveCallback(TabLabelFileCallback _tabLabelSaveCb) {TabLabelSaveCb=_tabLabelSaveCb;}
inline static const char* GetTabLabelAskForDeletionModalWindowName() {return "Save modified file/s ?##TabWindowTabLabelAskForDeletionModalWindowName";}


// Main method
IMGUI_API void render();

// Texture And Memory Data  (4 channels, no additional stride between lines, => rgba_buffer_out.size() = 4*(*w_out)*(*h_out) )
static void GetDockPanelIconImageRGBA(ImVector<unsigned char>& rgba_buffer_out,int* w_out=NULL,int* h_out=NULL); // Manually redrawn based on the ones in https://github.com/dockpanelsuite/dockpanelsuite (that is MIT licensed). So no copyright issues for this AFAIK, but I'm not a lawyer and I cannot guarantee it.
static ImTextureID DockPanelIconTextureID;  // User must load it (using GetDockPanelIconImageRGBA) and free it when no IMGUI_USE_XXX_BINDING is used.

// These are just optional "filtering" methods
bool isIsolated() const {return isolatedMode;}          // can't exchange tab labels outside
void setIsolatedMode(bool flag) {isolatedMode=flag;}
IMGUI_API void excludeTabWindow(TabWindow& tw);                   // can't exchange tab labels with...
IMGUI_API void includeTabWindow(TabWindow& tw);                   // removes from the "exclude list"...
const ImVector<TabWindow*>& getTabWindowsToExclude() const {return tabWindowsToExclude;}
IMGUI_API bool canExchangeTabLabelsWith(TabWindow* tw);

IMGUI_API bool isMergeble(struct TabWindowNode* node);
IMGUI_API int getNumTabs(struct TabWindowNode* node);
IMGUI_API int getNumClosableTabs(struct TabWindowNode* node);
IMGUI_API bool merge(struct TabWindowNode* node);        // Warning: it invalidates "node" after the call

IMGUI_API static void GetAllTabLabels(TabWindow* pTabWindowsIn, int numTabWindowsIn, ImVector<TabLabel*>& tabsOut, ImVector<TabWindow*>& parentsOut, bool onlyClosableTabs=false, bool onlyModifiedTabs=false);
inline static void SaveAll(ImVector<TabLabel*>& tabs, ImVector<TabWindow*>& parents) {CloseTabLabelsHelper(tabs,parents,true,false,false,true);}
inline static bool StartCloseAllDialog(ImVector<TabLabel*>& tabs, ImVector<TabWindow*>& parents,bool allowCancelDialog=true) {return CloseTabLabelsHelper(tabs,parents,true,true,allowCancelDialog,false);}
IMGUI_API static bool AreSomeDialogsOpen();

IMGUI_API void getAllTabLabels(ImVector<TabLabel*>& tabsOut,bool onlyClosableTabs=false, bool onlyModifiedTabs=false);
IMGUI_API void saveAll(ImVector<TabLabel*>* ptabs=NULL);
IMGUI_API bool startCloseAllDialog(ImVector<TabLabel*>* ptabs=NULL,bool allowCancelDialog=true);

mutable void* userPtr;

static ImGuiWindowFlags ExtraWindowFlags;

//-------------------------------------------------------------------------------
#       if (defined(IMGUIHELPER_H_) && !defined(NO_IMGUIHELPER_SERIALIZATION))
#       ifndef NO_IMGUIHELPER_SERIALIZATION_SAVE
public:
        IMGUI_API bool save(ImGuiHelper::Serializer& s);
        IMGUI_API bool save(const char* filename);
        IMGUI_API static bool Save(const char* filename,TabWindow* pTabWindows,int numTabWindows);
#       endif //NO_IMGUIHELPER_SERIALIZATION_SAVE
#       ifndef NO_IMGUIHELPER_SERIALIZATION_LOAD
public:
        IMGUI_API bool load(ImGuiHelper::Deserializer& d,const char ** pOptionalBufferStart=NULL);
        IMGUI_API bool load(const char* filename);
        IMGUI_API static bool Load(const char* filename,TabWindow* pTabWindows,int numTabWindows);
#       endif //NO_IMGUIHELPER_SERIALIZATION_LOAD
#       endif //NO_IMGUIHELPER_SERIALIZATION
//--------------------------------------------------------------------------------

protected:

IMGUI_API TabLabel* createTabLabel(const char* label,const char* tooltip=NULL,bool closable=true,bool draggable=true,void* userPtr=NULL,const char* userText=NULL,int userInt=0,int ImGuiWindowFlagsForContent=0);
IMGUI_API static bool CloseTabLabelsHelper(ImVector<TabLabel*>& tabs, ImVector<TabWindow*>& parents, bool saveAll, bool askForSaving, bool allowCancelDialog, bool dontCloseTabs);


friend struct TabLabel;
friend struct TabWindowNode;
friend struct TabWindowDragData;

private:
IMGUI_API static bool ModalDialogSaveDisplay(const char* dialogName,ImVector<TabWindow::TabLabel*>& TabsToAskFor,ImVector<TabWindow*>& TabsToAskForParents,
IMGUI_API bool closeTabsAfterSaving,bool allowCancel,bool * pMustCloseDialogOut=NULL,const char* btnDoNotSaveName="Do not save",const char* btnSaveName="Save",const char* btnCancelName="Cancel",
IMGUI_API const char* dialogTitleLine1="The following tab labels have unsaved changes.",const char* dialogTitleLine2="Do you want to save them ?");

};
typedef TabWindow::TabLabel TabWindowLabel;


// This has nothing to do with TabWindow; it's just a port of this gist: https://gist.github.com/Flix01/3bc3d7b3d996582e034e sharing "TabLabelStyle".
// Please see: https://github.com/ocornut/imgui/issues/261 for further info
// Based on the code by krys-spectralpixel (https://github.com/krys-spectralpixel), posted here: https://github.com/ocornut/imgui/issues/261
/* pOptionalHoveredIndex: a ptr to an optional int that is set to -1 if no tab label is hovered by the mouse.
 * pOptionalItemOrdering: an optional static array of unique integers from 0 to numTabs-1 that maps the tab label order. If one of the numbers is replaced by -1 the tab label is not visible (closed). It can be read/modified at runtime.
 * allowTabReorder (requires pOptionalItemOrdering): allows tab reordering through drag and drop (it modifies pOptionalItemOrdering).
 *                  However it seems to work only when dragging tabs from the left (top) to the right (bottom) and not vice-versa (this is bad, but can't lock the tab order in any way).
 * allowTabClosing (requires pOptionalItemOrdering): adds a close button to the tab. When the close button is clicked, the tab value in pOptionalItemOrdering is set to -1.
 * pOptionalClosedTabIndex (requires allowTabClosing): out variable (int pointer) that returns the index of the closed tab in last call or -1.
 * pOptionalClosedTabIndexInsideItemOrdering (requires allowTabClosing): same as above, but index inside the pOptionalItemOrdering array. Users can use this value to prevent single tabs from closing when their close button is clicked (since we can't mix closable and non-closable tabs here).
*/
IMGUI_API bool TabLabels(int numTabs, const char** tabLabels, int& selectedIndex, const char** tabLabelTooltips=NULL , bool wrapMode=true, int* pOptionalHoveredIndex=NULL, int* pOptionalItemOrdering=NULL, bool allowTabReorder=true, bool allowTabClosing=false, int* pOptionalClosedTabIndex=NULL,int * pOptionalClosedTabIndexInsideItemOrdering=NULL);

// ImGui::TabLabelsVertical() are similiar to ImGui::TabLabels(), but they do not support WrapMode.
IMGUI_API bool TabLabelsVertical(bool textIsRotatedCCW,int numTabs, const char** tabLabels, int& selectedIndex, const char** tabLabelTooltips=NULL, int* pOptionalHoveredIndex=NULL, int* pOptionalItemOrdering=NULL, bool allowTabReorder=false, bool allowTabClosing=false, int* pOptionalClosedTabIndex=NULL,int * pOptionalClosedTabIndexInsideItemOrdering=NULL,bool invertRounding=false);
IMGUI_API float CalcVerticalTabLabelsWidth();


// Untested attempt to provide serialization for ImGui::TabLabels(...) or ImGui::TabLabelsVertical(...): only "selectedIndex" and "pOptionalItemOrdering" are serialized.
//-------------------------------------------------------------------------------
#   if (defined(IMGUIHELPER_H_) && !defined(NO_IMGUIHELPER_SERIALIZATION))
#       ifndef NO_IMGUIHELPER_SERIALIZATION_SAVE
        IMGUI_API bool TabLabelsSave(ImGuiHelper::Serializer& s,int selectedIndex,const int* pOptionalItemOrdering=NULL,int numTabs=0);
        IMGUI_API bool TabLabelsSave(const char* filename,int selectedIndex,const int* pOptionalItemOrdering=NULL,int numTabs=0);
#       endif //NO_IMGUIHELPER_SERIALIZATION_SAVE
#       ifndef NO_IMGUIHELPER_SERIALIZATION_LOAD
        IMGUI_API bool TabLabelsLoad(ImGuiHelper::Deserializer& d,int* pSelectedIndex,int* pOptionalItemOrdering=NULL,int numTabs=0,const char ** pOptionalBufferStart=NULL);
        IMGUI_API bool TabLabelsLoad(const char* filename,int* pSelectedIndex,int* pOptionalItemOrdering=NULL,int numTabs=0);
#       endif //NO_IMGUIHELPER_SERIALIZATION_LOAD
#   endif //NO_IMGUIHELPER_SERIALIZATION
//--------------------------------------------------------------------------------



} // namespace ImGui


#endif //IMGUITABWINDOW_H_
