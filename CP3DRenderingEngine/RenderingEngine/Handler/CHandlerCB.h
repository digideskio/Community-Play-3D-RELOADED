#ifndef __H_HANDLER_CB_INCLUDED__
#define __H_HANDLER_CB_INCLUDED__

#include "CHandler.h"

using namespace irr;
using namespace scene;
using namespace video;
using namespace core;

namespace cp3d {
namespace rendering {

class DepthShaderCB : public video::IShaderConstantSetCallBack {
public:
	DepthShaderCB(CCP3DHandler* effectIn) : effect(effectIn) { };

	virtual void OnSetConstants(video::IMaterialRendererServices* services, s32 userData) {
		IVideoDriver* driver = services->getVideoDriver();

		worldViewProj = driver->getTransform(video::ETS_PROJECTION);			
		worldViewProj *= driver->getTransform(video::ETS_VIEW);
		worldViewProj *= driver->getTransform(video::ETS_WORLD);

		services->setVertexShaderConstant("mWorldViewProj", worldViewProj.pointer(), 16);
		
		services->setVertexShaderConstant("MaxD", &FarLink, 1);
	}

	CCP3DHandler* effect;
	f32 FarLink;
	core::matrix4 worldViewProj;
};

class ShadowShaderCB : public video::IShaderConstantSetCallBack {

public:

	ShadowShaderCB(CCP3DHandler* effectIn) : effect(effectIn) { };

	virtual void OnSetMaterial(const SMaterial& material) { }

	virtual void OnSetConstants(video::IMaterialRendererServices* services, s32 userData) {
		IVideoDriver* driver = services->getVideoDriver();

		#ifdef _IRR_COMPILE_WITH_DIRECT3D_11_
		if (driver->getDriverType() == EDT_DIRECT3D11) {
			core::matrix4 worldTransposed = driver->getTransform(video::ETS_WORLD);
			worldTransposed = worldTransposed.getTransposed();
			services->setVertexShaderConstant("mWorldTrans", worldTransposed.pointer(), 16);
		}
		#endif

		matrix4 worldViewProj = driver->getTransform(video::ETS_PROJECTION);			
		worldViewProj *= driver->getTransform(video::ETS_VIEW);
		worldViewProj *= driver->getTransform(video::ETS_WORLD);
		services->setVertexShaderConstant("mWorldViewProj", worldViewProj.pointer(), 16);

		worldViewProj = ProjLink;
		worldViewProj *= ViewLink;
		worldViewProj *= driver->getTransform(video::ETS_WORLD);
		services->setVertexShaderConstant("mWorldViewProj2", worldViewProj.pointer(), 16);

		driver->getTransform(video::ETS_WORLD).getInverse(invWorld);
		vector3df lightPosOS = LightLink;
		invWorld.transformVect(lightPosOS); 
		services->setVertexShaderConstant("LightPos", reinterpret_cast<f32*>(&lightPosOS.X), 3);
		
		services->setVertexShaderConstant("MaxD", reinterpret_cast<f32*>(&FarLink), 1);
		services->setVertexShaderConstant("MAPRES", &MapRes, 1);

		services->setPixelShaderConstant("LightColour", reinterpret_cast<f32*>(&LightColour.r), 4);
	}

	CCP3DHandler* effect;
	core::matrix4 invWorld;

	video::SColorf LightColour;
	core::matrix4 ProjLink;
	core::matrix4 ViewLink;
	core::vector3df LightLink;
	f32 FarLink, MapRes;

};


class ScreenQuadCB : public irr::video::IShaderConstantSetCallBack {

public:

	ScreenQuadCB(CCP3DHandler* effectIn, bool defaultV = true) 
		: effect(effectIn), defaultVertexShader(defaultV) {};

	CCP3DHandler* effect;
	bool defaultVertexShader;

