#define WLR_USE_UNSTABLE

#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/config/ConfigManager.hpp>
#include <hyprland/src/desktop/Window.hpp>
#include <hyprland/src/plugins/PluginAPI.hpp>
#define private public
#include <hyprland/src/managers/LayoutManager.hpp>
#include <hyprland/src/layout/DwindleLayout.hpp>
#include <hyprland/src/layout/MasterLayout.hpp>
#undef private

inline HANDLE         PHANDLE = nullptr;

inline CFunctionHook* g_pCalculateWorkspaceHookDwindle = nullptr;
typedef void (*origCalculateWorkspaceDwindle)(CHyprDwindleLayout*, const PHLWORKSPACE&);
inline CFunctionHook* g_pCalculateWorkspaceHookMaster = nullptr;
typedef void (*origCalculateWorkspaceMaster)(CHyprMasterLayout*, const PHLWORKSPACE&);

APICALL EXPORT std::string PLUGIN_API_VERSION() {
    return HYPRLAND_API_VERSION;
}

void hkCalculateWorkspaceDwindle(CHyprDwindleLayout* thisptr, const PHLWORKSPACE& pWorkspace) {
    const auto PMONITOR = pWorkspace->m_monitor.lock();
    const auto PLAYOUT  = reinterpret_cast<CHyprDwindleLayout*>(g_pLayoutManager->getCurrentLayout());

    if (!PMONITOR)
        return;

    if (pWorkspace->m_hasFullscreenWindow) {
        const auto PFULLWINDOW = pWorkspace->getFullscreenWindow();

        if (pWorkspace->m_fullscreenMode == FSMODE_FULLSCREEN) {
            *PFULLWINDOW->m_realPosition = PMONITOR->m_position;
            *PFULLWINDOW->m_realSize     = PMONITOR->m_size;
        } else if (pWorkspace->m_fullscreenMode == FSMODE_MAXIMIZED) {
            for (auto& n : PLAYOUT->m_dwindleNodesData) {
                if (n.workspaceID != pWorkspace->m_id)
                    continue;

                SDwindleNodeData fakeNode;
                fakeNode.pWindow        = n.pWindow;
                fakeNode.box            = {PMONITOR->m_position + PMONITOR->m_reservedTopLeft, PMONITOR->m_size - PMONITOR->m_reservedTopLeft - PMONITOR->m_reservedBottomRight};
                fakeNode.workspaceID    = pWorkspace->m_id;
                PFULLWINDOW->m_position = fakeNode.box.pos();
                PFULLWINDOW->m_size     = fakeNode.box.size();
                fakeNode.ignoreFullscreenChecks = true;

                PLAYOUT->applyNodeDataToWindow(&fakeNode);
            }
        }
    } else
        (*(origCalculateWorkspaceDwindle)g_pCalculateWorkspaceHookDwindle->m_original)(thisptr, pWorkspace);
}

void hkCalculateWorkspaceMaster(CHyprMasterLayout* thisptr, const PHLWORKSPACE& pWorkspace) {
    const auto PMONITOR = pWorkspace->m_monitor.lock();
    const auto PLAYOUT  = reinterpret_cast<CHyprMasterLayout*>(g_pLayoutManager->getCurrentLayout());

    if (!PMONITOR)
        return;

    if (pWorkspace->m_hasFullscreenWindow) {
        const auto PFULLWINDOW = pWorkspace->getFullscreenWindow();

        if (pWorkspace->m_fullscreenMode == FSMODE_FULLSCREEN) {
            *PFULLWINDOW->m_realPosition = PMONITOR->m_position;
            *PFULLWINDOW->m_realSize     = PMONITOR->m_size;
        } else if (pWorkspace->m_fullscreenMode == FSMODE_MAXIMIZED) {
            for (auto& n : PLAYOUT->m_masterNodesData) {
                if (n.workspaceID != pWorkspace->m_id)
                    continue;

                SMasterNodeData fakeNode;
                fakeNode.pWindow                = n.pWindow;
                fakeNode.position               = PMONITOR->m_position + PMONITOR->m_reservedTopLeft;
                fakeNode.size                   = PMONITOR->m_size - PMONITOR->m_reservedTopLeft - PMONITOR->m_reservedBottomRight;
                fakeNode.workspaceID            = pWorkspace->m_id;
                PFULLWINDOW->m_position         = fakeNode.position;
                PFULLWINDOW->m_size             = fakeNode.size;
                fakeNode.ignoreFullscreenChecks = true;

                PLAYOUT->applyNodeDataToWindow(&fakeNode);
            }
        }
    } else
        (*(origCalculateWorkspaceMaster)g_pCalculateWorkspaceHookMaster->m_original)(thisptr, pWorkspace);
}

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
    PHANDLE = handle;

    auto FNS = HyprlandAPI::findFunctionsByName(PHANDLE, "calculateWorkspace");
    for (auto& fn : FNS) {
        if (fn.demangled.contains("CHyprDwindleLayout"))
            g_pCalculateWorkspaceHookDwindle = HyprlandAPI::createFunctionHook(PHANDLE, fn.address, (void*)::hkCalculateWorkspaceDwindle);
        if (fn.demangled.contains("CHyprMasterLayout"))
            g_pCalculateWorkspaceHookMaster = HyprlandAPI::createFunctionHook(PHANDLE, fn.address, (void*)::hkCalculateWorkspaceMaster);
    }
    bool success = g_pCalculateWorkspaceHookDwindle && g_pCalculateWorkspaceHookMaster;
    if (!success)
        throw std::runtime_error("[contactlens] Hook init failed");
    success = success && g_pCalculateWorkspaceHookDwindle->hook() && g_pCalculateWorkspaceHookMaster->hook();
    if (!success)
        throw std::runtime_error("[contactlens] Hook failed");
    return {"contactlens", "Soft monocle layout for Hyprland, fixes weird resizing behavior when cycling maximized windows in dwindle and master.", "Dregu", "1.0"};
}

APICALL EXPORT void PLUGIN_EXIT() {
    ;
}
