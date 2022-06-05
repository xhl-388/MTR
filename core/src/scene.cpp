#include <scene.h>
#include <ishader.h>
#include <gouraudshader.h>
#include <zshader.h>
#include <depthshader.h>

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

Scene::~Scene() {
    delete m_zbuffer;
    delete m_shadowbuffer;
}

void Scene::Render() {
	m_image.clear();
	m_depthImg.clear();
	m_frame.clear();
	std::fill(m_zbuffer, m_zbuffer + m_WIDTH*m_HEIGHT, -std::numeric_limits<float>::max());
	std::fill(m_shadowbuffer, m_shadowbuffer + m_WIDTH*m_HEIGHT,
                -std::numeric_limits<float>::max());

    Vec3f cameraPos = m_camera.GetPosition();
    // first pass : shadow pass
	{
		m_camera.SetPosition(m_light.origin);
		SetProjectionCoeff(0);

		DepthShader depthShader;
        depthShader.scene = this;

		for (int i = 0; i < m_model->nfaces(); i++)
		{
			for (int j = 0; j < 3; j++){
				depthShader.vertex(i,j);
			}
			triangle(depthShader.varying_tri, m_depthImg, depthShader, m_shadowbuffer);
		}
	}

	{
		m_camera.SetPosition(cameraPos);
		SetProjectionCoeff(-1.f/(cameraPos-m_camera.GetLookPoint()).norm());

		ZShader zshader;
        zshader.scene = this;
		for (int i=0; i<m_model->nfaces(); i++) {
			for (int j=0; j<3; j++) {
				zshader.vertex(i, j);
			}
			triangle(zshader.varying_tri, m_frame, zshader, m_zbuffer);
		}
		
		omp_set_num_threads(20);
		#pragma omp parallel for
		for (int x=0; x<m_WIDTH; x++) {
			for (int y=0; y<m_HEIGHT; y++) {
				if (m_zbuffer[x+y*m_WIDTH] < -1e5) continue;
				float total = 0;
				for (float a=0; a<M_PI*2-1e-4; a += M_PI/4) {
					total += M_PI/2 - max_elevation_angle(m_zbuffer, Vec2f{x, y}, 
						Vec2f{std::cos(a), std::sin(a)});
				}
				total /= (M_PI/2)*8;
				total = std::pow(total, 100.f);
				int val = std::min(255,int(total*255));
				m_frame.set(x, y, TGAColor(val,val,val));
			}
		}
		std::fill(m_zbuffer,m_zbuffer+m_WIDTH*m_HEIGHT,-std::numeric_limits<float>::max());
	}

	Mat4x4f M = m_viewportMat*m_proMat*m_camera.GetViewMatrix();

	// second pass : render pass
	{
		GouraudShader shader;
        shader.scene = this;
		shader.uniform_M=m_proMat*m_camera.GetViewMatrix();
		shader.uniform_MIT=shader.uniform_M.inverse().transpose();
		shader.uniform_Mshadow = M*(shader.uniform_M.inverse());
		shader.ao_tex = &m_frame;
		for (int i = 0; i < m_model->nfaces(); i++)
		{
			for (int j = 0; j < 3; j++){
				shader.vertex(i,j);
			}
			triangle(shader.varying_tri, m_image, shader, m_zbuffer);
		}
	}
}

float Scene::max_elevation_angle(float *buffer, Vec2f p, Vec2f dir) {
    float maxangle = 0;
    for (float t=0.; t<10.; t+=1.) {
        Vec2f cur = p + dir*t;
        if (cur.x()>=m_WIDTH || cur.y()>=m_HEIGHT || cur.x()<0 || cur.y()<0
			||buffer[int(cur.x())+int(cur.y())*m_WIDTH]==-std::numeric_limits<float>::max()) 
			return maxangle;
        float distance = (p-cur).norm();
        if (distance < 1.f) continue;
        float elevation = (buffer[int(cur.x())+int(cur.y())*m_WIDTH]-buffer[int(p.x())+int(p.y())*m_WIDTH])/m_DEPTH;
		// std::cout<<elevation<<std::endl;
        maxangle = std::max(maxangle, atanf(elevation/distance));
    }
    return maxangle;
}

Vec3f Scene::barycentric(Vec2f A, Vec2f B, Vec2f C, Vec2f P)
{
	Vec3f u = Vec3f{B.x() - A.x(), C.x() - A.x(), A.x() - P.x()}.cross_product(
		Vec3f{B.y() - A.y(), C.y() - A.y(), A.y() - P.y()});
	if (std::abs(u.z()) < 1e-2)
		return Vec3f{-1, 1, 1};
	return Vec3f{1.f - (u.x() + u.y()) / u.z(), u.x() / u.z(), u.y() / u.z()};
}

void Scene::triangle(Mat<float,4,3>& clipc, TGAImage &image, IShader& shader, float* zbuffer)
{
	Mat<float,3,4> pts=(m_viewportMat*clipc).transpose();
	Mat<float,3,2> pts2;
	for(int i=0;i<3;i++)
		pts2.set_row(i,Mat<float,1,2>(pts.get_row(i)/pts[i][3]));
    Vec2f bboxmin{ std::numeric_limits<float>::max(),  std::numeric_limits<float>::max()};
    Vec2f bboxmax{-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max()};
	Vec2f clamp{image.get_width()-1, image.get_height()-1};
    for (int i=0; i<3; i++) {
        for (int j=0; j<2; j++) {
            bboxmin[j][0] = std::max(0.f,std::min(bboxmin[j][0], pts2[i][j]));
            bboxmax[j][0] = std::min(clamp[j][0], std::max(bboxmax[j][0], pts2[i][j]));
        }
    }

	TGAColor color(255,255,255);
	for (int Px = bboxmin.x(); Px <= bboxmax.x(); Px++)
	{
		for (int Py = bboxmin.y(); Py <= bboxmax.y(); Py++)
		{
			Vec3f bc_screen = barycentric(pts2.get_row(0).transpose(), pts2.get_row(1).transpose(), 
				pts2.get_row(2).transpose(), Vec2f{Px,Py});
			Vec3f bc_clip = Vec3f{bc_screen.x()/pts[0][3], bc_screen.y()/pts[1][3], 
				bc_screen.z()/pts[2][3]};
			bc_clip = bc_clip/(bc_clip.x()+bc_clip.y()+bc_clip.z());
			float frag_depth=Vec3f{pts[0][2]/pts[0][3],pts[1][2]/pts[1][3],pts[2][2]/pts[2][3]}.product(bc_clip);
			
			if (bc_screen.x() < 0 || bc_screen.y() < 0 || bc_screen.z() < 0 ||
				zbuffer[Px+Py*image.get_width()]> frag_depth)
				continue;
			bool discard = shader.fragment(bc_clip,color);
			if (!discard)
			{
                zbuffer[Px+Py*image.get_width()] = frag_depth;
				image.set(Px, Py, color);
			}
		}
	}
}

void Scene::WriteBuffersIntoFiles() {
	m_depthImg.flip_vertically();
	m_depthImg.write_tga_file(FileNameConcat(m_outputDir, "depth.tga"));
	m_depthImg.flip_vertically();

	m_frame.flip_vertically();
	m_frame.write_tga_file(FileNameConcat(m_outputDir, "framebuffer.tga"));
	m_frame.flip_vertically();

	m_image.flip_vertically();
	m_image.write_tga_file(FileNameConcat(m_outputDir, "output.tga"));
	m_image.flip_vertically();
}