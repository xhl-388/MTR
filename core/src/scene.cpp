#include <scene.h>

void Scene::RefreshProjectionMatrix() {
    m_proMat = Mat4x4f::identity();
	m_proMat[3][2] = m_projection_coeff;
}

void Scene::RefreshViewPortMatrix() {
    m_viewportMat=Mat4x4f::identity();
	m_viewportMat[0][3] = m_viewport_offset.x()+m_WIDTH/2.f;
    m_viewportMat[1][3] = m_viewport_offset.y()+m_HEIGHT/2.f;
    m_viewportMat[2][3] = m_DEPTH/2.f;

    m_viewportMat[0][0] = m_WIDTH/2.f;
    m_viewportMat[1][1] = m_HEIGHT/2.f;
    m_viewportMat[2][2] = m_DEPTH/2.f;
}