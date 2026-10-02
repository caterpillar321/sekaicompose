#include "ServerDecorationKDE.hpp"
#include "core/Compositor.hpp"
#include "../Compositor.hpp"
#include "../desktop/Window.hpp"

CServerDecorationKDE::CServerDecorationKDE(SP<COrgKdeKwinServerDecoration> resource_, SP<CWLSurfaceResource> surf) : m_resource(resource_) {
    if UNLIKELY (!good())
        return;

    m_resource->setRelease([this](COrgKdeKwinServerDecoration* pMgr) { PROTO::serverDecorationKDE->destroyResource(this); });
    m_resource->setOnDestroy([this](COrgKdeKwinServerDecoration* pMgr) { PROTO::serverDecorationKDE->destroyResource(this); });

    m_surface = surf;
    // SEKAI_CLIENT_DECO: 원본은 request_mode 를 무시하고 늘 server 로 답했다 — 앱이 고른 모드를 기억하고 그대로 답한다
    m_resource->setRequestMode([this](COrgKdeKwinServerDecoration*, uint32_t mode) {
        const bool CLIENT = mode == ORG_KDE_KWIN_SERVER_DECORATION_MODE_CLIENT;
        m_resource->sendMode(CLIENT ? ORG_KDE_KWIN_SERVER_DECORATION_MODE_CLIENT : ORG_KDE_KWIN_SERVER_DECORATION_MODE_SERVER);
        if (m_sekaiClient == CLIENT)
            return;
        m_sekaiClient = CLIENT;
        if (const auto S = m_surface.lock()) {
            if (const auto W = g_pCompositor->getWindowFromSurface(S); W && W->m_isMapped)
                W->updateDynamicRules();
        }
    });

    m_resource->sendMode(ORG_KDE_KWIN_SERVER_DECORATION_MANAGER_MODE_SERVER);
}

bool CServerDecorationKDEProtocol::sekaiWantsClient(SP<CWLSurfaceResource> surf) {
    for (auto const& d : m_decos) {
        if (d->m_surface.lock() == surf)
            return d->m_sekaiClient;
    }
    return false;
}

bool CServerDecorationKDE::good() {
    return m_resource->resource();
}

CServerDecorationKDEProtocol::CServerDecorationKDEProtocol(const wl_interface* iface, const int& ver, const std::string& name) : IWaylandProtocol(iface, ver, name) {
    ;
}

void CServerDecorationKDEProtocol::bindManager(wl_client* client, void* data, uint32_t ver, uint32_t id) {
    const auto RESOURCE = m_managers.emplace_back(makeUnique<COrgKdeKwinServerDecorationManager>(client, ver, id)).get();
    RESOURCE->setOnDestroy([this](COrgKdeKwinServerDecorationManager* p) { this->onManagerResourceDestroy(p->resource()); });

    RESOURCE->setCreate([this](COrgKdeKwinServerDecorationManager* pMgr, uint32_t id, wl_resource* pointer) { this->createDecoration(pMgr, id, pointer); });

    // send default mode of SSD, as Hyprland will never ask for CSD. Screw Gnome and GTK.
    RESOURCE->sendDefaultMode(ORG_KDE_KWIN_SERVER_DECORATION_MANAGER_MODE_SERVER);
}

void CServerDecorationKDEProtocol::onManagerResourceDestroy(wl_resource* res) {
    std::erase_if(m_managers, [&](const auto& other) { return other->resource() == res; });
}

void CServerDecorationKDEProtocol::destroyResource(CServerDecorationKDE* hayperlaaaand) {
    std::erase_if(m_decos, [&](const auto& other) { return other.get() == hayperlaaaand; });
}

void CServerDecorationKDEProtocol::createDecoration(COrgKdeKwinServerDecorationManager* pMgr, uint32_t id, wl_resource* surf) {
    const auto CLIENT = pMgr->client();
    const auto RESOURCE =
        m_decos.emplace_back(makeUnique<CServerDecorationKDE>(makeShared<COrgKdeKwinServerDecoration>(CLIENT, pMgr->version(), id), CWLSurfaceResource::fromResource(surf))).get();

    if UNLIKELY (!RESOURCE->good()) {
        pMgr->noMemory();
        m_decos.pop_back();
        return;
    }
}
