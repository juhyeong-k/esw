#include "system_management.h"

std::ofstream fileout("log");

System_resource::System_resource()
{
	sysinfo(&memInfo);
	totalVirtualMem = memInfo.totalram;
	totalVirtualMem += memInfo.totalswap;
	totalVirtualMem *= memInfo.mem_unit;

	totalPhysMem = memInfo.totalram;
	totalPhysMem *= memInfo.mem_unit;
}
uint64_t System_resource::getTotalVirtualMem()
{
	return totalVirtualMem;
}
uint64_t System_resource::getVirtualMemUsed()
{
	virtualMemUsed = memInfo.totalram - memInfo.freeram;
	virtualMemUsed += memInfo.totalswap - memInfo.freeswap;
	virtualMemUsed *= memInfo.mem_unit;
	return virtualMemUsed;
}
uint64_t System_resource::getTotalPhysMem()
{
	return totalPhysMem;
}
uint64_t System_resource::getPhysMemUsed()
{
	physMemUsed = memInfo.totalram - memInfo.freeram;
	physMemUsed *= memInfo.mem_unit;
	return physMemUsed;
}
System_resource system_resource;
