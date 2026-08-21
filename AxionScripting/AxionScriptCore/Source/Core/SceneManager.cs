using System;
using System.Runtime.InteropServices;

namespace AxionScriptCore {

	public class SceneManager {

		public static unsafe void LoadScene(string filepath) {
			IntPtr pathPtr = Marshal.StringToHGlobalAnsi(filepath);
			CoreAPI.API.Scene_Load(pathPtr);
			Marshal.FreeHGlobal(pathPtr);
		}

		public static unsafe void SaveScene(string filepath) {
			IntPtr pathPtr = Marshal.StringToHGlobalAnsi(filepath);
			CoreAPI.API.Scene_Save(pathPtr);
			Marshal.FreeHGlobal(pathPtr);
		}

		public static unsafe bool IsLoading {
			get { return CoreAPI.API.Scene_IsLoading() != 0; }
		}

	}

}
