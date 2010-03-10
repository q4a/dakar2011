#ifndef H_XEFFECTS_Q
#define H_XEFFECTS_Q

// Based on "CBaseFilter" by ItIsFree.
// Original thread: http://irrlicht.sourceforge.net/phpBB2/viewtopic.php?t=9857

#include <irrlicht.h>
#include "settings.h"

class CQuad
{
public:
	CQuad(core::vector3df offset, core::vector3df scale, bool useATI = false, bool pflip_vert = false)
	{
		Material.Wireframe = false;
		Material.Lighting = false;
		Material.ZWriteEnable = false;
        //Material.setFlag(EMF_BILINEAR_FILTER, false);

        if ((useATI || pflip_vert) && !(useATI && pflip_vert))
        //if ((useATI && !pflip_vert) && !(useATI && pflip_vert))
        {
		  Vertices[0] = irr::video::S3DVertex(-1.0f*scale.X+offset.X,-1.0f*scale.Y+offset.Y,0.0f,0,0,1,irr::video::SColor(0x0),0.0f,0.0f);
		  Vertices[1] = irr::video::S3DVertex(-1.0f*scale.X+offset.X, 1.0f*scale.Y+offset.Y,0.0f,0,0,1,irr::video::SColor(0x0),0.0f,1.0f);
		  Vertices[2] = irr::video::S3DVertex( 1.0f*scale.X+offset.X, 1.0f*scale.Y+offset.Y,0.0f,0,0,1,irr::video::SColor(0x0),1.0f,1.0f);
		  Vertices[3] = irr::video::S3DVertex( 1.0f*scale.X+offset.X,-1.0f*scale.Y+offset.Y,0.0f,0,0,1,irr::video::SColor(0x0),1.0f,0.0f);
		  Vertices[4] = irr::video::S3DVertex(-1.0f*scale.X+offset.X,-1.0f*scale.Y+offset.Y,0.0f,0,0,1,irr::video::SColor(0x0),0.0f,0.0f);
		  Vertices[5] = irr::video::S3DVertex( 1.0f*scale.X+offset.X, 1.0f*scale.Y+offset.Y,0.0f,0,0,1,irr::video::SColor(0x0),1.0f,1.0f);
        }
        else
        {
		  Vertices[0] = irr::video::S3DVertex(-1.0f*scale.X+offset.X,-1.0f*scale.Y+offset.Y,0.0f,0,0,1,irr::video::SColor(0x0),0.0f,1.0f);
		  Vertices[1] = irr::video::S3DVertex(-1.0f*scale.X+offset.X, 1.0f*scale.Y+offset.Y,0.0f,0,0,1,irr::video::SColor(0x0),0.0f,0.0f);
		  Vertices[2] = irr::video::S3DVertex( 1.0f*scale.X+offset.X, 1.0f*scale.Y+offset.Y,0.0f,0,0,1,irr::video::SColor(0x0),1.0f,0.0f);
		  Vertices[3] = irr::video::S3DVertex( 1.0f*scale.X+offset.X,-1.0f*scale.Y+offset.Y,0.0f,0,0,1,irr::video::SColor(0x0),1.0f,1.0f);
		  Vertices[4] = irr::video::S3DVertex(-1.0f*scale.X+offset.X,-1.0f*scale.Y+offset.Y,0.0f,0,0,1,irr::video::SColor(0x0),0.0f,1.0f);
		  Vertices[5] = irr::video::S3DVertex( 1.0f*scale.X+offset.X, 1.0f*scale.Y+offset.Y,0.0f,0,0,1,irr::video::SColor(0x0),1.0f,0.0f);
        }
        centre = (Vertices[0].Pos + Vertices[5].Pos) / 2;
	}

	virtual void render(irr::video::IVideoDriver* driver)
	{
        //if (!visible) return;
		irr::u16 indices[6] = {0, 1, 2, 3, 4, 5};

		driver->setMaterial(Material);
		driver->setTransform(irr::video::ETS_WORLD, irr::core::matrix4());
		driver->drawIndexedTriangleList(&Vertices[0], 6, &indices[0], 2);
	}

	virtual irr::video::SMaterial& getMaterial()
	{
		return Material;
	}   
    
    virtual void setVisible(bool vis)
    {
        visible = vis;
    }

	irr::video::ITexture* texture;

private:
	irr::video::S3DVertex Vertices[6];
    core::vector3df centre;
	irr::video::SMaterial Material;
    bool visible;
};

#endif
