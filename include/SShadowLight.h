#ifndef __H_SSHADOW_LIGHT_INCLUDED__
#define __H_SSHADOW_LIGHT_INCLUDED__

#include <irrlicht.h>

namespace cp3d {
namespace rendering {

class CCP3DRenderingEngine;
class CCP3DHandler;

struct SShadowLight {
	friend class CCP3DRenderingEngine;
	friend class CCP3DHandler;
public:

	//! Constructor
	//! \param shadowMapResolution: the shadow map resolution
	//! \param position: the light's position
	//! \param target: the light's target
	//! \param lightColor: the light's color
	//! \param nearValue: the light's near value
	//! \param farValue: the light's far value
	//! \param fov: the light's front of view value
	//! \param directional: if the light is directional of not
	SShadowLight(irr::u32 shadowMapResolution = 1024,
				 irr::core::vector3df position = irr::core::vector3df(0.f),
				 irr::core::vector3df& target = irr::core::vector3df(0.f),
				 irr::video::SColorf lightColor = irr::video::SColor(0xffffffff), 
				 irr::f32 nearValue = 10.0f, irr::f32 farValue = 400.0f,
				 irr::f32 fov = 90.0f * irr::core::DEGTORAD64, bool directional = false)
				 :	Pos(position), Tar(target), FarPlane(directional ? 1.0f : farValue), DiffuseColor(lightColor), 
					MapRes(shadowMapResolution), AutoRecalculate(true), MustRecalculate(true)
	{
		nearValue = nearValue <= 0.0f ? 0.1f : nearValue;

		updateViewMatrix();
		
		if(directional)
			ProjMat.buildProjectionMatrixOrthoLH(fov, fov, nearValue, farValue);
		else
			ProjMat.buildProjectionMatrixPerspectiveFovLH(fov, 1.0f, nearValue, farValue);
	}

	//! Sets the light's Position
	//! \param position: the new position of the light
	void setPosition(const irr::core::vector3df& position) {
		Pos = position;
		updateViewMatrix();
	}

	//! Sets the light's Target
	//! \param target: the new target of the light
	void setTarget(const irr::core::vector3df& target) {
		Tar = target;
		updateViewMatrix();
	}

	//! Gets the light's position
	inline const irr::core::vector3df& getPosition() const {
		return Pos;
	}

	//! Gets the light's Target.
	inline const irr::core::vector3df& getTarget()  const {
		return Tar;
	}

	//! Sets the light's view matrix.
	void setViewMatrix(const irr::core::matrix4& matrix) {
		ViewMat = matrix;
		irr::core::matrix4 vInverse;
		ViewMat.getInverse(vInverse);
		Pos = vInverse.getTranslation();
	}

	//! Sets the light's projection matrix.
	void setProjectionMatrix(const irr::core::matrix4& matrix) {
		ProjMat = matrix;
	}

	//! Gets the light's view matrix.
	inline irr::core::matrix4& getViewMatrix() {
		return ViewMat;
	}

	//! Gets the light's projection matrix.
	inline irr::core::matrix4& getProjectionMatrix() {
		return ProjMat;
	}

	//! Gets the light's far value.
	inline irr::f32 getFarValue() const {
		return FarPlane;
	}

	//! Gets the light's color.
	inline const irr::video::SColorf& getLightColor() const {
		return DiffuseColor;
	}

	//! Sets the light's color
	//! \param lightColor: the new color of the light
	void setLightColor(const irr::video::SColorf& lightColor) {
		DiffuseColor = lightColor;
	}

	//! Sets the shadow map resolution for this light
	//! \param shadowMapResolution: the resolution of the shadow map
	void setShadowMapResolution(const irr::u32 shadowMapResolution) {
		MapRes = shadowMapResolution;
	}

	//! Gets the shadow map resolution for this light
	inline const irr::u32 getShadowMapResolution() const {
		return MapRes;
	}

	//! Gets if the light must recalculate
	inline const bool mustRecalculate() const {
		return MustRecalculate;
	}

	//! Sets if the light must recalculate
	//! \param mustRecalculate: true if light must recalculate
	void setMustRecalculate(const bool mustRecalculate) {
		MustRecalculate = mustRecalculate;
	}

	//! Gets if the light auto recalculate
	inline const bool mustAutoRecalculate() const {
		return AutoRecalculate;
	}

	//! Sets if the light must auto recalculate
	//! \param autoRecalculate: true if the light must auto recalculate
	void setMustAutoRecalculate(const bool autoRecalculate) {
		AutoRecalculate = autoRecalculate;
	}

private:

	void updateViewMatrix() {
		ViewMat.buildCameraLookAtMatrixLH(Pos, Tar,
			(Pos - Tar).dotProduct(irr::core::vector3df(1.0f, 0.0f, 1.0f)) == 0.0f ?
			irr::core::vector3df(0.0f, 0.0f, 1.0f) : irr::core::vector3df(0.0f, 1.0f, 0.0f)); 
	}

	irr::scene::ILightSceneNode *LightScenenode;
	irr::video::SColorf DiffuseColor;
	irr::core::vector3df Pos, Tar;
	irr::f32 FarPlane;
	irr::core::matrix4 ViewMat, ProjMat;
	irr::u32 MapRes;
	bool AutoRecalculate, MustRecalculate;
};

} /// End namespace rendering
} /// End namsepace cp3d

#endif
