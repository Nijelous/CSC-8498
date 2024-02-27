#pragma once

using std::vector;

using namespace NCL::Maths;

namespace NCL {
	namespace CSC8503 {
		class Transform
		{
		public:
			Transform();
			~Transform();

			Transform& SetPosition(const Vector3& worldPos);
			Transform& SetScale(const Vector3& worldScale);
			Transform& SetOrientation(const Quaternion& newOr);

			Vector3 GetPosition() const {
				return position;
			}

			Vector3 GetScale() const {
				return scale;
			}

			Quaternion GetOrientation() const {
				return orientation;
			}

			Matrix4 GetMatrix() const {
				return mMatrix;
			}

			void SetMatrix(Matrix4 matrix) {
				mMatrix = matrix;
			}
			void UpdateMatrix();

			inline bool operator<(const Transform& rhs) const { return this->GetPosition().x == rhs.GetPosition().x ? 
				(this->GetPosition().y == rhs.GetPosition().y ? this->GetPosition().z < rhs.GetPosition().z : this->GetPosition().y < rhs.GetPosition().y) 
				: this->GetPosition().x < rhs.GetPosition().x; }
		protected:
			Matrix4		mMatrix;
			Quaternion	orientation;
			Vector3		position;

			Vector3		scale;
		};
	}
}

