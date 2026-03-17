:: Compile Vertex Shaders
fxc.exe /T vs_5_1 /E VSMain /Fh Batch2DQuad_VS.h /Vn g_Batch2DQuad_VS Batch2DQuad.hlsl
fxc.exe /T vs_5_1 /E VSMain /Fh Batch2DLine_VS.h /Vn g_Batch2DLine_VS Batch2DLine.hlsl
fxc.exe /T vs_5_1 /E VSMain /Fh StandardPBR_VS.h /Vn g_StandardPBR_VS StandardPBR.hlsl
fxc.exe /T vs_5_1 /E VSMain /Fh ShadowMap_VS.h /Vn g_ShadowMap_VS ShadowMap.hlsl

:: Compile Pixel Shaders
fxc.exe /T ps_5_1 /E PSMain /Fh Batch2DQuad_PS.h /Vn g_Batch2DQuad_PS Batch2DQuad.hlsl
fxc.exe /T ps_5_1 /E PSMain /Fh Batch2DLine_PS.h /Vn g_Batch2DLine_PS Batch2DLine.hlsl
fxc.exe /T ps_5_1 /E PSMain /Fh StandardPBR_PS.h /Vn g_StandardPBR_PS StandardPBR.hlsl
fxc.exe /T ps_5_1 /E PSMain /Fh ShadowMap_PS.h /Vn g_ShadowMap_PS ShadowMap.hlsl