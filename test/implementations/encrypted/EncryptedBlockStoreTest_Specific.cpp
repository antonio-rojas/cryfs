#include <google/gtest/gtest.h>
#include "testutils/FakeAuthenticatedCipher.h"
#include "../../../implementations/encrypted/EncryptedBlockStore.h"
#include "../../../implementations/testfake/FakeBlockStore.h"
#include "../../../utils/BlockStoreUtils.h"
#include <messmer/cpp-utils/data/DataFixture.h>

using ::testing::Test;

using std::unique_ptr;
using std::make_unique;

using cpputils::DataFixture;
using cpputils::Data;

using blockstore::testfake::FakeBlockStore;

using namespace blockstore::encrypted;

class EncryptedBlockStoreTest: public Test {
public:
  static constexpr unsigned int BLOCKSIZE = 1024;
  EncryptedBlockStoreTest():
    _baseBlockStore(make_unique<FakeBlockStore>()),
    baseBlockStore(_baseBlockStore.get()),
    blockStore(make_unique<EncryptedBlockStore<FakeAuthenticatedCipher>>(std::move(_baseBlockStore), FakeAuthenticatedCipher::Key1())),
    data(DataFixture::generate(BLOCKSIZE)) {
  }
  unique_ptr<FakeBlockStore> _baseBlockStore;
  FakeBlockStore *baseBlockStore;
  unique_ptr<EncryptedBlockStore<FakeAuthenticatedCipher>> blockStore;
  Data data;

  blockstore::Key CreateBlockDirectlyWithFixtureAndReturnKey() {
    return blockStore->create(data)->key();
  }

  blockstore::Key CreateBlockWriteFixtureToItAndReturnKey() {
    auto block = blockStore->create(Data(data.size()));
    block->write(data.data(), 0, data.size());
    return block->key();
  }

  void ModifyBaseBlock(const blockstore::Key &key) {
    auto block = baseBlockStore->load(key);
    uint8_t middle_byte = ((byte*)block->data())[10];
    uint8_t new_middle_byte = middle_byte + 1;
    block->write(&new_middle_byte, 10, 1);
  }

  blockstore::Key CopyBaseBlock(const blockstore::Key &key) {
    auto source = baseBlockStore->load(key);
    return blockstore::utils::copyToNewBlock(baseBlockStore, *source)->key();
  }
};

TEST_F(EncryptedBlockStoreTest, LoadingWithSameKeyWorks_WriteOnCreate) {
  auto key = CreateBlockDirectlyWithFixtureAndReturnKey();
  auto loaded = blockStore->load(key);
  EXPECT_NE(nullptr, loaded.get());
  EXPECT_EQ(data.size(), loaded->size());
  EXPECT_EQ(0, std::memcmp(data.data(), loaded->data(), data.size()));
}

TEST_F(EncryptedBlockStoreTest, LoadingWithSameKeyWorks_WriteSeparately) {
  auto key = CreateBlockWriteFixtureToItAndReturnKey();
  auto loaded = blockStore->load(key);
  EXPECT_NE(nullptr, loaded.get());
  EXPECT_EQ(data.size(), loaded->size());
  EXPECT_EQ(0, std::memcmp(data.data(), loaded->data(), data.size()));
}

TEST_F(EncryptedBlockStoreTest, LoadingWithDifferentKeyDoesntWork_WriteOnCreate) {
  auto key = CreateBlockDirectlyWithFixtureAndReturnKey();
  blockStore->__setKey(FakeAuthenticatedCipher::Key2());
  auto loaded = blockStore->load(key);
  EXPECT_EQ(nullptr, loaded.get());
}

TEST_F(EncryptedBlockStoreTest, LoadingWithDifferentKeyDoesntWork_WriteSeparately) {
  auto key = CreateBlockWriteFixtureToItAndReturnKey();
  blockStore->__setKey(FakeAuthenticatedCipher::Key2());
  auto loaded = blockStore->load(key);
  EXPECT_EQ(nullptr, loaded.get());
}

TEST_F(EncryptedBlockStoreTest, LoadingModifiedBlockFails_WriteOnCreate) {
  auto key = CreateBlockDirectlyWithFixtureAndReturnKey();
  ModifyBaseBlock(key);
  auto loaded = blockStore->load(key);
  EXPECT_EQ(nullptr, loaded.get());
}

TEST_F(EncryptedBlockStoreTest, LoadingModifiedBlockFails_WriteSeparately) {
  auto key = CreateBlockWriteFixtureToItAndReturnKey();
  ModifyBaseBlock(key);
  auto loaded = blockStore->load(key);
  EXPECT_EQ(nullptr, loaded.get());
}

TEST_F(EncryptedBlockStoreTest, LoadingWithDifferentBlockIdFails_WriteOnCreate) {
  auto key = CreateBlockDirectlyWithFixtureAndReturnKey();
  auto key2 = CopyBaseBlock(key);
  auto loaded = blockStore->load(key2);
  EXPECT_EQ(nullptr, loaded.get());
}

TEST_F(EncryptedBlockStoreTest, LoadingWithDifferentBlockIdFails_WriteSeparately) {
  auto key = CreateBlockWriteFixtureToItAndReturnKey();
  auto key2 = CopyBaseBlock(key);
  auto loaded = blockStore->load(key2);
  EXPECT_EQ(nullptr, loaded.get());
}
