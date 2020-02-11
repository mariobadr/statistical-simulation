#include "cpu/testers/packet_trace/packet_trace_gen.hh"

#include <fstream>

#include "debug/PacketTraceGen.hh"
#include "proto/packet.pb.h"
#include "sim/sim_exit.hh"
#include "sim/system.hh"

/* PacketTraceGenParams */

// This function is declared in the generated params class and must
// be defined by us.
PacketTraceGen *PacketTraceGenParams::create()
{
    return new PacketTraceGen(this);
}

/* PacketTraceGen - Public */

PacketTraceGen::PacketTraceGen(const PacketTraceGenParams *p)
    : MemObject(p), port(name() + ".port", this), masterID(0),
      stepEvent([this] { step(); }, name()), system(p->system),
      trace(p->trace), alignToCache(p->cache_align),
      maxOutstandingRequests(p->outstanding), maxPackets(p->max_packets)
{
}

BaseMasterPort &PacketTraceGen::getMasterPort(const std::string &name,
                                              const PortID idx)
{
    if (name == "port") {
        // TODO: Explain why this is needed.
        return port;
    } else {
        return MemObject::getMasterPort(name, idx);
    }
}

void PacketTraceGen::init()
{
    if (!port.isConnected()) {
        fatal("The port of %s is not connected.\n", name());
    }

    // Can't request a masterID after init(), so we do it here.
    masterID = params()->system->getMasterId(this);
}

void PacketTraceGen::initState()
{
    ProtoMessage::PacketHeader header_msg;
    if (!trace.read(header_msg)) {
        panic("Failed to read packet header from trace\n");
    } else if (header_msg.tick_freq() != SimClock::Frequency) {
        panic("Trace was recorded with a different tick frequency %d\n",
              header_msg.tick_freq());
    }
}

void PacketTraceGen::startup()
{
    scheduleNextEvent();
}

DrainState PacketTraceGen::drain()
{
    if (!stepEvent.scheduled()) {
        // no event has been scheduled yet (e.g. switched from atomic mode)
        return DrainState::Drained;
    }

    if (retryPacket == nullptr) {
        deschedule(stepEvent);

        return DrainState::Drained;
    } else {
        return DrainState::Draining;
    }
}

void PacketTraceGen::regStats()
{
    ClockedObject::regStats();

    // Initialise all the stats
    using namespace Stats;

    numRetries.name(name() + ".numRetries").desc("Number of retries");

    retryTicks.name(name() + ".retryTicks")
        .desc("Time spent waiting due to back-pressure (ticks)");

    suppressedPackets.name(name() + ".suppressedPackets")
        .desc("Number of packets synthesized but not sent.");
}

void PacketTraceGen::retry()
{
    assert(retryPacket != nullptr);

    // Update stats.
    numRetries++;

    // Attempt to send the packet again.
    const bool packet_sent = port.sendTimingReq(retryPacket);

    if (packet_sent) {
        outstandingRequests++;

        // Update stats.
        const Tick delay = curTick() - retryTick;
        retryTicks += delay;
        retryTick = 0;

        retryPacket = nullptr;

        scheduleNextEvent();
    }
}

void PacketTraceGen::step()
{
    assert(nextPacket != nullptr);

    const bool packet_sent = sendPacket(nextPacket);
    nextPacket = nullptr;

    if (packet_sent) {
        if (system->isTimingMode()) {
            outstandingRequests++;
        }

        packetsGenerated++;

        if (packetsGenerated % 1000000 == 0) {
            DPRINTF(PacketTraceGen, "%d packets have been generated.\n",
                    packetsGenerated);
        }

        scheduleNextEvent();
    }
}

bool PacketTraceGen::sendPacket(PacketPtr packet)
{
    if (params()->system->isMemAddr(packet->getAddr())) {
        if (system->isAtomicMode()) {
            port.sendAtomic(packet);
            receivePacket(packet);

            return true;
        }

        const bool packet_sent = port.sendTimingReq(packet);
        if (!packet_sent) {
            // Save this packet and try sending it again later.
            retryPacket = packet;
            retryTick = curTick();

            return false; // packet not sent
        }
    } else {
        DPRINTF(PacketTraceGen, "Suppressed %s 0x%x\n", packet->cmdString(),
                packet->getAddr());

        suppressedPackets++;

        delete packet->req;
        delete packet;
        packet = nullptr;
    }

    return true; // packet was sent/suppressed
}

void PacketTraceGen::receivePacket(PacketPtr packet)
{
    if (system->isTimingMode()) {
        outstandingRequests--;
        assert(outstandingRequests >= 0);
    }

    // Cleanup memory.
    delete packet->req;
    delete packet;
    packet = nullptr;

    // Need more requests?
    if (limitRequests() && outstandingRequests == 0) {
        scheduleNextEvent();
    }
}

void PacketTraceGen::scheduleNextEvent()
{
    assert(nextPacket == nullptr);

    if (limitRequests() && outstandingRequests == maxOutstandingRequests) {
        // Wait for responses.
        return;
    }

    if (maxPackets > 0 && packetsGenerated == maxPackets) {
        exitSimLoop(name() +
                    " has generated the maximum number of packets.\n");

        return;
    }

    ProtoMessage::Packet p;
    if (trace.read(p)) {
        const Request::FlagsType flags = p.has_flags() ? p.flags() : 0;

        const unsigned block_size = system->cacheLineSize();
        const unsigned size = std::min(p.size(), block_size);

        Request *request = new Request(p.addr(), size, flags, masterID);

        MemCmd command = p.cmd();
        nextPacket = new Packet(request, command);

        if (alignToCache) {
            if (nextPacket->getOffset(block_size) + size > block_size) {
                // The request spans two cache blocks, overwrite the address to
                // align it to the block address.
                nextPacket->setAddr(nextPacket->getBlockAddr(block_size));
            }
        }

        uint8_t *data = new uint8_t[request->getSize()];
        nextPacket->dataDynamic(data);

        if (command.isWrite()) {
            std::fill_n(data, request->getSize(), (uint8_t)masterID);
        }

        Tick nextTick = std::max(p.tick(), curTick());
        schedule(stepEvent, nextTick);
    } else {
        exitSimLoop(name() + " has generated all packets from the trace.\n");
    }
}

bool PacketTraceGen::limitRequests() const
{
    return system->isTimingMode() && maxOutstandingRequests > 0;
}

/* PacketTraceGen::Port */

PacketTraceGen::Port::Port(const std::string &name, MemObject *owner)
    : MasterPort(name, owner)
{
}

bool PacketTraceGen::Port::recvTimingResp(PacketPtr packet)
{
    auto generator = static_cast<PacketTraceGen *>(&owner);
    generator->receivePacket(packet);

    // Signal that we have received the response.
    return true;
}

void PacketTraceGen::Port::recvReqRetry()
{
    auto generator = static_cast<PacketTraceGen *>(&owner);
    generator->retry();
}

/* PacketTraceGen - Private */

const PacketTraceGenParams *PacketTraceGen::params() const
{
    return dynamic_cast<const PacketTraceGenParams *>(_params);
}
