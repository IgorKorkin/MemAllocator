#include "memory_allocator.h"

namespace memory_allocator {

	//////////////////////////////////////////////////////////////////////////

	bool MemAllocator::basic_mem_access() {

		// Prepare buffers

		return scm_manager.send_ctrl_code(MEM_ALLOCATOR_BASIC_MEM_ACCESS, NULL, 0, NULL, 0, 0);
	}

	//////////////////////////////////////////////////////////////////////////

	/*  */
	bool MemAllocator::start_set_thread() {
		ULONG64 new_temp = 0;
		std::cin >> std::hex >> new_temp;

		ULONG64 addr_temp_in_krnl = 0;
		bool b_res = 
			scm_manager.send_ctrl_code(MEM_ALLOCATOR_START_SET_THREAD, (LPVOID)&new_temp, sizeof ULONG64, &addr_temp_in_krnl, sizeof ULONG64, 0);

		print_messages :: print_mes(TEXT("\t Temp  0x%I64X  is located here 0x%I64X "), new_temp, addr_temp_in_krnl);

		return b_res;
	}

	/*  */
	bool MemAllocator::stop_this_thread() {
		return
			scm_manager.send_ctrl_code(MEM_ALLOCATOR_STOP_THIS_THREAD, NULL, 0, NULL, 0, 0);
	}

	//////////////////////////////////////////////////////////////////////////

	struct command_pair_triple {
		std::string key_definition;
		TControlFunc key_function;
	};

	map <std::string, command_pair_triple> g_CommandsList;

	void add_unique_command(const std::string keyName, const TControlFunc keyFunction, const std::string def) {
		for (const auto & item : g_CommandsList) {
			if ((keyFunction && (keyFunction == item.second.key_function)) ||
				(keyName == item.first)) {
				std::cout << "Internal error: two keys have the same func" << endl;
				exit(-1);
			}
		}
		g_CommandsList.insert({ keyName,{ def, keyFunction, } });
	}

	void init_input_commands() {
		/*
		'_starttemp <temp in hex>' -- start and set temp loop
		'_stoptemp' -- stop loop
		'basic' -- run basic memory accesses
		'exit' -- exit this app
		'q' -- fast quit
		*/
		add_unique_command("basic",
			&memory_allocator::MemAllocator::basic_mem_access, "' -- run basic memory accesses ");
		add_unique_command("_starttemp",
			&memory_allocator::MemAllocator::start_set_thread, " <temp in hex>' -- start and set temp loop ");
		add_unique_command("_stoptemp",
			&memory_allocator::MemAllocator::stop_this_thread, "' -- stop loop");
		add_unique_command("exit", NULL, "' -- exit this app ");
		add_unique_command("q", NULL, "' -- fast quit");
	}

	void print_supported_commands() {
		std::cout << endl;
		std::cout << "MemAllocator allocates the sensitive data in the kernel mode >>" << endl;
		for (const auto & item : g_CommandsList) {
			cout << " '" << item.first << item.second.key_definition << endl;
		}
	}

	PARSE_RESULT parse_call(memory_allocator::MemAllocator & my_testbed) {
		string string_command = { 0 };
		string_command = { 0 };
		std::cin >> string_command; //std::getline(std::cin >> std::ws, string_command);

		const auto item = g_CommandsList.find(string_command);

		if (item != g_CommandsList.end()) {
			if (NULL == item->second.key_function) {
				return PARSE_RESULT::QUIT; // quit 
			}
			else {
				(my_testbed.*item->second.key_function)();
				return PARSE_RESULT::OK; // success
			}
		}
		return PARSE_RESULT::WRONG; // wrong input command
	}

} // namespace testbed 