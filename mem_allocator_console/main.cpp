#include "windows.h"
#include <locale.h> // LC_ALL

#include "memory_allocator.h"
#include "..\shared\mem_allocator_shared.h" // strings defines
#include "resource.h"  // resource defines

int wmain(int argc, wchar_t *argv[], wchar_t *envp[]) {

	argc; argv; envp; // to avoid warning C4100
	setlocale(LC_ALL, "");
	setvbuf(stdout, NULL, _IONBF, 0);

	if (check_windows_support::is_ok()) 
	{
		memory_allocator::MemAllocator testbed; // activate testbed
		if (testbed.is_ok(MEM_ALLOCATOR_RESOURCE, MEM_ALLOCATOR_SYS_FILE, MEM_ALLOCATOR_SERVNAME_APP, MEM_ALLOCATOR_LINKNAME_APP)) 
		{
			memory_allocator::init_input_commands();
			do {
				memory_allocator::print_supported_commands();
				switch (memory_allocator::parse_call(testbed)) {
				case memory_allocator::PARSE_RESULT::WRONG:
					std::cout << " ---wrong input, try again---" << endl;
					break;
				case memory_allocator::PARSE_RESULT::QUIT:
					cin.ignore();
					cout << "Press enter to exit." << endl;
					cin.ignore(); // std::system("PAUSE");
					return 0;
				}
				cin.clear();
				cin.ignore(10000, '\n');
			} while (true);
		}
	}
}