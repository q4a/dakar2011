/****************************************************************
*                                                               *
*    Name: my_shaders.cpp                                       *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*       This file contains the shader handling.                 *
*                                                               *
****************************************************************/

#include "my_shaders.h"
#include "settings.h"
#include "gameplay.h"

#ifdef __linux__
#include "linux_includes.h"
#endif


class MyShaderCallBack : public video::IShaderConstantSetCallBack
{
public:
    MyShaderCallBack(IrrlichtDevice* pdevice, scene::ILightSceneNode* plnode) :
        video::IShaderConstantSetCallBack(), device(pdevice), m_lnode(plnode) {}
    
	virtual void OnSetConstants(video::IMaterialRendererServices* services,
			s32 userData)
	{
		video::IVideoDriver* driver = services->getVideoDriver();
        //printf("onsetconst\n");

		// set inverted world matrix
		// if we are using highlevel shaders (the user can select this when
		// starting the program), we must set the constants by name.
		core::matrix4 invWorld = driver->getTransform(video::ETS_WORLD);
		invWorld.makeInverse();
		services->setVertexShaderConstant("mInvWorld", invWorld.pointer(), 16);

		// set clip matrix

		core::matrix4 worldViewProj;
		worldViewProj = driver->getTransform(video::ETS_PROJECTION);			
		worldViewProj *= driver->getTransform(video::ETS_VIEW);
		worldViewProj *= driver->getTransform(video::ETS_WORLD);
		services->setVertexShaderConstant("mWorldViewProj", worldViewProj.pointer(), 16);

		// set camera position

		core::vector3df pos = m_lnode->getLightData().Position;//m_lnode->getPosition();
        //device->getSceneManager()->getActiveCamera()->getAbsolutePosition();

		services->setVertexShaderConstant("mLightPos", reinterpret_cast<f32*>(&pos), 3);

		// set light color

		video::SColorf col = m_lnode->getLightData().DiffuseColor;//(0.0f,1.0f,1.0f,0.0f);

		services->setVertexShaderConstant("mLightColor", reinterpret_cast<f32*>(&col), 4);

		// set transposed world matrix
	    //core::matrix4 world = driver->getTransform(video::ETS_WORLD).getTransposed();
		//world = world.getTransposed();
		//services->setVertexShaderConstant("mTransWorld", world.pointer(), 16);
		
        if (driverType == video::EDT_OPENGL	)
        {
            if (UsedMaterial && UsedMaterial->getTexture(0))
            {
                //printf("tex0\n");
                int num = 0;
                services->setPixelShaderConstant("tex0", (float*)(&num), 1);
            }
            if (UsedMaterial && UsedMaterial->getTexture(1))
            {
                //printf("tex1\n");
                int num = 1;
                services->setPixelShaderConstant("tex1", (float*)(&num), 1);
            }
            if (UsedMaterial && UsedMaterial->getTexture(2))
            {
                int num = 2;
                services->setPixelShaderConstant("envMap", (float*)(&num), 1);
            }
            if (UsedMaterial && UsedMaterial->getTexture(3))
            {
                int num = 3;
                services->setPixelShaderConstant("tex3", (float*)(&num), 1);
            }
            if (UsedMaterial && UsedMaterial->getTexture(4))
            {
                int num = 4;
                services->setPixelShaderConstant("tex4", (float*)(&num), 1);
            }
            if (UsedMaterial && UsedMaterial->getTexture(5))
            {
                int num = 5;
                services->setPixelShaderConstant("tex5", (float*)(&num), 1);
            }
        }
        if (UsedMaterial && UsedMaterial->getTexture(2) && !UsedMaterial->getTexture(3))
        {
    		core::vector3df pos = device->getSceneManager()->getActiveCamera()->getAbsolutePosition();
    		services->setVertexShaderConstant("eyePosition", reinterpret_cast<f32*>(&pos), 3);
        }
        //float num = 1.0f;
        //services->setPixelShaderConstant("myMulti", &num/*UsedMaterial->getTextureMatrix(1).pointer()*/, 1);
			   
        UsedMaterial = 0;
	}
	
