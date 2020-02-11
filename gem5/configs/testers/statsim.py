import m5

# import all simulation objects from gem5
from m5.objects import *

# add the root level configs directory to our path
from m5.util import addToPath
addToPath('../')

# for configuring the memory system
from common import MemConfig

class L1Cache(Cache):
    tag_latency = 2
    data_latency = 2
    response_latency = 2
    mshrs = 4
    tgts_per_mshr = 20

    size = '16kB'
    assoc = 2


class L2Cache(Cache):
    tag_latency = 20
    data_latency = 20
    response_latency = 20
    mshrs = 20
    tgts_per_mshr = 12

    size = '256kB'
    assoc = 8


def create_generator(generator, input_file, limit, cache_sim):
    if input_file is None:
        fatal("No input file provided for traffic generator.")

    if generator.strip() == "PacketTraceGen":
        return PacketTraceGen(trace = input_file, outstanding = 0,
            max_packets = limit, cache_align = cache_sim)
    elif generator.strip() == "HrdGen":
        return HrdGen(model_data = input_file, max_packets = limit)
    elif generator.strip() == "MocktailsGen":
        return MocktailsGen(model_data = input_file, outstanding = 0,
            max_packets = limit, cache_align = cache_sim)
    elif generator.strip() == "StmGen":
        return StmGen(model_data = input_file, max_packets = limit)
    else:
        fatal("Unknown generator requested.")


def add_main_memory(system, options):
    system.mem_ranges = [AddrRange(options.mem_address_range)]

    # create and connect DRAM to the system
    MemConfig.config_mem(options, system)

    for mem_ctrl in system.mem_ctrls:
        # don't save any data
        mem_ctrl.null = True

        if options.addr_map == 0:
           mem_ctrl.addr_mapping = "RoCoRaBaCh"
        elif options.addr_map == 1:
           mem_ctrl.addr_mapping = "RoRaBaCoCh"
        else:
            fatal("Invalid address map argument.")


def add_caches(system, options):
    system.l1_cache = L1Cache()
    system.l1_cache.size = options.l1_size
    system.l1_cache.assoc = options.l1_assoc

    system.l2_cache = L2Cache()
    system.l2_cache.size = options.l2_size
    system.l2_cache.assoc = options.l2_assoc

