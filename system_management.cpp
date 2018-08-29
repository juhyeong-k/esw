#include "system_management.h"
#include "car_lib.h"

std::ofstream fileout("log");
/**
  * @Brief
  */
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
/**
  * @Brief
  */
void Driver::view()
{
	gettimeofday(&st,NULL);
    start_get_Speed();
    get_Distance();
    get_Speed();
    gettimeofday(&et,NULL);
    optime = (double)(et.tv_sec)+(double)(et.tv_usec)/1000000.0
             -(double)(st.tv_sec)-(double)(st.tv_usec)/1000000.0;
    printf("optime :: %lf sec\n", optime);
}
void Driver::get_Distance()
{
	front = DistanceSensor(1);
	r_front = DistanceSensor(2);
	r_back = DistanceSensor(3);
	back = DistanceSensor(4);
    l_back = DistanceSensor(5);
    l_front = DistanceSensor(6);
}
void Driver::start_get_Speed()
{
    
}
void Driver::get_Speed()
{

}