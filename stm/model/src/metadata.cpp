#include "stm/metadata.hpp"

#include "stm.pb.h"

namespace stm {

std::unique_ptr<profile> read(ioproto::istream &stream)
{
  Configuration proto_config;

  if(!stream.read(&proto_config)) {
    return nullptr;
  }

  profile::parameters params;
  params.num_cols = proto_config.sdc_col_count();
  params.num_rows = proto_config.sdc_row_count();
  params.stride_depth = proto_config.stride_depth();

  auto p = std::make_unique<profile>(params);

  {
    GlobalCounts proto_global;

    if(!stream.read(&proto_global)) {
      throw std::runtime_error("Could not read global counts from file.");
    }

    p->write_count = proto_global.write_count();
    p->read_count = proto_global.total_requests() - p->write_count;

    p->sdc_update_count = proto_global.sdc_count();
    p->min_address = proto_global.min_address();
    p->max_address = proto_global.max_address();

    p->spc.start_address = proto_global.spc_start_address();
    p->spc.last_address = p->spc.start_address;
  }

  {
    for(std::size_t i = 0; i < proto_config.sdc_row_count(); i++) {
      SDCRow proto_row;

      if(!stream.read(&proto_row)) {
        throw std::runtime_error("Could not read SDC row from file.");
      }

      for(int col = 0; col < proto_row.count_size(); ++col) {
        auto const j = static_cast<std::size_t>(col);

        p->sdc.rows[i].columns[j].count = proto_row.count(col);
        p->sdc.rows[i].columns[j].tag = proto_row.tag(col);
        p->sdc.rows[i].columns[j].valid = true;
      }
    }
  }

  {
    for(std::size_t i = 0; i < proto_config.spc_row_count(); i++) {
      SPCRow proto_row;

      if(!stream.read(&proto_row)) {
        throw std::runtime_error("Could not read SPC row from file.");
      }

      history_sequence pattern(proto_config.stride_depth());
      for(int j = proto_row.stride_history_size() - 1; j >= 0; --j) {
        pattern.add(proto_row.stride_history(j));
      }

      for(int j = 0; j < proto_row.next_stride_size(); j++) {
        std::int64_t const stride = proto_row.next_stride(j);
        std::uint64_t const count = proto_row.count(j);

        p->spc.stride_patterns.set(pattern, stride, count);
      }
    }
  }

  return p;
}

void append(ioproto::ofstream &stream, profile const &p)
{
  {
    Configuration config{};
    config.set_sdc_row_count(p.sdc.row_size());
    config.set_sdc_col_count(p.sdc.column_size());
    config.set_spc_row_count(p.spc.stride_patterns.size());
    config.set_stride_depth(p.spc.stride_pattern_depth());

    stream.write(config);
  }

  {
    GlobalCounts global{};
    global.set_total_requests(p.count());
    global.set_sdc_count(p.sdc_update_count);
    global.set_write_count(p.write_count);

    global.set_spc_start_address(p.spc.start_address);
    global.set_min_address(p.min_address);
    global.set_max_address(p.max_address);

    stream.write(global);
  }

  {
    for(auto const &row : p.sdc.rows) {
      SDCRow proto_row{};

      for(auto const &column : row.columns) {
        proto_row.add_tag(column.tag);
        proto_row.add_count(column.count);
      }

      stream.write(proto_row);
    }
  }

  {
    for(auto const &row : p.spc.stride_patterns) {
      SPCRow proto_row{};

      // Save the entire stride history for hashing/pattern matching.
      for(auto const &stride : row.second.pattern) {
        proto_row.add_stride_history(stride);
      }

      // Save the next strides and their frequency.
      for(auto const &count_pair : row.second.counts) {
        proto_row.add_next_stride(count_pair.first);
        proto_row.add_count(count_pair.second);
      }

      stream.write(proto_row);
    }
  }
}
} // namespace stm