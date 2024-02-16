#pragma once
#include "GameObject.h"

namespace NCL {
	namespace CSC8503 {
		class Door : public GameObject {
		public:
			Door(){
				mName = "Door";
			}

			virtual void Open();
			virtual void Close();

		protected:
			bool mIsOpen;
		};
	}
}
