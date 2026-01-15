# Embedded IP Stack Architecture - Module & Function Mapping

This document provides a complete overview of the architecture, modules, and function placement for the embedded zero-copy, DMA-driven IP stack with socket support.

---

# 1. NetworkContext (Orchestration Layer)

| Function | Location | Responsibility | Notes |
|---------|----------|----------------|-------|
| `SetSendCallback()` | NetworkContext | Registers driver TX callback | Called at init |
| `SendPacket(IP_PacketMsg_t*)` | NetworkContext | Entry point for all TX packets | Calls driver callback |
| `GetMsgQ()` | NetworkContext | Returns RX queue | Used by IP_Manager |
| `GetActiveIP()` | NetworkContext | Returns local IP | Used by ARP, ICMP, UDP, TCP |
| `GetMAC_Address()` | NetworkContext | Returns local MAC | Used by ARP, IP header build |
| `GetActiveSubnetMask()` | NetworkContext | Subnet mask | Used by ARP learning |

---

# 2. IP_Manager (Layer-3 Dispatcher)

| Function | Location | Responsibility | Notes |
|---------|----------|----------------|-------|
| `Run()` | IP_Manager | Main RX loop | Never frees IP packets |
| `ProcessIP(pMsg)` | IP_Manager | Dispatch to ICMP/UDP/TCP/RAW | Ownership transferred |
| `Initialize()` | IP_Manager | Init ARP, DHCP, SNTP | - |

---

# 3. ARP Module

| Function | Location | Responsibility | Notes |
|---------|----------|----------------|-------|
| `Initialize()` | NetARP | Clear table, start timer | - |
| `ProcessIP(pRX)` | NetARP | Learn MAC from IP traffic | Does NOT free pRX |
| `ProcessARP(pRX)` | NetARP | Handle ARP request/reply | Always frees pRX |
| `UpdateEntry(ip, mac)` | NetARP | Update ARP cache | - |

---

# 4. ICMP Module

| Function | Location | Responsibility | Notes |
|---------|----------|----------------|-------|
| `Process(pRX)` | NetICMP | Handle Echo Request, send reply | Frees pRX, sends pTX |
| `Initialize()` | NetICMP | (empty) | - |

---

# 5. UDP Module

## Protocol Engine (NetUDP)

| Function | Location | Responsibility | Notes |
|---------|----------|----------------|-------|
| `UDP_Input(pMsg)` | NetUDP | Deliver packet to socket | Frees pMsg if no socket |
| `BuildUDPHeader()` | NetUDP | Fill UDP header | Called by UDP_Send |

## Socket Layer

| Function | Location | Responsibility | Notes |
|---------|----------|----------------|-------|
| `Socket::SendTo()` | Socket | User API | Calls `UDP_Send()` |
| `UDP_Send(pUdp, data, len, dest, sent)` | SocketManager | Build full packet + SendPacket | Allocates pMsg + pPacket |
| `Socket::RecvFrom()` | Socket | User API | Frees pMsg after copy |

## Socket Manager

| Function | Location | Responsibility | Notes |
|---------|----------|----------------|-------|
| `FindUDPSocketByPort(port)` | SocketManager | Lookup UDP socket | Used by UDP_Input |

---

# 6. TCP Module

## Protocol Engine (NetTCP)

| Function | Location | Responsibility | Notes |
|---------|----------|----------------|-------|
| `TCP_Input(pMsg)` | NetTCP | Dispatch to connection or listener | Frees pMsg if no match |
| `TCP_ProcessSegment(conn, pMsg)` | NetTCP | State machine | Frees pMsg or queues it |
| `TCP_Send(conn, data, len, sent)` | SocketManager | Build TCP segment | Calls SendPacket |

## Socket Layer

| Function | Location | Responsibility | Notes |
|---------|----------|----------------|-------|
| `Socket::Send()` | Socket | User API | Calls TCP_Send |
| `Socket::Recv()` | Socket | User API | Frees pMsg after copy |

## Socket Manager

| Function | Location | Responsibility | Notes |
|---------|----------|----------------|-------|
| `FindTCPConnection()` | SocketManager | Match 4‑tuple | - |
| `FindTCPListener()` | SocketManager | Match listening port | - |

---

# 7. RAW Module

| Function | Location | Responsibility | Notes |
|---------|----------|----------------|-------|
| `RAW_Input(pMsg)` | NetRAW | Deliver to RAW socket | Frees pMsg if no socket |
| `FindRAWByProtocol(proto)` | SocketManager | Lookup RAW socket | - |

---

# 8. Ethernet Driver (DMA Descriptors)

| Function | Location | Responsibility | Notes |
|---------|----------|----------------|-------|
| `LowLevelOutput(IP_PacketMsg_t**)` | ETH_IF_Driver | Put packet in TX descriptor | Returns SYS_READY or SYS_BUSY |
| `FlushCompletedTxDescriptors()` | ETH_IF_Driver | Free completed TX packets | Called inside LowLevelOutput |
| `TxIRQ_Handler()` | ETH_IF_Driver | Free pMsg->pPacket + pMsg | - |
| `RxIRQ_Handler()` | ETH_IF_Driver | Allocate pMsg + pPacket, push to RX queue | - |

---

# 9. Memory Pool

| Function | Location | Responsibility |
|---------|----------|----------------|
| `AllocAndClear(size, tag)` | MemoryPool | Allocate zeroed block |
| `Free(ptr)` | MemoryPool | Free block |

Used everywhere.

---

# 10. Global Socket Table

| Symbol | Location | Responsibility |
|--------|----------|----------------|
| `g_SocketTable[SOCKET_MAX_COUNT]` | SocketManager | Stores all sockets |

---

# Summary

This architecture provides:

- clean separation of concerns  
- zero-copy TX/RX  
- DMA descriptor-driven pipeline  
- deterministic socket lookup  
- protocol engines independent of sockets  
- NetworkContext as the central orchestrator  

This is a professional-grade embedded IP stack layout.