#include "allocated_mem_access.h"

extern "C" namespace allocated_memory_access 
{
	TESTBED_STR _GlobalStruct = { 0 };

	CONFIG_THREAD configThread = { 0 };

	const ULONG armor_tag = 'RMRA'; // ARMR armor

	// Initialize global buffers
	_Use_decl_annotations_ void AllocatedMemoryAccess::init() {
		_pGlobalStrPaged =
			reinterpret_cast<TESTBED_STR *>(ExAllocatePool(PagedPool, sizeof(TESTBED_STR)));
		if (_pGlobalStrPaged) {
			RtlSecureZeroMemory(_pGlobalStrPaged, sizeof(TESTBED_STR));
		}
		_pGlobalStrNonPaged =
			reinterpret_cast<TESTBED_STR *>(ExAllocatePool(NonPagedPool, sizeof(TESTBED_STR)));
		if (_pGlobalStrNonPaged) {
			RtlSecureZeroMemory(_pGlobalStrNonPaged, sizeof(TESTBED_STR));
		}
	}
		
	// Print buffers
	_Use_decl_annotations_ void AllocatedMemoryAccess::print_struct(const char* title, const TESTBED_STR * str) {
		MEM_ALLOCATOR_LOGGER("[%10s] \t at addr 0x%.16I64X, content [%X] [%s] ",
			title, 
			str, 
			str->one_byte, 
			str->buf);
	}

	_Use_decl_annotations_ NTSTATUS AllocatedMemoryAccess::basic_mem_accesses() {

		ULONG_PTR LowLimit = { 0 };
		ULONG_PTR HighLimit = { 0 };
		IoGetStackLimits(&LowLimit, &HighLimit);
		MEM_ALLOCATOR_LOGGER("Stack limits [%s] %I64X-%I64X ",
			__FUNCTION__, LowLimit, HighLimit);

		__int64 rsp_value = AsmReadRSP();
		MEM_ALLOCATOR_LOGGER("RSP value %I64X ",
			rsp_value);

		MEM_ALLOCATOR_LOGGER("[Function], addr %.16I64X ", &AllocatedMemoryAccess::basic_mem_accesses);

		char * patch_byte = (char*)PsInitialSystemProcess;
		char first_byte = *patch_byte; // = 'I';
		char twentieth_byte = *(patch_byte + 20); //*(patch_byte + 20) = 'G';
		MEM_ALLOCATOR_LOGGER("   EPROCESS %.16I64X [0] = %02X, [20] = %02X ",
				PsInitialSystemProcess, 
				first_byte, 
				twentieth_byte);

		patch_byte = (char*)0xFFFF800000000000;
		__try {
			first_byte = *patch_byte;
		}
		__except (EXCEPTION_EXECUTE_FAULT) {
			first_byte = 0;
		}

		MEM_ALLOCATOR_LOGGER("   %.16I64X [0] = %02X ",
			patch_byte,
			first_byte);

		// 1 Access to local variable on stack memory
		TESTBED_STR local_struct = { 0 };
		local_struct.one_byte = 0x11;
		memcpy(local_struct.buf, "LocalVar", sizeof(local_struct.buf));
		print_struct("Local variable", &local_struct);

		// 2 Access to global variable
		RtlSecureZeroMemory(&_GlobalStruct, sizeof(TESTBED_STR));
		_GlobalStruct.one_byte = 0x22;
		memcpy(_GlobalStruct.buf, "GlobalVar", sizeof(local_struct.buf));
		print_struct("Global variable", &_GlobalStruct);

		static TESTBED_STR stat_local_struct = { 0 };
		// 3 Access to static global variable
		RtlSecureZeroMemory(&stat_local_struct, sizeof(TESTBED_STR));
		stat_local_struct.one_byte = 0x33;
		memcpy(stat_local_struct.buf, "LocalStatic", sizeof(local_struct.buf));
		print_struct("Static local variable", &stat_local_struct);

		// 5 Allocate local variable on the heap and write to it 
		const auto local_paged_buf =
			reinterpret_cast<TESTBED_STR *>(ExAllocatePool(PagedPool, sizeof(TESTBED_STR)));
		if (local_paged_buf) {
			RtlSecureZeroMemory(local_paged_buf, sizeof(TESTBED_STR));
			local_paged_buf->one_byte = 0x55;
			memcpy(local_paged_buf->buf, "AllocLocalPaged", sizeof(local_paged_buf->buf));
			print_struct("Allocated Local Paged", local_paged_buf);
			ExFreePool(local_paged_buf);
		}

		// 5 Allocate local variable on the heap and write to it 
		const auto local_nonpaged_buf =
			reinterpret_cast<TESTBED_STR *>(ExAllocatePool(NonPagedPool, sizeof(TESTBED_STR)));
		if (local_nonpaged_buf) {
			RtlSecureZeroMemory(local_nonpaged_buf, sizeof(TESTBED_STR));
			local_nonpaged_buf->one_byte = 0x55;
			memcpy(local_nonpaged_buf->buf, "AllocLocalNonPaged", sizeof(local_nonpaged_buf->buf));
			print_struct("Allocated Local NonPaged", local_nonpaged_buf);
			ExFreePool(local_nonpaged_buf);
		}

		// 6 Access to allocated Paged-global variable
		if (_pGlobalStrPaged) {
			RtlSecureZeroMemory(_pGlobalStrPaged, sizeof(TESTBED_STR));
			_pGlobalStrPaged->one_byte = 0x66;
			memcpy(_pGlobalStrPaged->buf, "GlobPagedBuf", sizeof(local_struct.buf));
			print_struct("Allocated Global Paged", _pGlobalStrPaged);
		}

		// 7 Access to allocated NonPaged-global variable
		if (_pGlobalStrNonPaged) {
			RtlSecureZeroMemory(_pGlobalStrNonPaged, sizeof(TESTBED_STR));
			_pGlobalStrNonPaged->one_byte = 0x77;
			memcpy(_pGlobalStrNonPaged->buf, "GlobNonPagedBuf", sizeof(local_struct.buf));
			print_struct("Allocated Global NonPaged", _pGlobalStrNonPaged);
		}
		return STATUS_SUCCESS;
	}

	//////////////////////////////////////////////////////////////////////////

	void set_print_proc_info(void *buf, ULONG sz) {
		RtlSecureZeroMemory(buf, sz);
		NTSTATUS nt_status = ZwQuerySystemInformation(SystemBasicInformation, buf, sz, &sz);
		PSYSTEM_BASIC_INFORMATION psys_info = (PSYSTEM_BASIC_INFORMATION)buf;
		if (NT_SUCCESS(nt_status)) {
			MEM_ALLOCATOR_LOGGER("Buffer #2 with SystemBasicInformation ");
			MEM_ALLOCATOR_LOGGER("LowestPhysicalPage:",
				psys_info->LowestPhysicalPage);

			MEM_ALLOCATOR_LOGGER("HighestPhysicalPage:",
				psys_info->HighestPhysicalPage);

// 			while (psys_info->NextEntryOffset){
// 				psys_info = (PSYSTEM_PROCESS_INFORMATION)((char *)psys_info + psys_info->NextEntryOffset);
// 				DbgPrint(" (%wZ : %X) ", &psys_info->ImageName, psys_info->UniqueProcessId);
//			}
		}
	}

	bool set_proc_info(void *buf, ULONG sz) {
		RtlSecureZeroMemory(buf, sz);
		NTSTATUS nt_status = ZwQuerySystemInformation(SystemBasicInformation, buf, sz, &sz);
		return NT_SUCCESS(nt_status);
	}

	void print_proc_info(void *buf, ULONG sz) {
		if (buf){
			PSYSTEM_BASIC_INFORMATION psys_info = (PSYSTEM_BASIC_INFORMATION)buf;
			MEM_ALLOCATOR_LOGGER("Buffer #2 with SystemBasicInformation %I64X - %I64X", buf, (char*)buf + sz );
			MEM_ALLOCATOR_LOGGER("LowestPhysicalPage:", psys_info->LowestPhysicalPage);
			MEM_ALLOCATOR_LOGGER("HighestPhysicalPage:", psys_info->HighestPhysicalPage);
// 			while (psys_info->NextEntryOffset){
// 				psys_info = (PSYSTEM_PROCESS_INFORMATION)((char *)psys_info + psys_info->NextEntryOffset);
// 				DbgPrint(" (%wZ : %X) ", &psys_info->ImageName, psys_info->UniqueProcessId);
//			}
		}
	}

	VOID memory_access_loop(_In_ PVOID StartContext) {
		configThread.flagLoopIsActive = true;
		REACTOR_CONFIG* p_data = (REACTOR_CONFIG*)StartContext;
		LARGE_INTEGER timeout = { 0 };
		timeout.QuadPart = (LONGLONG)(-1000 * 1000 * 10 * 3);  //  3s
		ULONG64 data = 0;
		while (configThread.flagLoopIsActive){

			__try {
				KeWaitForMutexObject(&configThread.mutex, Executive, KernelMode, FALSE, NULL);
				data = p_data->tempReactor;
				KeReleaseMutex(&configThread.mutex, FALSE);
			}
			__except (EXCEPTION_EXECUTE_FAULT) {
				data = 0;
			}
			MEM_ALLOCATOR_LOGGER("Buffer #1 %I64X - %I64X ", p_data, (char*)p_data + sizeof(REACTOR_CONFIG) );
			MEM_ALLOCATOR_LOGGER("Temp is   %X   located %I64X ", data, &p_data->tempReactor );
			print_proc_info(p_data->buf_proc_info, p_data->buf_proc_info_sz);
			KeDelayExecutionThread(KernelMode, FALSE, &timeout);
		}
	}

	NTSTATUS AllocatedMemoryAccess::set_memory_thread(void* buf, void *outbuf) {
		REACTOR_CONFIG* p_data = (REACTOR_CONFIG*)configThread.pconfig_data;

		KeWaitForMutexObject(&configThread.mutex, Executive, KernelMode, FALSE, NULL);
		p_data->tempReactor = *(ULONG64*)buf;
		KeReleaseMutex(&configThread.mutex, FALSE);

		*((ULONG64*)outbuf) = ((ULONG64)&p_data->tempReactor);

		return STATUS_SUCCESS;
	}

	NTSTATUS AllocatedMemoryAccess::start_set_thread(void* inbuf, void *outbuf){
		if (configThread.flagLoopIsActive){
			return 
				set_memory_thread(inbuf, outbuf);
		}
		else {
			KeInitializeMutex(&configThread.mutex, 0);
			REACTOR_CONFIG *pinit_data = (REACTOR_CONFIG*)ExAllocatePoolWithTag(
				NonPagedPool, sizeof REACTOR_CONFIG, armor_tag);
			RtlSecureZeroMemory(pinit_data, sizeof REACTOR_CONFIG);
			pinit_data->tempReactor = *(ULONG64*)inbuf; // update value in the kernel memory
			*((ULONG64*)outbuf) = (ULONG64)&pinit_data->tempReactor; // return updated value to the console

			pinit_data->buf_proc_info_sz = 0;
			ZwQuerySystemInformation(SystemBasicInformation, NULL, 0, &pinit_data->buf_proc_info_sz);
			pinit_data->buf_proc_info = ExAllocatePoolWithTag(NonPagedPool, pinit_data->buf_proc_info_sz, armor_tag);
			if (pinit_data->buf_proc_info) {
				set_proc_info(pinit_data->buf_proc_info, pinit_data->buf_proc_info_sz);
			}

			configThread.pconfig_data = pinit_data;
			NTSTATUS nt_status =
				PsCreateSystemThread(
					&configThread.handleMemoryLoop,
					THREAD_ALL_ACCESS, NULL, NULL, NULL,
					memory_access_loop, pinit_data);

			if (!NT_SUCCESS(nt_status)) {
				MEM_ALLOCATOR_LOGGER(" PsCreateSystemThread error %08X ", nt_status);
			}
			return nt_status;
		}
	}

	NTSTATUS AllocatedMemoryAccess::stop_this_thread() {
		NTSTATUS nt_status = STATUS_SUCCESS;
		if (configThread.flagLoopIsActive){
			configThread.flagLoopIsActive = false;
			nt_status = ObReferenceObjectByHandle(configThread.handleMemoryLoop,
				THREAD_ALL_ACCESS, NULL, KernelMode, (PVOID*)&configThread.pthread, NULL);
			if (NT_SUCCESS(nt_status)) {
				if (STATUS_SUCCESS == (nt_status = KeWaitForSingleObject(configThread.pthread,
					Executive, KernelMode, FALSE, NULL))) {
					ObDereferenceObject(configThread.pthread);
					if (configThread.pconfig_data) {
						ExFreePool(configThread.pconfig_data);
					}
				}
			}
		}
		return nt_status;
	}
}