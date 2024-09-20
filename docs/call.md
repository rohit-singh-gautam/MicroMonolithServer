# Server Call interface

Service call interface is divided in three layers
1. Network Protocol, e.g. HTTP, HTTPS, etc.
2. API Protocol, e.g. REST, SOAP
3. Service - This is the place where all the services are implemented.

API protocol can be dependent on network protocols. For example REST is always under HTTP. Hence REST will be receiving ProcessRead from HTTP

Normally there can be mapping from API Protocol and actual APIs, below are example:
1. Add Request - Add Entry
2. Delete Request - Delete Entry

If no mapping provided source API name will be directly provided to Service interface.



## REST
As this service may have overloaded port to support multiple types of interface, based on different calls can be made. First level of mapping is made throught path for example ```/cache``` can be mapped to REST interface, there can be multiple REST interface mapped to different services. Each REST interface will call funtions of services implementer based on the function calls. Function calls can be hierarchical in our case.
All interface can be configured from configurations.