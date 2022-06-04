#pragma once

#include <ishader.h>
#include <scene.h>

struct GouraudShader : public IShader{
    Scene* scene;
	TGAImage* ao_tex;
	Mat<float,2,3> varying_uv; 
	Mat<float,4,3> varying_tri; // clip space triangle vertex coordinates
	Mat<float,3,3> ndc_tri;		// triangle vertices' NDC coordinates
	Mat<float,3,3> varying_nrm;	// original rough per vertex normal in .obj
	Mat4x4f uniform_M;   //  Projection*ModelView
    Mat4x4f uniform_MIT; // (Projection*ModelView).invert_transpose()
	Mat4x4f uniform_Mshadow; // transform framebuffer screen coordinates to shadowbuffer screen coordinates

	virtual Vec4f vertex(int iface, int nthvert){
		varying_uv.set_col(nthvert,scene->m_model->uv(iface,nthvert));
		varying_nrm.set_col(nthvert,Mat<float,3,1>(uniform_MIT*Vec4f(scene->m_model->normal(iface,nthvert),0)));
		Vec4f gl_vertex=uniform_M*Vec4f(scene->m_model->vert(iface,nthvert),1);
		varying_tri.set_col(nthvert,gl_vertex);
		ndc_tri.set_col(nthvert,Mat<float,3,1>(gl_vertex/gl_vertex.w()));
		return gl_vertex;
	}

	virtual bool fragment(Vec3f bar, TGAColor& color){
		Vec4f sb_p = uniform_Mshadow*varying_tri*bar; // corresponding point in the shadow buffer
        sb_p = sb_p/sb_p.w();
        int idx = int(sb_p.x()) + int(sb_p.y())*scene->m_WIDTH; // index in the shadowbuffer array
        float shadow = .3+.7*(scene->m_shadowbuffer[idx]<sb_p.z()+43.34); // magic coeff to avoid z-fighting

		Vec3f bn=(varying_nrm*bar).normalized();
		Vec2f uv=varying_uv*bar;

		Mat<float,3,3> A;
		A.set_row(0,(ndc_tri.get_col(1)-ndc_tri.get_col(0)).transpose());
		A.set_row(1,(ndc_tri.get_col(2)-ndc_tri.get_col(0)).transpose());
		A.set_row(2,bn.transpose());

		Mat<float,3,3> AI=A.inverse();

		Vec3f i = AI * Vec3f{varying_uv[0][1] - varying_uv[0][0], varying_uv[0][2] - varying_uv[0][0], 0};
        Vec3f j = AI * Vec3f{varying_uv[1][1] - varying_uv[1][0], varying_uv[1][2] - varying_uv[1][0], 0};

		Mat<float,3,3> B;
		B.set_col(0,i.normalized());
		B.set_col(1,j.normalized());
		B.set_col(2,bn);

		Vec3f n = (B*scene->m_model->normal(uv)).normalized();
		Vec3f l = scene->m_light.dir;
		Vec3f r = (n*(n.product(l)*2.f)-l).normalized();
		float spec = pow(std::max(0.f,r.z()), scene->m_model->specular(uv));
		float diffuse = std::max(0.f,n.product(l));
		TGAColor c = scene->m_model->diffuse(uv);
        color = c;
		TGAColor ambient = ao_tex->get(uv.x()*scene->m_WIDTH,uv.y()*scene->m_HEIGHT);
        for (int i=0; i<3; i++) color[i] = std::min<float>(c[i]*shadow*(1.2*diffuse + .6*spec+ ambient[i]/255.f*0.3), 255);
        return false;
    }

	virtual ~GouraudShader() {
		
	}
};