/* Copyright (c) 2018 - present, VE Software Inc. All rights reserved
 *
 * This source code is licensed under Apache 2.0 License
 *  (found in the LICENSE.Apache file in the root directory)
 */

#ifndef KVSTORE_ROCKSENGINE_H_
#define KVSTORE_ROCKSENGINE_H_

#include <gtest/gtest_prod.h>
#include <rocksdb/db.h>
#include "base/Base.h"
#include "kvstore/KVIterator.h"
#include "kvstore/KVEngine.h"

namespace nebula {
namespace kvstore {

class RocksRangeIter : public KVIterator {
public:
    RocksRangeIter(rocksdb::Iterator* iter, rocksdb::Slice start, rocksdb::Slice end)
        : iter_(iter)
        , start_(start)
        , end_(end) {}

    ~RocksRangeIter()  = default;

    bool valid() const override {
        return !!iter_ && iter_->Valid() && (iter_->key().compare(end_) < 0);
    }

    void next() override {
        iter_->Next();
    }

    void prev() override {
        iter_->Prev();
    }

    folly::StringPiece key() const override {
        return folly::StringPiece(iter_->key().data(), iter_->key().size());
    }

    folly::StringPiece val() const override {
        return folly::StringPiece(iter_->value().data(), iter_->value().size());
    }

private:
    std::unique_ptr<rocksdb::Iterator> iter_;
    rocksdb::Slice start_;
    rocksdb::Slice end_;
};


class RocksPrefixIter : public KVIterator {
public:
    RocksPrefixIter(rocksdb::Iterator* iter, rocksdb::Slice prefix)
        : iter_(iter)
        , prefix_(prefix) {}

    ~RocksPrefixIter()  = default;

    bool valid() const override {
        return !!iter_ && iter_->Valid() && (iter_->key().starts_with(prefix_));
    }

    void next() override {
        iter_->Next();
    }

    void prev() override {
        iter_->Prev();
    }

    folly::StringPiece key() const override {
        return folly::StringPiece(iter_->key().data(), iter_->key().size());
    }

    folly::StringPiece val() const override {
        return folly::StringPiece(iter_->value().data(), iter_->value().size());
    }

private:
    std::unique_ptr<rocksdb::Iterator> iter_;
    rocksdb::Slice prefix_;
};


class RocksEngine : public KVEngine {
    FRIEND_TEST(RocksEngineTest, SimpleTest);

public:
    RocksEngine(GraphSpaceID spaceId,
                const std::string& dataPath,
                std::shared_ptr<rocksdb::MergeOperator> mergeOp = nullptr,
                std::shared_ptr<rocksdb::CompactionFilterFactory> cfFactory = nullptr);

    ~RocksEngine();

    ResultCode get(const std::string& key,
                   std::string* value) override;

    ResultCode put(std::string key,
                   std::string value) override;

    ResultCode multiPut(std::vector<KV> keyValues) override;

    ResultCode range(const std::string& start,
                     const std::string& end,
                     std::unique_ptr<KVIterator>* iter) override;

    ResultCode prefix(const std::string& prefix,
                      std::unique_ptr<KVIterator>* iter) override;

    ResultCode remove(const std::string& key) override;

    ResultCode removeRange(const std::string& start,
                           const std::string& end) override;

    void addPart(PartitionID partId) override;

    void removePart(PartitionID partId) override;

    std::vector<PartitionID> allParts() override;

    int32_t totalPartsNum() override;

    ResultCode ingest(const std::vector<std::string>& files) override;

    ResultCode setOption(const std::string& config_key,
                         const std::string& config_value) override;

    ResultCode setDBOption(const std::string& config_key,
                           const std::string& config_value) override;

private:
    std::string partKey(PartitionID partId);

private:
    std::string  dataPath_;
    std::string  extraPath_;
    std::unique_ptr<rocksdb::DB> db_{nullptr};
    int32_t partsNum_ = -1;
};

}  // namespace kvstore
}  // namespace nebula
#endif  // KVSTORE_ROCKSENGINE_H_
