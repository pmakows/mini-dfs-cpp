# Mini Distributed File System (DFS)

A minimal distributed file system implemented in C++ with multiple storage nodes, a cache layer, and client-side logic for data placement and chunking.


The system is containerized using Docker and orchestrated with Docker Compose. It consists of multiple services running as separate containers: three storage nodes responsible for storing data blocks, a metadata service maintaining file-to-block mappings, and a cache service that accelerates read operations.

A client component interacts with the system by splitting files into chunks, distributing them across storage nodes, and reconstructing them on read. Small files can be cached and served directly from the cache layer, while larger files bypass the cache based on size thresholds and are fetched directly from storage nodes.

The architecture diagram below illustrates how these components interact, including data flow between the client, cache, metadata service, and storage nodes.

## Architecture

![Architecture](docs/architecture.png)

## Components

- Storage nodes store data blocks and expose a simple HTTP API (PUT /block/<id>, GET /block/<id>)
- Metadata service maintains file-to-block mapping and tracks block locations
- Cache node handles cache hit/miss, speeds up reads and supports invalidation
- Client (DFS CLI) splits files into chunks, distributes data using hashing and reconstructs files during reads

## How it works

Write flow:
client → cache → storage nodes

1. File is split into chunks
2. Each chunk is assigned to a storage node
3. Metadata is stored
4. Cache is updated

Read flow:
client → cache → storage nodes (on miss)

- Cache hit returns data immediately
- Cache miss fetches from storage and populates cache

## Run the system
```
docker compose up -d --build
```

Check services:
```
docker compose ps
```

Stop:
```
docker compose down -v
```

## Automated tests (scripts-based)
```
bash scripts/run_all.sh
```

The test suite includes cache tests, storage tests, metadata tests, split test (client CLI), end-to-end tests, invalidation tests and large file tests.

## Manuall tests (curl examples)

#### STORAGE (9001)
```
curl -X PUT localhost:9001/block/test --data-binary "hello"
curl localhost:9001/block/test
```

#### METADATA (9000)
```
curl -X POST localhost:9000/files \
  -H "Content-Type: application/json" \
  -d '{"path":"/file.txt","blocks":[{"id":"block1","nodes":["localhost:9001"],"size":5}]}'

curl localhost:9000/files/file.txt
```

#### CLIENT (CLI)
```
./build/dfs split input.txt
./build/dfs put input.txt /file.txt
./build/dfs get /file.txt output.txt
```

## CI (GitHub Actions)

On every push and pull request the system is built, Docker services are started, client CLI is built locally (for split test) and all smoke tests are executed. The workflow can also be triggered manually.

## Project evolution

M1: single storage node  
M2: HTTP API  
M3: multiple storage nodes  
M4: data distribution (hashing)  
M5: DFS client  
M6: cache layer  
M7: CI, tests and Docker orchestration  

## Design notes

- HTTP is used for simplicity and easy debugging (curl)
- Single Docker image is reused across services
- Cache is implemented as a separate component
- Tests are split into HTTP-based integration tests and CLI-based client logic tests

## Planned future work
- M8: multi-node cache (sharding across cache nodes)
- M9: cache eviction policies (LRU / TTL)
- M10: data replication in storage layer
- M11: basic fault tolerance (read fallback across replicas)
