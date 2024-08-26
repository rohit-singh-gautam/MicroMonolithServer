echo This must be executed using sudo

sysctl net.core.rmem_max=26214400
sysctl net.core.wmem_max=26214400
sysctl net.ipv4.ip_unprivileged_port_start=80
#sysctl net.ipv4.ip_unprivileged_port_start=443

if [ -f ./build/deviceserver/DeviceServer ]; then
echo Settng Cap on DeviceServer
setcap 'cap_net_bind_service=+eip' ./build/test/httpserver/httpserver
fi

if [ -f ./out/build/Release/deviceserver/DeviceServer ]; then
echo Settng Cap on Release DeviceServer
setcap 'cap_net_bind_service=+eip' ./out/build/Release/test/httpserver/httpserver
fi

if [ -f ./out/build/Debug/deviceserver/DeviceServer ]; then
echo Settng Cap on Debug DeviceServer
setcap 'cap_net_bind_service=+eip' ./out/build/Debug/test/httpserver/httpserver
fi

if [ -f ./build/deviceserver/DeviceServer ]; then
echo Settng Cap on DeviceServer
setcap 'cap_net_bind_service=+eip' ./build/app/mms
fi

if [ -f ./out/build/Release/deviceserver/DeviceServer ]; then
echo Settng Cap on Release DeviceServer
setcap 'cap_net_bind_service=+eip' ./out/build/Release/app/mms
fi

if [ -f ./out/build/Debug/deviceserver/DeviceServer ]; then
echo Settng Cap on Debug DeviceServer
setcap 'cap_net_bind_service=+eip' ./out/build/Debug/app/mms
fi