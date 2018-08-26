#include "system_management.h"

std::ofstream fileout("log");

System_resource::System_resource()
{
	sysinfo(&memInfo);
	totalVirtualMem = memInfo.totalram;
	totalVirtualMem += memInfo.totalswap;
	totalVirtualMem *= memInfo.mem_unit;
}
uint64_t System_resource::gettotalVirtualMem()
{
	return totalVirtualMem;
}
uint64_t System_resource::getvirtualMemUsed()
{
	virtualMemUsed = memInfo.totalram - memInfo.freeram;
	virtualMemUsed += memInfo.totalswap - memInfo.freeswap;
	virtualMemUsed *= memInfo.mem_unit;
	return virtualMemUsed;
}
System_resource system_resource;
