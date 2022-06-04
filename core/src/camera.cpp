#include <camera.h>

void Camera::RefreshViewMatrix() {
    Vec3f z = (m_pos - m_lookpoint).normalized();
    Vec3f x = m_up.cross_product(z).normalized();
    Vec3f y = z.cross_product(x).normalized();

    Mat4x4f res=Mat4x4f::identity();
	res.assign(x.concat_right(y).concat_right(z).transpose());
	Mat4x4f Tr=Mat4x4f::identity();
	Tr.set_col(3, Vec4f(Vec3f::zero() - m_lookpoint, 1));
   	m_viewMat = res*Tr;
}

void Camera::LookAt(const Vec3f& pos, const Vec3f& up, const Vec3f& lookpoint) {
    m_pos = pos;
    m_up = up;
    m_lookpoint = lookpoint;
    RefreshViewMatrix();
}