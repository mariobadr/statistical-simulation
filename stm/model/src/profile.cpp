#include "stm/profile.hpp"

namespace stm {

profile::profile(parameters const &p)
    : sdc(p.num_rows, p.num_cols)
    , spc(p.stride_depth)
    , min_address(std::numeric_limits<std::uint64_t>::max())
{
}

void profile::update(uint64_t address, operation op)
{
  min_address = std::min(address, min_address);
  max_address = std::max(address, max_address);

  auto const tag_match = sdc.update(address);

  if(!tag_match) {
    spc.update(address);
  } else {
    sdc_update_count++;
  }

  if(op == operation::read) {
    read_count++;
  } else if(op == operation::write) {
    write_count++;
  }
}
}