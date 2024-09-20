# Micro Monolith Server
As name suggest this the aim of this application to create a server that acts as monolithic servers for micro shards. When very small operations such as grabing caches is stored on separate server, a server having primary responsibility just to pack a data and return it to client, require to make call to cache server, database, etc. This calls may constitue latency and CPU usage, having all services in memory can save a lot here also, scope of vertical scaling also increases.

## Coding Guidelines
1. All size and count type must be size_t.