	virtual void OnSetConstants(irr::video::IMaterialRendererServices* services, irr::s32 userData) {
		if(services->getVideoDriver()->getDriverType() == irr::video::EDT_OPENGL) {
			irr::u32 TexVar = 0;
			services->setPixelShaderConstant("ColorMapSampler", (irr::s32*)(&TexVar), 1); 

			TexVar = 1;
			services->setPixelShaderConstant("ScreenMapSampler", (irr::s32*)(&TexVar), 1); 

			TexVar = 2;
			services->setPixelShaderConstant("DepthMapSampler", (irr::s32*)(&TexVar), 1); 

			TexVar = 3;
			services->setPixelShaderConstant("UserMapSampler", (irr::s32*)(&TexVar), 1);
		}

		if(defaultVertexShader) {
			const irr::core::dimension2du currentRTTSize = services->getVideoDriver()->getCurrentRenderTargetSize();
			const irr::f32 screenX = (irr::f32)currentRTTSize.Width, screenY = (irr::f32)currentRTTSize.Height;

			services->setVertexShaderConstant("screenX", &screenX, 1);
			services->setVertexShaderConstant("screenY", &screenY, 1);

			irr::scene::ISceneManager* smgr = effect->getActiveSceneManager();
			irr::scene::ICameraSceneNode* cam = smgr->getActiveCamera();

			const irr::core::position2di tLeft = services->getVideoDriver()->getViewPort().UpperLeftCorner;
			const irr::core::position2di bRight = services->getVideoDriver()->getViewPort().LowerRightCorner;

			const irr::core::line3df sLines[4] = {
				smgr->getSceneCollisionManager()->getRayFromScreenCoordinates
				(irr::core::position2di(tLeft.X, tLeft.Y), cam),
				smgr->getSceneCollisionManager()->getRayFromScreenCoordinates
				(irr::core::position2di(bRight.X, tLeft.Y), cam),
				smgr->getSceneCollisionManager()->getRayFromScreenCoordinates
				(irr::core::position2di(tLeft.X, bRight.Y), cam),
				smgr->getSceneCollisionManager()->getRayFromScreenCoordinates
				(irr::core::position2di(bRight.X, bRight.Y), cam)
			};

			services->setVertexShaderConstant("LineStarts0", &sLines[0].start.X, 3);
			services->setVertexShaderConstant("LineStarts1", &sLines[1].start.X, 3);
			services->setVertexShaderConstant("LineStarts2", &sLines[2].start.X, 3);
			services->setVertexShaderConstant("LineStarts3", &sLines[3].start.X, 3);

			services->setVertexShaderConstant("LineEnds0", &sLines[0].end.X, 3);
			services->setVertexShaderConstant("LineEnds1", &sLines[1].end.X, 3);
			services->setVertexShaderConstant("LineEnds2", &sLines[2].end.X, 3);
			services->setVertexShaderConstant("LineEnds3", &sLines[3].end.X, 3);
		}

		if(uniformDescriptors.size()) {
			irr::core::map<irr::core::stringc, SUniformDescriptor>::Iterator mapIter = uniformDescriptors.getIterator();

			for(;!mapIter.atEnd();mapIter++) {
				if(mapIter.getNode()->getValue().fPointer == 0)
					continue;

				services->setPixelShaderConstant(mapIter.getNode()->getKey().c_str(), mapIter.getNode()->getValue().fPointer,
					mapIter.getNode()->getValue().paramCount);
			}
		}
	}

	struct SUniformDescriptor {
		SUniformDescriptor() : fPointer(0), paramCount(0) {}

		SUniformDescriptor(const irr::f32* fPointerIn, irr::u32 paramCountIn)
			: fPointer(fPointerIn), paramCount(paramCountIn) {}

		const irr::f32* fPointer;
		irr::u32 paramCount;
	};

	irr::core::map<irr::core::stringc, SUniformDescriptor> uniformDescriptors;
};

} /// End namespace rendering
} /// End namespace cp3d

#endif
