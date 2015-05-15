#pragma once
#ifndef BLOCKSTORE_IMPLEMENTATIONS_CACHING_NEWBLOCK_H_
#define BLOCKSTORE_IMPLEMENTATIONS_CACHING_NEWBLOCK_H_

#include "../../interface/BlockStore.h"
#include <messmer/cpp-utils/data/Data.h>

#include "messmer/cpp-utils/macros.h"
#include <memory>

namespace blockstore {
namespace caching {
class CachingBlockStore;

//TODO Does it make sense to write a general DataBackedBlock that just stores a Data object and maps the block operations to it?
//     Can we reuse that object somewhere else?
//     Maybe a second abstract class for BlockRefBackedBlock?

// This is a block that was created in CachingBlockStore, but doesn't exist in the base block store yet.
// It only exists in the cache and it is created in the base block store when destructed.
class NewBlock: public Block {
public:
  NewBlock(const Key &key, cpputils::Data data, CachingBlockStore *blockStore);
  virtual ~NewBlock();

  const void *data() const override;
  void write(const void *source, uint64_t offset, uint64_t size) override;
  void flush() override;

  size_t size() const override;

  void remove();

  bool alreadyExistsInBaseStore() const;

private:
  CachingBlockStore *_blockStore;
  cpputils::Data _data;
  std::unique_ptr<Block> _baseBlock;
  bool _dataChanged;

  void writeToBaseBlockIfChanged();

  DISALLOW_COPY_AND_ASSIGN(NewBlock);
};

}
}

#endif
