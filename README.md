# Windows10X64-1803-POC
This is a poc to trigger a bug on windows 10 x64 1803 from win32kbase by kaspersky
## Getting Started

### Requirement

	Install Requirement:

	1) win10x64 1803 full update
	
	2) Any Application such as Kaspersky (kis19.0.0.1088aen_14170.exe) that set W32kEtwEnabledKeyword & 0x400 to 1

### Before execute poc, kaspersky must be running and after that show BSOD
	
	system service exception 

	what failed:win32kbase.dll	

### Analysis
	
	1)Registers
		rdx is my address that passed by NtUserSetWindowsHookEx
		1: kd> .cxr 0xffff8785cd67ef60

		rax=0000000000000000 rbx=0000000000000000 rcx=0000000000000000
		rdx=000000000000ffff rsi=000000000000ffff rdi=0000000000000000
		rip=fffff9992ae4f807 rsp=ffff8785cd67f950 rbp=ffff8785cd67fb00
		 r8=0000000000000000  r9=0000000000000000 r10=0000000000000000
		r11=0000000000000400 r12=0000000000000000 r13=0000000000000000
		r14=0000000000000000 r15=0000000000000000
		iopl=0         nv up ei pl nz na po nc
		cs=0010  ss=0018  ds=002b  es=002b  fs=0053  gs=002b             efl=00010206
		win32kbase!EtwTraceAuditApiSetWindowsHookEx+0x37187:
		fffff999`2ae4f807 488b4a08        mov     rcx,qword ptr [rdx+8] ds:002b:00000000`00010007=???????????????? ;controlable address


	2)Call Stack

		0: kd> kv
		 # Child-SP          RetAddr           : Args to Child                                                           : Call Site
		00 ffff8785`cb19f950 fffff999`2aa4dfc2 : 00000000`00000000 00000000`0000ffff 00000000`00000000 fffff800`00000000 : win32kbase!EtwTraceAuditApiSetWindowsHookEx+0x37187
		01 ffff8785`cb19f9a0 fffff800`b49b2f13 : 00000000`00000001 00000000`00000000 ffffad84`7f16b080 00000000`00000001 : win32kfull!NtUserSetWindowsHookEx+0xe2
		02 ffff8785`cb19fa10 00007ffe`815e21a4 : 00007ff7`0f5020f5 00000000`00000000 00007ffe`815e2190 0000006b`0e1ff800 : nt!KiSystemServiceCopyEnd+0x13 (TrapFrame @ ffff8785`cb19fa80)
		03 0000006b`0e1ff7c8 00007ff7`0f5020f5 : 00000000`00000000 00007ffe`815e2190 0000006b`0e1ff800 00000000`00000000 : win32u!NtUserSetWindowsHookEx+0x14
		04 0000006b`0e1ff7d0 00007ffe`82183034 : 00000000`00000000 00000000`00000000 00000000`00000000 00000000`00000000 : DangerOfUserModeCallback!call_win32k+0x85 [e:\project\0day\kasper\dangerofusermodecallback\dangerofusermodecallback\dangerofusermodecallback\source.cpp @ 31] 
		05 0000006b`0e1ff940 00007ffe`846b1551 : 00000000`00000000 00000000`00000000 00000000`00000000 00000000`00000000 : KERNEL32!BaseThreadInitThunk+0x14
		06 0000006b`0e1ff970 00000000`00000000 : 00000000`00000000 00000000`00000000 00000000`00000000 00000000`00000000 : ntdll!RtlUserThreadStart+0x21

	3)On the EtwTraceAuditApiSetWindowsHookEx, W32kEtwEnabledKeyword & 0x400 must be return 1 to call win32kbase!McTemplateK0qzppq 

		char __fastcall EtwTraceAuditApiSetWindowsHookEx(char a1, __int64 a2, int a3)
		{
		  char v3; // r10
		  char result; // al
		  __int64 v5; // rcx

		  v3 = a1;
		  if ( W32kEtwEnabledKeyword & 0x400 )
		  {
		    result = byte_1C01AE1D8 - 1;
		    if ( (unsigned __int8)(byte_1C01AE1D8 - 1) > 2u )
		    {
		      if ( qword_1C01AE1C0 & 0x400 )
		      {
		        result = 0;
		        if ( (qword_1C01AE1C8 & 0x400) == qword_1C01AE1C8 )
		        {
		          v5 = 0i64;
		          if ( a2 )
		            v5 = *(_QWORD *)(a2 + 8);
		          result = BYTE4(Microsoft_Windows_Win32kEnableBits);
		          if ( Microsoft_Windows_Win32kEnableBits & 0x800000000i64 )
		            result = McTemplateK0qzppq(v5, a2, a3, v3, v5, a3);
		        }
		      }
		    }
		  }
		  return result;
		}

	4)After call the win32kbase!McTemplateK0qzppq methode the win32kbase!McGenEventWriteKM methode called that it is a wrapper for nt!EtwWrite

		__int64 __fastcall EtwWrite(unsigned __int64 RegHandle,
			 _EVENT_DESCRIPTOR *EventDescriptor, 
			 _GUID *ActivityId, 
			 unsigned int UserDataCount, 
			 _EVENT_DATA_DESCRIPTOR *UserData
		)


	5)EtwWrite passed my address on UserData and ... to MEMCPY

		# Child-SP          RetAddr           : Args to Child                                                           : Call Site
		00 ffff8785`cae37388 fffff800`b48640fc : 00000000`00000000 00000000`00000000 fffff9bc`40a21000 ffff8785`cae374e0 : nt!memcpy+0x1e4
		01 ffff8785`cae37390 fffff800`b4863a66 : 00000000`0000ffff 00000000`00000000 00000000`00000000 fffff800`00000000 : nt!EtwpEventWriteFull+0x5cc
		02 ffff8785`cae376f0 fffff800`b48638cc : 00000000`00000000 00000000`00000000 00000000`00000000 00000000`00000000 : nt!EtwWriteEx+0x186
		03 ffff8785`cae37800 fffff999`2ae60bed : 00000000`00000000 00000000`00000000 00000000`00000000 ffff8785`cae37909 : nt!EtwWrite+0x2c
		04 ffff8785`cae37850 fffff999`2ae63827 : 00000000`00000000 00000000`00000000 00000000`00000000 00000000`00000000 : win32kbase!McGenEventWriteKM+0x7d
		05 ffff8785`cae378a0 fffff999`2ae4f838 : 00000000`00000003 fffff999`00000000 00000000`00000000 00000000`00000000 : win32kbase!McTemplateK0qzppq+0xc3
		06 ffff8785`cae37950 fffff999`2aa4dfc2 : 00000000`00000000 00000000`0000ffff 00000000`00000000 fffff800`00000000 : win32kbase!EtwTraceAuditApiSetWindowsHookEx+0x371b8
		07 ffff8785`cae379a0 fffff800`b49b2f13 : 00000000`00000001 00000000`00000000 ffffad84`8027e080 fffff800`00000001 : win32kfull!NtUserSetWindowsHookEx+0xe2
		08 ffff8785`cae37a10 00007ff6`5a4f20a9 : 00007ff6`5a4f10a7 0e18c002`48020bf4 00000000`00000000 00000000`00000000 : nt!KiSystemServiceCopyEnd+0x13 (TrapFrame @ ffff8785`cae37a80)
		09 000000b6`6c1ffb48 00007ff6`5a4f10a7 : 0e18c002`48020bf4 00000000`00000000 00000000`00000000 00000000`00000000 : tokernel+0x20a9
		0a 000000b6`6c1ffb50 00007ffe`82183034 : 00000000`0017ade6 00000000`00000000 00000000`00000000 00000000`00000000 : tokernel+0x10a7
		0b 000000b6`6c1ffc30 00007ffe`846b1551 : 00000000`00000000 00000000`00000000 00000000`00000000 00000000`00000000 : KERNEL32!BaseThreadInitThunk+0x14
		0c 000000b6`6c1ffc60 00000000`00000000 : 00000000`00000000 00000000`00000000 00000000`00000000 00000000`00000000 : ntdll!RtlUserThreadStart+0x21

# ANY ONE TO EXPLOIT THIS POC?