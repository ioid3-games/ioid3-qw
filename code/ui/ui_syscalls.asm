code

equ memset									-1
equ memcpy									-2
equ strncpy									-3
equ sin										-4
equ cos										-5
equ atan2									-6
equ sqrt									-7
equ floor									-8
equ ceil									-9

equ trap_Print								-21
equ trap_Error								-22
equ trap_Milliseconds						-23
equ trap_RealTime							-24
equ trap_Argc								-25
equ trap_Argv								-26
equ trap_Cvar_Reset							-27
equ trap_Cvar_Create						-28
equ trap_Cmd_ExecuteText					-29
equ trap_Cvar_Register						-30
equ trap_Cvar_Update						-31
equ trap_Cvar_Set							-32
equ trap_Cvar_SetValue						-33
equ trap_Cvar_VariableValue					-34
equ trap_Cvar_InfoStringBuffer				-35
equ trap_Cvar_VariableStringBuffer			-36
equ trap_FS_FOpenFile						-37
equ trap_FS_Read							-38
equ trap_FS_Write							-39
equ trap_FS_Seek							-40
equ trap_FS_FCloseFile						-41
equ trap_FS_GetFileList						-42
equ trap_PC_AddGlobalDefine					-43
equ trap_PC_LoadSource						-44
equ trap_PC_FreeSource						-45
equ trap_PC_ReadToken						-46
equ trap_PC_SourceFileAndLine				-47

equ trap_GetGlconfig						-101
equ trap_MemoryRemaining					-102
equ trap_UpdateScreen						-103

equ trap_GetClientState						-151
equ trap_GetConfigString					-152

equ trap_LAN_GetServerCount					-201
equ trap_LAN_GetServerAddressString			-202
equ trap_LAN_GetServerInfo					-203
equ trap_LAN_MarkServerVisible				-204
equ trap_LAN_UpdateVisiblePings				-205
equ trap_LAN_ResetPings						-206
equ trap_LAN_LoadCachedServers				-207
equ trap_LAN_SaveCachedServers				-208
equ trap_LAN_AddServer						-209
equ trap_LAN_RemoveServer					-210
equ trap_LAN_ServerStatus					-211
equ trap_LAN_GetServerPing					-212
equ trap_LAN_ServerIsVisible				-213
equ trap_LAN_CompareServers					-214
equ trap_LAN_GetPing						-215
equ trap_LAN_GetPingInfo					-216
equ trap_LAN_GetPingQueueCount				-217
equ trap_LAN_ClearPing						-218

equ trap_R_RegisterModel					-301
equ trap_R_RegisterSkin						-302
equ trap_R_RegisterShaderNoMip				-303
equ trap_R_RegisterFont						-304
equ trap_R_ClearScene						-305
equ trap_R_AddRefEntityToScene				-306
equ trap_R_AddPolyToScene					-307
equ trap_R_AddLightToScene					-308
equ trap_R_RenderScene						-309
equ trap_R_SetColor							-310
equ trap_R_DrawStretchPic					-311
equ trap_CM_LerpTag							-312
equ trap_R_ModelBounds						-313
equ trap_R_RemapShader						-314
equ trap_CM_LoadModel						-315

equ trap_S_RegisterSound					-401
equ trap_S_StartLocalSound					-402
equ trap_S_StartBackgroundTrack				-403
equ trap_S_StopBackgroundTrack				-404

equ trap_Key_KeynumToStringBuf				-501
equ trap_Key_GetBindingBuf					-502
equ trap_Key_SetBinding						-503
equ trap_Key_IsDown							-504
equ trap_Key_GetOverstrikeMode				-505
equ trap_Key_SetOverstrikeMode				-506
equ trap_Key_ClearStates					-507
equ trap_Key_GetCatcher						-508
equ trap_Key_SetCatcher						-509
equ trap_GetClipboardData					-510

equ trap_CIN_PlayCinematic					-601
equ trap_CIN_StopCinematic					-602
equ trap_CIN_RunCinematic					-603
equ trap_CIN_DrawCinematic					-604
equ trap_CIN_SetExtents						-605
equ trap_GetCDKey							-606
equ trap_SetCDKey							-607
equ trap_VerifyCDKey						-608
equ trap_SetPbClStatus						-609
