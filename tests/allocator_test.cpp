#include <gtest/gtest.h>
#include "memory_manager.h"

class MemoryManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup if needed
    }
};

TEST_F(MemoryManagerTest, AllocZeroReturnsNull) {
    MemoryManager mm(1024 * 1024);
    ASSERT_EQ(mm.alloc(0), nullptr);
}

TEST_F(MemoryManagerTest, AllocTooBigReturnsNull) {
    MemoryManager mm(1024 * 1024);
    ASSERT_EQ(mm.alloc(2 * 1024 * 1024), nullptr);
}

TEST_F(MemoryManagerTest, FreeNullDoesNotCrash) {
    MemoryManager mm(1024 * 1024);
    ASSERT_NO_THROW(mm.free(nullptr));
}

TEST_F(MemoryManagerTest, DoubleFreeIsSafe) {
    MemoryManager mm(1024 * 1024);
    void* p = mm.alloc(100);
    ASSERT_NE(p, nullptr);
    mm.free(p);
    ASSERT_NO_THROW(mm.free(p)); // Should handle double-free gracefully
}

TEST_F(MemoryManagerTest, BasicAllocAndFree) {
    MemoryManager mm(1024 * 1024);
    void* p1 = mm.alloc(100);
    ASSERT_NE(p1, nullptr);
    void* p2 = mm.alloc(200);
    ASSERT_NE(p2, nullptr);
    mm.free(p1);
    mm.free(p2);
}

TEST_F(MemoryManagerTest, SlabAllocatorIntegration) {
    MemoryManager mm(1024 * 1024);
    void* p1 = mm.alloc(32);     // Should go to slab
    void* p2 = mm.alloc(8192);   // Should go to page manager
    ASSERT_NE(p1, nullptr);
    ASSERT_NE(p2, nullptr);
    mm.free(p1);
    mm.free(p2);
}

TEST_F(MemoryManagerTest, FragmentationCalculation) {
    MemoryManager mm(4 * 4096); // 4 pages
    ASSERT_DOUBLE_EQ(mm.getFragmentation(), 0.0);

    void* p1 = mm.alloc(4096); // page 0
    void* p2 = mm.alloc(4096); // page 1
    void* p3 = mm.alloc(4096); // page 2
    void* p4 = mm.alloc(4096); // page 3

    ASSERT_NE(p1, nullptr);
    ASSERT_NE(p2, nullptr);
    ASSERT_NE(p3, nullptr);
    ASSERT_NE(p4, nullptr);

    mm.free(p1); // free page 0
    mm.free(p3); // free page 2

    // Now pages: [free, used, free, used]
    double frag1 = mm.getFragmentation();
    EXPECT_NEAR(frag1, 0.5, 1e-6); // 2 free pages, max block=1 → 1 - 1/2 = 0.5

    mm.free(p2); // free page 1
    // Now pages: [free, free, free, used]
    double frag2 = mm.getFragmentation();
    EXPECT_NEAR(frag2, 0.0, 1e-6); // 3 free pages contiguous → 1 - 3/3 = 0.0
}
