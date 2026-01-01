# MemoryNode & NodeList Architecture

## Overview

The **MemoryNode** and **NodeList** subsystem implements a deterministic, fixed‑size block allocator tailored for embedded systems. It provides predictable memory usage, constant‑time allocation, and explicit ownership semantics — all without relying on the standard heap.

This system is ideal for packet buffers, protocol workspaces, DMA descriptors, and any short‑lived structures that must be allocated and freed frequently without fragmentation.

---

## 1. NodeList — Intrusive List of Fixed‑Size Nodes

### Purpose

`NodeList` manages a doubly‑linked list of nodes allocated from a custom memory pool.  
Each node is a **single contiguous block** containing:

```
[ NodeList_t header ][ user data payload ]
```

### Responsibilities

- Maintain a doubly‑linked list of nodes  
- Allocate and free nodes from the memory pool  
- Provide access to the user data portion  
- Support **index‑based sequential scanning**  
- Guarantee deterministic behavior (no hidden allocations)

### Initialization

Because `NodeList` is allocated from a raw memory pool, **its constructor is never called**.  
Therefore, it must be explicitly initialized:

```cpp
void NodeList::Initialize(size_t NodeDataSize);
```

This sets:

- `m_pFirstNode`  
- `m_pLastNode`  
- `m_pScanNode`  
- `m_NumberOfNode`  
- `m_NodeSize = sizeof(NodeList_t) + NodeDataSize`

### Key Operations

| Function | Description |
|---------|-------------|
| `AddNode(id, &dataPtr)` | Allocates a new node and links it into the list |
| `RemoveNode(id)` | Unlinks and frees a specific node |
| `RemoveAllNode()` | Frees the entire list deterministically |
| `GetNodeDataPointer(id)` | Returns the payload pointer for a given node (legacy) |
| `GetNodeByIndex(index)` | Returns the node at a given list position |
| `ResetScanNode()` | Resets the internal iterator |
| `GetNextNode()` | Returns the next node’s data and advances the iterator |

### Memory Layout Guarantee

```
m_NodeSize = sizeof(NodeList_t) + userPayloadSize
```

This ensures:

- no fragmentation  
- constant‑time allocation  
- predictable memory usage  

---

## 2. MemoryNode — High‑Level Manager for a Group of Nodes

### Purpose

`MemoryNode` wraps a `NodeList` and provides a higher‑level abstraction for allocating a *set* of nodes large enough to hold a requested amount of data.

### Responsibilities

- Own a `NodeList` instance  
- Allocate enough nodes to satisfy a requested total size  
- Free all nodes and the NodeList itself  
- Provide sequential access to node payloads  
- Ensure alloc/free symmetry  
- Use **index‑based traversal** for deterministic iteration

### Lifecycle

A `MemoryNode` is created using:

```cpp
MemoryNode* n = MemoryNode::AllocNode(totalSize, nodeDataSize);
```

And destroyed using:

```cpp
MemoryNode::FreeNode(n);
```

This guarantees:

- all nodes are freed  
- the NodeList object is freed  
- the MemoryNode object is freed  
- no memory pool tags leak  

### Key Operations

| Function | Description |
|---------|-------------|
| `Create(nodeDataSize)` | Allocates and initializes a NodeList |
| `Alloc(totalSize)` | Grows/shrinks the list to fit the requested size |
| `Begin()` | Resets internal index to 0 |
| `GetNext()` | Returns the next node’s data pointer (index‑based) |
| `Free()` | Frees all nodes and the NodeList |
| `AllocNode(size, nodeDataSize)` | Factory: allocates MemoryNode + NodeList |
| `FreeNode(ptr)` | Frees MemoryNode + NodeList |

---

## 3. Why This System Exists

Traditional dynamic allocation (`malloc/free`) is unsuitable for many embedded systems due to:

- fragmentation  
- unpredictable timing  
- hidden allocations  
- lack of deterministic behavior  

This system avoids all of that:

### ✔ No fragmentation  
All nodes are the same size.

### ✔ Deterministic timing  
Allocation and free are constant‑time.

### ✔ Explicit ownership  
The caller always knows when memory is allocated and freed.

### ✔ Safe for ISRs and real‑time tasks  
No heap, no locks, no surprises.

### ✔ Perfect for packet‑based systems  
Ideal for Ethernet drivers, IP stacks, DMA buffers, etc.

---

## 4. Typical Usage Pattern

```cpp
MemoryNode* packet = MemoryNode::AllocNode(totalSize, perNodeSize);

packet->Begin();
void* p = packet->GetNext();

while (p != nullptr)
{
    // Fill node data
    p = packet->GetNext();
}

MemoryNode::FreeNode(packet);
```

---

## 5. Summary

The MemoryNode/NodeList subsystem provides:

- deterministic memory allocation  
- zero fragmentation  
- explicit lifecycle control  
- predictable behavior under load  
- clean integration with custom memory pools  

It is a robust foundation for any embedded networking or protocol subsystem requiring fast, reliable, and repeatable memory management.

---

## Mermaid Diagram — Architecture Overview

```mermaid
flowchart TD

    MP[MemoryPool - fixed-size block allocator]
    MN[MemoryNode - manages a group of nodes]
    NL[NodeList - intrusive doubly-linked list]
    NODE[NodeList_t + payload (contiguous block)]

    MP --> MN
    MN --> NL
    NL --> NODE

    subgraph MemoryNode_Internals
        MNCreate[Create]
        MNAlloc[Alloc totalSize]
        MNBegin[Begin]
        MNGetNext[GetNext (index-based)]
        MNFree[Free]
    end

    subgraph NodeList_Internals
        NLInit[Initialize]
        NLAdd[AddNode]
        NLRemove[RemoveNode]
        NLRemoveAll[RemoveAllNode]
        NLIndex[GetNodeByIndex]
    end

    MN --> MNCreate
    MN --> MNAlloc
    MN --> MNBegin
    MN --> MNGetNext
    MN --> MNFree

    NL --> NLInit
    NL --> NLAdd
    NL --> NLRemove
    NL --> NLRemoveAll
    NL --> NLIndex

    NLAdd --> NODE
    NLIndex --> NODE
    NLRemove --> NODE
    NLRemoveAll --> NODE
```