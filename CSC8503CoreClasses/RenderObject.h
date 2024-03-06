#pragma once
#include "Texture.h"
#include "Shader.h"
#include "Mesh.h"

#include "MeshAnimation.h"
#include "MeshMaterial.h"
#ifdef USEGL
#include <glad/gl.h>
#endif

#include "Transform.h"
#include <AnimationObject.h>


namespace NCL {
	using namespace NCL::Rendering;

	namespace CSC8503 {
		class Transform;
		using namespace Maths;

		class RenderObject
		{
		public:
			RenderObject(Transform* parentTransform, Mesh* mesh, Texture* albedo, Texture* normal, Shader* shader, float cullSphereRadius);
			RenderObject(Transform* parentTransform, Mesh* mesh, Texture* albedo, Texture* normal, Shader* shader, Vector4 colour, float cullSphereRadius);
			~RenderObject();

			void SetAlbedoTexture(Texture* t) {
				mAlbedoTex = t;
			}

			Texture* GetAlbedoTexture() const {
				return mAlbedoTex;
			}

			void SetNormalTexture(Texture* t) {
				mNormalTex = t;
			}

			Texture* GetNormalTexture() const {
				return mNormalTex;
			}

			Mesh*	GetMesh() const {
				return mMesh;
			}

			Transform*		GetTransform() const {
				return mTransform;
			}

			Shader*		GetShader() const {
				return mShader;
			}

			void SetColour(const Vector4& c) {
				mColour = c;
			}

			Vector4 GetColour() const {
				return mColour;
			}

			float GetCullSphereRadius() const {
				return mCullSphereRadius;
			}

			float GetSqDistToCam() const {
				return mSqDistToCam;
			}

			void SetSqDistToCam(const Vector3& camPos) {
				mSqDistToCam = (camPos - mTransform->GetPosition()).LengthSquared();
			}

			void SetSqDistToCam(float sqDist) {
				mSqDistToCam = sqDist;
			}

			static bool CompareBySqCamDist(const RenderObject* a, const RenderObject* b) {
				return(a->mSqDistToCam < b->mSqDistToCam) ? true : false;
			}


			void SetOutlined(bool outlined) {
				mOutlined = outlined;
			}

			bool GetOutlined() const {
				return mOutlined;
			}

			void SetIsInstanced(bool isInstanced) {
				mIsInstanced = isInstanced;
			}

			bool IsInstanced() const {
				return mIsInstanced;
			}


			void SetCurrentFrame(int currentFrame) {
				mCurrentFrame = currentFrame;
			}

			int GetCurrentFrame() const  {
				return mCurrentFrame;
			}

			void SetMatTextures(vector<GLuint> matTextures) {
				mMatTextures = matTextures;
			}

			vector<GLuint>  GetMatTextures() const{
				return mMatTextures;
			}

			void SetFrameMatricesVec(std::vector<std::vector<Matrix4>> frameMatrices) {
				mFrameMatricesVec = frameMatrices;
			}

			std::vector<std::vector<Matrix4>>  GetFrameMatricesVec() const {
				return mFrameMatricesVec;
			}

			AnimationObject* GetAnimationObject() const {
				return mAnimationObject;
			}

			void SetAnimationObject(AnimationObject* newObject) {
				mAnimationObject = newObject;
			}



		protected:
			Mesh*		mMesh;
			Texture* mAlbedoTex;
			Texture* mNormalTex;
			Shader*		mShader;
			Transform*	mTransform;
	

			Vector4		mColour;
			AnimationObject* mAnimationObject;

			float		mCullSphereRadius;
			float mSqDistToCam;
			bool mOutlined = false;
			bool mIsInstanced = false;

			vector<GLuint>  mMatTextures;
			std::vector<std::vector<Matrix4>> mFrameMatricesVec;


			int		mCurrentFrame;
			
		};
	}
}
