echo This must be executed using sudo

sysctl net.core.rmem_max=26214400
sysctl net.core.wmem_max=26214400
sysctl net.ipv4.ip_unprivileged_port_start=7
#sysctl net.ipv4.ip_unprivileged_port_start=443

if [ -f ./build/test/httpserver/httpserver ]; then
echo Settng Cap on Test HTTP Server
setcap 'cap_net_bind_service=+eip' ./build/test/httpserver/httpserver
fi

if [ -f ./out/build/Release/test/httpserver/httpserver ]; then
echo Settng Cap on Release Test HTTP Server
setcap 'cap_net_bind_service=+eip' ./out/build/Release/test/httpserver/httpserver
fi

if [ -f ./out/build/Debug/test/httpserver/httpserver ]; then
echo Settng Cap on Debug Test HTTP Server
setcap 'cap_net_bind_service=+eip' ./out/build/Debug/test/httpserver/httpserver
fi

if [ -f ./build/test/echoserver/echoserver ]; then
echo Settng Cap on Test ECHO Server
setcap 'cap_net_bind_service=+eip' ./build/test/echoserver/echoserver
fi

if [ -f ./out/build/Release/test/echoserver/echoserver ]; then
echo Settng Cap on Release Test ECHO Server
setcap 'cap_net_bind_service=+eip' ./out/build/Release/test/echoserver/echoserver
fi

if [ -f ./out/build/Debug/test/echoserver/echoserver ]; then
echo Settng Cap on Debug Test ECHO Server
setcap 'cap_net_bind_service=+eip' ./out/build/Debug/test/echoserver/echoserver
fi

if [ -f ./build/app/mms ]; then
echo Settng Cap on MMS Server
setcap 'cap_net_bind_service=+eip' ./build/app/mms
fi

if [ -f ./out/build/Release/app/mms ]; then
echo Settng Cap on Release MMS Server
setcap 'cap_net_bind_service=+eip' ./out/build/Release/app/mms
fi

if [ -f ./out/build/Debug/app/mms ]; then
echo Settng Cap on Debug MMS Server
setcap 'cap_net_bind_service=+eip' ./out/build/Debug/app/mms
fi
