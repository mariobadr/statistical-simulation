/*
 * Author: Mario Badr
 */

#ifndef __CPU_PACKET_TRACE_PACKET_TRACE_GEN_HH__
#define __CPU_PACKET_TRACE_PACKET_TRACE_GEN_HH__

#include <random>

#include "mem/mem_object.hh"
#include "mem/qport.hh"
#include "params/PacketTraceGen.hh"
#include "proto/protoio.hh"

class PacketTraceGen : public MemObject
{
  public:
    /**
     * Constructor.
     */
    PacketTraceGen(const PacketTraceGenParams *);

    /**
     * Get the master port that needs to be bound to a slave.
     *
     * idx is intended for vector ports, which we do not use.
     */
    BaseMasterPort &getMasterPort(const std::string &name,
                                  PortID idx = InvalidPortID) override;

    /**
     * Initialize the PacketTraceGen. First initialization call.
     */
    void init() override;

    /**
     * Initialize any values for a "cold start" of the simulation.
     */
    void initState() override;

    /**
     * Final initialization call.
     *
     * Schedules the first event in the event loop.
     */
    void startup() override;

    /**
     * Wait for all packets to be drained.
     */
    DrainState drain() override;

    /**
     * Register statistics
     */
    void regStats() override;

  private:
    /**
     * Customizes some port functionality for Synfull.
     */
    class Port : public MasterPort
    {
      public:
        friend class PacketTraceGen;

        /**
         * Constructor.
         */
        Port(const std::string &name, MemObject *owner);

        bool recvTimingResp(PacketPtr pkt) override;

        void recvReqRetry() override;
    };

  private:
    /**
     * Convenience function for casting inherited params object.
     */
    const PacketTraceGenParams *params() const;

    /**
     * Try to send a previously dropped packet again. This should
     * not be called directly. The function is called from the
     * PacketTraceGen's port on a recvReqRetry() callback.
     */
    void retry();

    /**
     * Step to simulation to the next event.
     */
    void step();

    /**
     * Send a packet on the port.
     *
     * @return true if the port was busy, false otherwise
     */
    bool sendPacket(PacketPtr packet);

    /**
     * Handle the response of a packet.
     */
    void receivePacket(PacketPtr packet);

    /**
     * Schedule an update event based on which partition is next.
     */
    void scheduleNextEvent();

    /**
     * @return true if requests should be throttled, false otherwise.
     */
    bool limitRequests() const;

  private:
    /** The port that packets are sent/received on. */
    Port port;

    /** MasterID used in generated requests. */
    MasterID masterID;

    /** Event for scheduling updates */
    EventFunctionWrapper stepEvent;

    /** Not NULL if the port was busy when sending a packet. */
    PacketPtr retryPacket = nullptr;

    PacketPtr nextPacket = nullptr;

    System *system;

    ProtoInputStream trace;

    bool const alignToCache;

    int const maxOutstandingRequests;

    int const maxPackets;

    int packetsGenerated = 0;

    int outstandingRequests = 0;

  private:
    /** The last time the port was busy (stat helper). */
    Tick retryTick = 0;

    /** Count the number of retries. */
    Stats::Scalar numRetries;

    /** Count the time incurred from back-pressure. */
    Stats::Scalar retryTicks;

    /** Count the number of times a packet was not sent. */
    Stats::Scalar suppressedPackets;
};

#endif