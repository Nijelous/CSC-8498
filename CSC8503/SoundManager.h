#pragma once
#include "Vector3.h"


namespace NCL{
	using namespace Maths;
	namespace CSC8503 {
		class SoundManager {
		public:
			SoundManager();
			~SoundManager();

			/*vec3df ConvertToVec3df(Vector3 soundPos);
			
			ISound* AddWalkSound();

			ISound* AddSprintSound();

			void PlayOneTimeSound(Vector3 position);

			void SetSoundToBePaused(ISound* sound, bool isPaused);

			void SetSoundPosition(ISound* sound, Vector3 pos);

			void DeleteSounds();*/

		protected:
			FMOD::System* system = NULL;
			//ISound* mFootStep;
			//std::map<Sounds, ISound*(SoundManager::*)()> mSoundMap;
			/*std::vector<ISound*> mSounds;
			ISoundEngine* mSoundEngine;*/
		};
	}
}