#ifndef H_XEFFECTS_SQ
#define H_XEFFECTS_SQ

// Based on "CBaseFilter" by ItIsFree.
// Original thread: http://irrlicht.sourceforge.net/phpBB2/viewtopic.php?t=9857

#include <irrlicht.h>
#include "settings.h"

class CScreenQuad
{
public:
	CScreenQuad(bool useATI, bool pflip_vert)
	{
		Material.Wireframe = false;
		Material.Lighting = false;
		Material.ZWriteEnable = false;
		Material.BackfaceCulling = false;
		Material.ZBuffer = ECFN_ALWAYS/*ECFN_NEVER*/;
        //Material.setFlag(EMF_BILINEAR_FILTER, false);
        
        if ((useATI || pflip_vert) && !(useATI && pflip_vert))
        {
		  Vertices[0] = irr::video::S3DVertex(-1.0f,-1.0f,0.0f,0,0,1,irr::video::SColor(0x0),0.0f,0.0f);
		  Vertices[1] = irr::video::S3DVertex(-1.0f, 1.0f,0.0f,0,0,1,irr::video::SColor(0x0),0.0f,1.0f);
		  Vertices[2] = irr::video::S3DVertex( 1.0f, 1.0f,0.0f,0,0,1,irr::video::SColor(0x0),1.0f,1.0f);
		  Vertices[3] = irr::video::S3DVertex( 1.0f,-1.0f,0.0f,0,0,1,irr::video::SColor(0x0),1.0f,0.0f);
        }
        else
        {
		  Vertices[0] = irr::video::S3DVertex(-1.0f,-1.0f,0.0f,0,0,1,irr::video::SColor(0x0),0.0f,1.0f);
		  Vertices[1] = irr::video::S3DVertex(-1.0f, 1.0f,0.0f,0,0,1,irr::video::SColor(0x0),0.0f,0.0f);
		  Vertices[2] = irr::video::S3DVertex( 1.0f, 1.0f,0.0f,0,0,1,irr::video::SColor(0x0),1.0f,0.0f);
		  Vertices[3] = irr::video::S3DVertex( 1.0f,-1.0f,0.0f,0,0,1,irr::video::SColor(0x0),1.0f,1.0f);
        }
	}

	virtual void render(irr::video::IVideoDriver* driver)
	{
		irr::u16 indices[6] = {0, 1, 2, 3, 0, 2};

		driver->setMaterial(Material);
		driver->setTransform(irr::video::ETS_WORLD, irr::core::matrix4());
		driver->drawIndexedTriangleList(&Vertices[0], 4, &indices[0], 2);
        driver->setMaterial(irr::video::SMaterial());
	}

	virtual irr::video::SMaterial& getMaterial()
	{
		return Material;
	}
	
	void set2DVertexPos(int index, const irr::core::position2d<s32>& newPos, const irr::core::dimension2d<u32>& p_screenSize)
	{
        Vertices[index].Pos.X = 2.f * ((float)newPos.X / (float)p_screenSize.Width) - 1.f;
        Vertices[index].Pos.Y = (1.f - ((float)newPos.Y / (float)p_screenSize.Height)) * 2.f - 1.f;
    }

	//irr::video::ITexture* rt[2];

private:
	irr::video::S3DVertex Vertices[4];
	irr::video::SMaterial Material;
};

#endif
