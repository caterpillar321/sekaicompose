#include "XDGDecoration.hpp"
#include "XDGShell.hpp"
#include "../desktop/Window.hpp"
#include <algorithm>

// SEKAI_CLIENT_DECO: 앱이 고른 모드를 기억하고 그대로 돌려준다 (원본은 늘 server 로 답했다).
//   client 를 고른 앱(Electron 의 Discord·VS Code, Firefox 탭 제목줄, GTK 헤더바 …)은 제목줄을 스스로 그리므로
//   hyprbars 가 그 창에는 막대를 그리지 않는다 (CWindow::sekaiClientDecoration) — 제목줄이 두 개로 보이던 것
static void sekaiNoteXDGMode(wl_resource* toplevel, bool client) {
    const auto TL = CXDGToplevelResource::fromResource(toplevel);
    if (!TL || TL->m_sekaiClientDeco == client)
        return;
    TL->m_sekaiClientDeco = client;
    if (const auto W = TL->m_window.lock(); W && W->m_isMapped)
        W->updateDynamicRules(); // hyprbars 가 windowUpdateRules 로 막대를 다시 정한다
}

CXDGDecoration::CXDGDecoration(SP<CZxdgToplevelDecorationV1> resource_, wl_resource* toplevel) : m_resource(resource_), m_toplevelResource(toplevel) {
    if UNLIKELY (!m_resource->resource())
        return;

    m_resource->setDestroy([this](CZxdgToplevelDecorationV1* pMgr) { PROTO::xdgDecoration->destroyDecoration(this); });
    m_resource->setOnDestroy([this](CZxdgToplevelDecorationV1* pMgr) { PROTO::xdgDecoration->destroyDecoration(this); });

    m_resource->setSetMode([this](CZxdgToplevelDecorationV1*, zxdgToplevelDecorationV1Mode mode) {
        std::string modeString;
        switch (mode) {
            case ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE: modeString = "MODE_CLIENT_SIDE"; break;
            case ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE: modeString = "MODE_SERVER_SIDE"; break;
            default: modeString = "INVALID"; break;
        }

        const bool CLIENT = mode == ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE;
        LOGM(LOG, "setMode: {}. Sending {} as reply. (SEKAI_CLIENT_DECO)", modeString, CLIENT ? "MODE_CLIENT_SIDE" : "MODE_SERVER_SIDE");
        sekaiNoteXDGMode(m_toplevelResource, CLIENT);
        m_resource->sendConfigure(CLIENT ? ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE : ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
    });

    m_resource->setUnsetMode([this](CZxdgToplevelDecorationV1*) {
        LOGM(LOG, "unsetMode. Sending MODE_SERVER_SIDE.");
        sekaiNoteXDGMode(m_toplevelResource, false);
        m_resource->sendConfigure(ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
    });

    m_resource->sendConfigure(ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
}

bool CXDGDecoration::good() {
    return m_resource->resource();
}

wl_resource* CXDGDecoration::toplevelResource() {
    return m_toplevelResource;
}

CXDGDecorationProtocol::CXDGDecorationProtocol(const wl_interface* iface, const int& ver, const std::string& name) : IWaylandProtocol(iface, ver, name) {
    ;
}

void CXDGDecorationProtocol::bindManager(wl_client* client, void* data, uint32_t ver, uint32_t id) {
    const auto RESOURCE = m_managers.emplace_back(makeUnique<CZxdgDecorationManagerV1>(client, ver, id)).get();
    RESOURCE->setOnDestroy([this](CZxdgDecorationManagerV1* p) { this->onManagerResourceDestroy(p->resource()); });

    RESOURCE->setDestroy([this](CZxdgDecorationManagerV1* pMgr) { this->onManagerResourceDestroy(pMgr->resource()); });
    RESOURCE->setGetToplevelDecoration([this](CZxdgDecorationManagerV1* pMgr, uint32_t id, wl_resource* xdgToplevel) { this->onGetDecoration(pMgr, id, xdgToplevel); });
}

void CXDGDecorationProtocol::onManagerResourceDestroy(wl_resource* res) {
    std::erase_if(m_managers, [&](const auto& other) { return other->resource() == res; });
}

void CXDGDecorationProtocol::destroyDecoration(CXDGDecoration* decoration) {
    m_decorations.erase(decoration->toplevelResource());
}

void CXDGDecorationProtocol::onGetDecoration(CZxdgDecorationManagerV1* pMgr, uint32_t id, wl_resource* xdgToplevel) {
    if UNLIKELY (m_decorations.contains(xdgToplevel)) {
        pMgr->error(ZXDG_TOPLEVEL_DECORATION_V1_ERROR_ALREADY_CONSTRUCTED, "Decoration object already exists");
        return;
    }

    const auto CLIENT = pMgr->client();
    const auto RESOURCE =
        m_decorations.emplace(xdgToplevel, makeUnique<CXDGDecoration>(makeShared<CZxdgToplevelDecorationV1>(CLIENT, pMgr->version(), id), xdgToplevel)).first->second.get();

    if UNLIKELY (!RESOURCE->good()) {
        pMgr->noMemory();
        m_decorations.erase(xdgToplevel);
        return;
    }
}