import optparse
import m5

# import all simulation objects from gem5
from m5.objects import *

from statsim import *


def parse_args():
    parser = optparse.OptionParser()

    # Arguments for configuring the traffic generator.
    parser.add_option("--generator", type="string", default=None,
        help="The traffic generator to use.")

    parser.add_option("--generator-input", type="string", default=None,
        help="The input to the traffic generator.")

    parser.add_option("--generator-limit", type="int", default=0,
        help="Limit the number of packets generated.")

    # Arguments for monitoring the traffic generator.
    parser.add_option("--save-generator-trace", type="int", default="0",
        help="Set to 1 to attach a probe that generates a trace.")

    parser.add_option("--save-generator-footprint", type="int", default="1",
        help="Set to 1 to attach a probe that tracks the memory footprint.")

    # Arguments for configuring the system.
    parser.add_option("--system-frequency", type="string" ,default="1000MHz",
       help = "System frequency")

    # Arguments for configuring main memory.
    parser.add_option("--mem-type", type="choice", default="LPDDR3_1600_1x32",
        choices=MemConfig.mem_names(), help = "Type of memory to use.")

    parser.add_option("--mem-channels", type="int", default=4,
        help = "Number of memory channels")

    parser.add_option("--mem-address-range", type="string", default="32GB",
        help = "System memory address range in MB")

    parser.add_option("--addr_map", type="int", default=1,
        # Ro: row, Co: column, Ra: rank, Ba: bank, Ch: channel
        help = "0: RoCoRaBaCh; 1: RoRaBaCoCh/RoRaBaChCo")

    (options, args) = parser.parse_args()

    return(options)


options = parse_args()

# Configure the system.
system = System()
system.mmap_using_noreserve = True
system.mem_mode = 'timing'

# Specify the clock domain used by the system.
system.clk_domain = SrcClockDomain()
system.clk_domain.clock = options.system_frequency
system.clk_domain.voltage_domain = VoltageDomain(voltage = '1V')

# Create the interconnects.
system.membus = IOXBar(width = 32)

# Create the components.
add_main_memory(system, options)
system.tgen = create_generator(options.generator, options.generator_input,
    options.generator_limit, False)

# Add the system probes.
system.monitor = CommMonitor()

if options.save_generator_trace == 1:
    system.monitor.trace = MemTraceProbe(trace_file = "monitor.ptrc.gz")

if options.save_generator_footprint == 1:
    system.monitor.footprint = MemFootprintProbe()

# Connect the system to the membus.
system.system_port = system.membus.slave
system.monitor.master = system.membus.slave
system.tgen.port = system.monitor.slave

# Set up the simulation.
root = Root(full_system = False, system = system)
m5.instantiate()

# Launch the simulation.
print("Starting the simulation.")
exit_event = m5.simulate()
print("Exiting @ tick %i because %s" % (m5.curTick(),
    exit_event.getCause()))
