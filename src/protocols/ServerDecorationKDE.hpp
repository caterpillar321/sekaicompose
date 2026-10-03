#pragma once

#include <vector>
#include <cstdint>
#include "WaylandProtocol.hpp"
#include "kde-server-decoration.hpp"

class CWLSurfaceResource;

class CServerDecorationKDE {
  public:
    CServerDecorationKDE(SP<COrgKdeKwinServerDecoration> resource_, SP<CWLSurfaceResource> surf);

    bool good();

    // SEKAI_CLIENT_DECO: 앱이 요청한 모드 (client = 제목줄을 스스로 그린다)
    bool                   m_sekaiClient = false;
    WP<CWLSurfaceResource> m_surface;

  private:
    SP<COrgKdeKwinServerDecoration> m_resource;
};

class CServerDecorationKDEProtocol : public IWaylandProtocol {
  public:
    CServerDecorationKDEProtocol(const wl_interface* iface, const int& ver, const std::string& name);

    virtual void bindManager(wl_client* client, void* data, uint32_t ver, uint32_t id);

    bool         sekaiWantsClient(SP<CWLSurfaceResource> surf); // SEKAI_CLIENT_DECO
    bool         sekaiHas(SP<CWLSurfaceResource> surf);         // SEKAI_GEOM_CSD: 이 표면이 장식 객체를 만들었다

  private:
    void onManagerResourceDestroy(wl_resource* res);
    void destroyResource(CServerDecorationKDE* deco);

    void createDecoration(COrgKdeKwinServerDecorationManager* pMgr, uint32_t id, wl_resource* surf);

    //
    std::vector<UP<COrgKdeKwinServerDecorationManager>> m_managers;
    std::vector<UP<CServerDecorationKDE>>               m_decos;

    friend class CServerDecorationKDE;
};

namespace PROTO {
    inline UP<CServerDecorationKDEProtocol> serverDecorationKDE;
};
