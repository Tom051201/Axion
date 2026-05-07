using System;

namespace AxionScriptCore {

	public class Input {

		public static unsafe bool IsKeyPressed(KeyCode keyCode) {
			byte isDown = CoreAPI.API.Input_IsKeyPressed((ushort)keyCode);
			return isDown == 1;
		}

		public static unsafe bool IsMouseButtonPressed(MouseButton button) {
			byte isDown = CoreAPI.API.Input_IsMouseButtonPressed((ushort)button);
			return isDown == 1;
		}

		public static unsafe void GetMousePosition(out float x, out float y) {
			float outX, outY;
			CoreAPI.API.Input_GetMousePosition(&outX, &outY);
			x = outX;
			y = outY;
		}

		public static float GetMouseX() {
			GetMousePosition(out float x, out float _);
			return x;
		}

		public static float GetMouseY() {
			GetMousePosition(out float _, out float y);
			return y;
		}

	}

}
