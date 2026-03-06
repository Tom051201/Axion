
namespace AxionScriptCore {

	public class AudioSource {
		private Entity m_Entity;

		internal AudioSource(Entity entity) {
			m_Entity = entity;
		}

		public unsafe void Play() {
			CoreAPI.API.Audio_Play(m_Entity.ID.High, m_Entity.ID.Low);
		}

		public unsafe void Stop() {
			CoreAPI.API.Audio_Stop(m_Entity.ID.High, m_Entity.ID.Low);
		}

		public unsafe float Volume {
			get => CoreAPI.API.Audio_GetVolume(m_Entity.ID.High, m_Entity.ID.Low);
			set => CoreAPI.API.Audio_SetVolume(m_Entity.ID.High, m_Entity.ID.Low, value);
		}

		public unsafe float Pitch {
			get => CoreAPI.API.Audio_GetPitch(m_Entity.ID.High, m_Entity.ID.Low);
			set => CoreAPI.API.Audio_SetPitch(m_Entity.ID.High, m_Entity.ID.Low, value);
		}

	}

}
