#pragma once
#ifndef BLOCKSTORE_IMPLEMENTATIONS_ONDISK_ONDISKBLOCK_H_
#define BLOCKSTORE_IMPLEMENTATIONS_ONDISK_ONDISKBLOCK_H_

#include <boost/filesystem/path.hpp>
#include "../../interface/Block.h"
#include <messmer/cpp-utils/data/Data.h>
#include <iostream>

#include "messmer/cpp-utils/macros.h"

namespace blockstore {
namespace ondisk {
class OnDiskBlockStore;

class OnDiskBlock: public Block {
public:
  virtual ~OnDiskBlock();

  static std::unique_ptr<OnDiskBlock> LoadFromDisk(const boost::filesystem::path &rootdir, const Key &key);
  static std::unique_ptr<OnDiskBlock> CreateOnDisk(const boost::filesystem::path &rootdir, const Key &key, cpputils::Data data);
  static void RemoveFromDisk(const boost::filesystem::path &rootdir, const Key &key);

  const void *data() const override;
  void write(const void *source, uint64_t offset, uint64_t size) override;

  void flush() override;

  size_t size() const override;

private:
  const boost::filesystem::path _filepath;
  cpputils::Data _data;
  bool _dataChanged;

  OnDiskBlock(const Key &key, const boost::filesystem::path &filepath, cpputils::Data data);

  void _fillDataWithZeroes();
  void _storeToDisk() const;

  DISALLOW_COPY_AND_ASSIGN(OnDiskBlock);
};

}
}

#endif
