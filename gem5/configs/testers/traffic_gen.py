# Copyright (c) 2017 ARM Limited
# All rights reserved.
#
# The license below extends only to copyright in the software and shall
# not be construed as granting a license to any other intellectual
# property including but not limited to intellectual property relating
# to a hardware implementation of the functionality of the software
# licensed hereunder.  You may use the software subject to the license
# terms below provided that you ensure that this notice is replicated
# unmodified and in its entirety in all distributions of the software,
# modified or unmodified, in source code or in binary form.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met: redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer;
# redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution;
# neither the name of the copyright holders nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Authors: Riken Gohil
#          Mario Badr


def get_cfg_file(input, name):
  if (".cfg" in input):
    # The input is already in the desired format
    return(input)
  elif (".ptrc" in input):
      # Create a dummy cfg that points to the protobuf trace
      cfg_file_name = name + ".cfg"

      cfg_file = open(cfg_file_name, 'w')
      config = ("STATE 0 100000000000 TRACE {0} 0" +
        "\nSTATE 1 1 EXIT" +
        "\nINIT 0" +
        "\nTRANSITION 0 1 1"
        "\nTRANSITION 1 1 1"
        "\n" ).format(input)
      cfg_file.write(config)
      cfg_file.close()

      return(cfg_file_name)
  elif (".stm" in input):
      # Create a dummy cfg that points to the protobuf model
      cfg_file_name = "stm.cfg"

      cfg_file = open(cfg_file_name, 'w')
      config = ("STATE 0 1 STM {0} 100" +
        "\nSTATE 1 1 EXIT" +
        "\nINIT 0" +
        "\nTRANSITION 0 1 1"
        "\nTRANSITION 1 1 1"
        "\n" ).format(input)
      cfg_file.write(config)
      cfg_file.close()

      return(cfg_file_name)
  else:
      fatal("Input file is not supported by TrafficGen.")

