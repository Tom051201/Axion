:: Compile Vertex Shaders
dxc.exe -T vs_6_5 -E VSMain -Fh Batch2DQuad_VS.h -Vn g_Batch2DQuad_VS Batch2DQuad.hlsl
dxc.exe -T vs_6_5 -E VSMain -Fh Batch2DLine_VS.h -Vn g_Batch2DLine_VS Batch2DLine.hlsl
dxc.exe -T vs_6_5 -E VSMain -Fh StandardPBR_VS.h -Vn g_StandardPBR_VS StandardPBR.hlsl
dxc.exe -T vs_6_5 -E VSMain -Fh SkeletalPBR_VS.h -Vn g_SkeletalPBR_VS SkeletalPBR.hlsl
dxc.exe -T vs_6_5 -E VSMain -Fh ShadowMap_VS.h -Vn g_ShadowMap_VS ShadowMap.hlsl
dxc.exe -T vs_6_5 -E VSMain -Fh SkeletalShadowMap_VS.h -Vn g_SkeletalShadowMap_VS SkeletalShadowMap.hlsl
dxc.exe -T vs_6_5 -E VSMain -Fh Skybox_VS.h -Vn g_Skybox_VS Skybox.hlsl
dxc.exe -T vs_6_5 -E VSMain -Fh ToneMapping_VS.h -Vn g_ToneMapping_VS ToneMapping.hlsl

:: Compile Pixel Shaders
dxc.exe -T ps_6_5 -E PSMain -Fh Batch2DQuad_PS.h -Vn g_Batch2DQuad_PS Batch2DQuad.hlsl
dxc.exe -T ps_6_5 -E PSMain -Fh Batch2DLine_PS.h -Vn g_Batch2DLine_PS Batch2DLine.hlsl
dxc.exe -T ps_6_5 -E PSMain -Fh StandardPBR_PS.h -Vn g_StandardPBR_PS StandardPBR.hlsl
dxc.exe -T ps_6_5 -E PSMain -Fh SkeletalPBR_PS.h -Vn g_SkeletalPBR_PS SkeletalPBR.hlsl
dxc.exe -T ps_6_5 -E PSMain -Fh Skybox_PS.h -Vn g_Skybox_PS Skybox.hlsl
dxc.exe -T ps_6_5 -E PSMain -Fh ToneMapping_PS.h -Vn g_ToneMapping_PS ToneMapping.hlsl