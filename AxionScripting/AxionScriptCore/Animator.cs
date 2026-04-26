
namespace AxionScriptCore {

	public class Animator {
		private Entity m_Entity;

		internal Animator(Entity entity) {
			m_Entity = entity;
		}

		public unsafe void Play() {
			CoreAPI.API.Animation_Play(m_Entity.ID.High, m_Entity.ID.Low);
		}

		public unsafe void Stop() {
			CoreAPI.API.Animation_Stop(m_Entity.ID.High, m_Entity.ID.Low);
		}

		public unsafe bool IsPlaying {
			get {
				return CoreAPI.API.Animation_IsPlaying(m_Entity.ID.High, m_Entity.ID.Low) == 1;
			}
			set {
				if (value) Play();
				else Stop();
			}
		}

	}

}