    void OnSetMaterial(const video::SMaterial& material)
    {
        //printf("onsetmaterial\n");
        UsedMaterial=&material;
    }


public:
	IrrlichtDevice* device;
	scene::ILightSceneNode* m_lnode;
	const video::SMaterial *UsedMaterial;
};


void setupShaders (IrrlichtDevice* device,
                   IVideoDriver* driver,
                   video::E_DRIVER_TYPE driverType,
                   ISceneManager* smgr,
                   scene::ICameraSceneNode* camera,
                   bool usehls,
                   scene::ILightSceneNode* plnode
                   )
{
#ifdef IRRLICHT_SDK_15
	const c8* light_tex_vsFileName; // filename for the vertex shader
	const c8* light_tex_psFileName; // filename for the pixel shader

	const c8* light_tex_car_vsFileName; // filename for the vertex shader
	const c8* light_tex_car_psFileName; // filename for the pixel shader

	const c8* light_notex_vsFileName; // filename for the vertex shader
	const c8* light_notex_psFileName; // filename for the pixel shader

	const c8* light_notex_car_vsFileName; // filename for the vertex shader
	const c8* light_notex_car_psFileName; // filename for the pixel shader

	const c8* light_2tex_vsFileName; // filename for the vertex shader
	const c8* light_2tex_psFileName; // filename for the pixel shader

	const c8* light_2tex_2_vsFileName; // filename for the vertex shader
	const c8* light_2tex_2_psFileName; // filename for the pixel shader

    const c8* smoke_psFileName; // filename for the pixel shader
#define NULLSTRING 0    
#else
#ifdef __linux___if_old_irrlicht_svn_
	core::string<c16> light_tex_vsFileName; // filename for the vertex shader
	core::string<c16> light_tex_psFileName; // filename for the pixel shader

	core::string<c16> light_tex_car_vsFileName; // filename for the vertex shader
	core::string<c16> light_tex_car_psFileName; // filename for the pixel shader

	core::string<c16> light_notex_vsFileName; // filename for the vertex shader
	core::string<c16> light_notex_psFileName; // filename for the pixel shader

	core::string<c16> light_notex_car_vsFileName; // filename for the vertex shader
	core::string<c16> light_notex_car_psFileName; // filename for the pixel shader

	core::string<c16> light_2tex_vsFileName; // filename for the vertex shader
	core::string<c16> light_2tex_psFileName; // filename for the pixel shader

	core::string<c16> light_2tex_2_vsFileName; // filename for the vertex shader
	core::string<c16> light_2tex_2_psFileName; // filename for the pixel shader

    core::string<c16> smoke_psFileName; // filename for the pixel shader
#define NULLSTRING ""
#else
	io::path light_tex_vsFileName; // filename for the vertex shader
	io::path light_tex_psFileName; // filename for the pixel shader

	io::path light_tex_car_vsFileName; // filename for the vertex shader
	io::path light_tex_car_psFileName; // filename for the pixel shader

	io::path light_notex_vsFileName; // filename for the vertex shader
	io::path light_notex_psFileName; // filename for the pixel shader

	io::path light_notex_car_vsFileName; // filename for the vertex shader
	io::path light_notex_car_psFileName; // filename for the pixel shader

	io::path light_2tex_vsFileName; // filename for the vertex shader
	io::path light_2tex_psFileName; // filename for the pixel shader

	io::path light_2tex_2_vsFileName; // filename for the vertex shader
	io::path light_2tex_2_psFileName; // filename for the pixel shader

    io::path smoke_psFileName; // filename for the pixel shader
    io::path nullpath;
#define NULLSTRING nullpath
#endif // __linux__
#endif // IRRLICHT_SDK_15
    dprintf(printf("T&L: %d\n", driver->queryFeature(video::EVDF_HARDWARE_TL)));
    dprintf(printf("Multitexturing: %d\n", driver->queryFeature(video::EVDF_MULTITEXTURE)));
    dprintf(printf("Stencil buffer: %d\n", driver->queryFeature(video::EVDF_STENCIL_BUFFER)));
    dprintf(printf("Vertex shader 1.1: %d\n", driver->queryFeature(video::EVDF_VERTEX_SHADER_1_1)));
    dprintf(printf("Vertex shader 2.0: %d\n", driver->queryFeature(video::EVDF_VERTEX_SHADER_2_0)));
    dprintf(printf("Vertex shader 3.0: %d\n", driver->queryFeature(video::EVDF_VERTEX_SHADER_3_0)));
    dprintf(printf("Pixel shader 1.1: %d\n", driver->queryFeature(video::EVDF_PIXEL_SHADER_1_1)));
    dprintf(printf("Pixel shader 1.2: %d\n", driver->queryFeature(video::EVDF_PIXEL_SHADER_1_2)));
    dprintf(printf("Pixel shader 1.3: %d\n", driver->queryFeature(video::EVDF_PIXEL_SHADER_1_3)));
    dprintf(printf("Pixel shader 1.4: %d\n", driver->queryFeature(video::EVDF_PIXEL_SHADER_1_4)));
    dprintf(printf("Pixel shader 2.0: %d\n", driver->queryFeature(video::EVDF_PIXEL_SHADER_2_0)));
    dprintf(printf("Pixel shader 3.0: %d\n", driver->queryFeature(video::EVDF_PIXEL_SHADER_3_0)));
    dprintf(printf("ARB vertex program: %d\n", driver->queryFeature(video::EVDF_ARB_VERTEX_PROGRAM_1)));
    dprintf(printf("ARB fragment program: %d\n", driver->queryFeature(video::EVDF_ARB_FRAGMENT_PROGRAM_1)));
    dprintf(printf("GLSL: %d\n", driver->queryFeature(video::EVDF_ARB_GLSL)));
    dprintf(printf("HLSL: %d\n", driver->queryFeature(video::EVDF_HLSL)));

	if (((driverType == video::EDT_DIRECT3D9 && driver->queryFeature(video::EVDF_HLSL)) ||
		 (driverType == video::EDT_OPENGL && driver->queryFeature(video::EVDF_ARB_GLSL))) &&
         (driver->queryFeature(video::EVDF_PIXEL_SHADER_2_0) || driver->queryFeature(video::EVDF_ARB_FRAGMENT_PROGRAM_1)) && 
         (driver->queryFeature(video::EVDF_VERTEX_SHADER_2_0) || driver->queryFeature(video::EVDF_ARB_VERTEX_PROGRAM_1))  
         && usehls)
	{
		dprintf(printf("use high level shaders.\n"));
	}
	else
	{
		dprintf(printf("Not use high level shaders, because of missing driver/hardware support.\n"));
        useShaders = false;
        return;
    }
    
	switch(driverType)
	{
	case video::EDT_DIRECT3D9:
			light_tex_psFileName = "data/shaders/light_tex_9.hlsl";
			light_tex_vsFileName = light_tex_psFileName; // both shaders are in the same file
			light_tex_car_psFileName = "data/shaders/light_tex_car_9.hlsl";
			light_tex_car_vsFileName = light_tex_car_psFileName; // both shaders are in the same file
			light_notex_psFileName = "data/shaders/light_notex_9.hlsl";
			light_notex_vsFileName = light_notex_psFileName;
			light_notex_car_psFileName = "data/shaders/light_notex_car_9.hlsl";
			light_notex_car_vsFileName = light_notex_psFileName;
			smoke_psFileName = "data/shaders/smoke_9.hlsl";
			light_2tex_psFileName = "data/shaders/light_2tex_9.hlsl";
			light_2tex_vsFileName = light_2tex_psFileName;
			light_2tex_2_psFileName = "data/shaders/light_2tex_2_9.hlsl";
			light_2tex_2_vsFileName = light_2tex_psFileName;
		break;

	case video::EDT_OPENGL:
			light_tex_psFileName = "data/shaders/light_tex_gl.frag";
			light_tex_vsFileName = "data/shaders/light_tex_gl.vert";
			light_tex_car_psFileName = "data/shaders/light_tex_car_gl.frag";
			light_tex_car_vsFileName = "data/shaders/light_tex_car_gl.vert";
			light_notex_psFileName = "data/shaders/light_notex_gl.frag";
			light_notex_vsFileName = "data/shaders/light_notex_gl.vert";
			light_notex_car_psFileName = "data/shaders/light_notex_car_gl.frag";
			light_notex_car_vsFileName = "data/shaders/light_notex_car_gl.vert";
			light_2tex_psFileName = "data/shaders/light_2tex_gl.frag";
			light_2tex_vsFileName = "data/shaders/light_2tex_gl.vert";
			light_2tex_2_psFileName = "data/shaders/light_2tex_2_gl.frag";
			light_2tex_2_vsFileName = "data/shaders/light_2tex_gl.vert";
			smoke_psFileName = "data/shaders/smoke_gl.frag";
		break;
	}

	if (!driver->queryFeature(video::EVDF_PIXEL_SHADER_2_0) &&
		!driver->queryFeature(video::EVDF_ARB_FRAGMENT_PROGRAM_1))
	{
		dprintf(printf("WARNING: Pixel shaders disabled "\
			"because of missing driver/hardware support.\n"));
            useShaders = false;
	}
	
	if (!driver->queryFeature(video::EVDF_VERTEX_SHADER_2_0) &&
		!driver->queryFeature(video::EVDF_ARB_VERTEX_PROGRAM_1))
	{
		dprintf(printf("WARNING: Vertex shaders disabled "\
			"because of missing driver/hardware support.\n"));
            useShaders = false;
	}

    if (!useShaders) return;
    video::IGPUProgrammingServices* gpu = driver->getGPUProgrammingServices();

	if (gpu)
	{
		MyShaderCallBack* mc = new MyShaderCallBack(device, plnode);

		// create the shaders depending on if the user wanted high level
		// or low level shaders:
//if (!useCgShaders){
			// create material from high level shaders (hlsl or glsl)
		myMaterialType_light_tex = myMaterialType_light_tex_wfar = gpu->addHighLevelShaderMaterialFromFiles(
			light_tex_vsFileName, "vertexMain", video::EVST_VS_2_0,
			light_tex_psFileName, "pixelMain", video::EPST_PS_2_0,
			mc, /*video::EMT_SOLID*/video::EMT_TRANSPARENT_ALPHA_CHANNEL);

		myMaterialType_light_tex_s_car = gpu->addHighLevelShaderMaterialFromFiles(
			light_tex_car_vsFileName, "vertexMain", video::EVST_VS_2_0,
			light_tex_car_psFileName, "pixelMain", video::EPST_PS_2_0,
			mc, video::EMT_SOLID);

		myMaterialType_light_notex_car = myMaterialType_light_notex = myMaterialType_light_notex_wfar = 
          gpu->addHighLevelShaderMaterialFromFiles(
			light_notex_vsFileName, "vertexMain", video::EVST_VS_2_0,
			light_notex_psFileName, "pixelMain", video::EPST_PS_2_0,
			mc, video::EMT_SOLID);

		myMaterialType_light_2tex = gpu->addHighLevelShaderMaterialFromFiles(
			light_2tex_vsFileName, "vertexMain", video::EVST_VS_2_0,
			light_2tex_psFileName, "pixelMain", video::EPST_PS_2_0,
			mc, video::EMT_SOLID/*video::EMT_DETAIL_MAP*/);

		myMaterialType_light_2tex_2 = gpu->addHighLevelShaderMaterialFromFiles(
			light_2tex_2_vsFileName, "vertexMain", video::EVST_VS_2_0,
			light_2tex_2_psFileName, "pixelMain", video::EPST_PS_2_0,
			mc, video::EMT_SOLID/*video::EMT_DETAIL_MAP*/);
//}
		myMaterialType_smoke = gpu->addHighLevelShaderMaterialFromFiles(
			NULLSTRING, "vertexMain", video::EVST_VS_2_0,
			smoke_psFileName, "pixelMain", video::EPST_PS_2_0,
			mc, /*video::EMT_SOLID*/video::EMT_TRANSPARENT_ALPHA_CHANNEL);

		mc->drop();
	}
    
}

