#include <iostream>
#include <thread>
#include <vector>
#include <cassert>

#include "serialization.h"

using namespace SERIALIZATION;

static void fail(const char* msg)
{
    std::cerr << "FAIL: " << msg << std::endl;
    std::exit(1);
}

static void ok(const char* msg)
{
    std::cout << "PASS: " << msg << std::endl;
}

void test_construction_and_serialize()
{
    Serialization root;
    auto *v = new SInt32("count", 7);
    root.add_member(v);

    auto json = root.get_json();
    if (json["count"] != 7) fail("construction/serialize: value mismatch");

    ok("construction & serialize");
    delete v; // cleanup (Serialization does not delete member pointer itself)
}

void test_copy_shares_pointer_and_refcount_behavior()
{
    Serialization root;
    auto *p = new SInt32("n", 10);
    root.add_member(p);

    {
        Serialization copy = root; // copy should copy the map (pointers) and inc_ref each member
        // changing the underlying value via the original pointer should be visible in the copy
        p->get_value() = 42;
        auto json = copy.get_json();
        if (json["n"] != 42) fail("copy: shared pointer did not reflect updated value");
    }

    // copy destroyed -> dec_ref called. original still holds the member.
    auto json = root.get_json();
    if (json["n"] != 42) fail("copy: original lost value after copy destruction");

    ok("copy shares pointer and maintains refcounts correctly (no crash)");

    delete p; // cleanup
}

void test_swap_behaves_like_move()
{
    SInt32 a("a", 1);
    SInt32 b("b", 2);

    // swap via base class -> should exchange names and stored values
    static_cast<SERTypeBase&>(a).swap(static_cast<SERTypeBase&>(b));

    if (a.get_name() != "b") fail("swap: name not swapped");
    if (b.get_name() != "a") fail("swap: name not swapped (other)");
    if (a.get_value() != 2) fail("swap: value not swapped");
    if (b.get_value() != 1) fail("swap: value not swapped (other)");

    ok("swap (move-like) semantics");
}

void test_multithreaded_incdec_no_crash()
{
    // Create a single heap-allocated member. Threads will add/remove references to it.
    auto *shared = new SInt32("shared", 123);

    const int THREADS = 16;
    const int ITER = 2000;

    auto worker = [shared, ITER]() {
        for (int i = 0; i < ITER; ++i) {
            Serialization s;
            s.add_member(shared); // inc_ref
            // read the value (no concurrent writer) to exercise get_value()
            volatile int v = shared->get_value(); (void)v;
            // s goes out of scope -> its destructor will dec_ref
        }
    };

    std::vector<std::thread> ths;
    for (int i = 0; i < THREADS; ++i) ths.emplace_back(worker);
    for (auto &t : ths) t.join();

    // After all threads joined, reference count should have returned to 1 (the original)
    // Deleting 'shared' should not crash (will call destructor and dec_ref)
    delete shared;

    ok("multithreaded add/remove references (no crash)");
}

int main()
{
    std::cout << "Running serialization unit tests...\n";

    test_construction_and_serialize();
    test_copy_shares_pointer_and_refcount_behavior();
    test_swap_behaves_like_move();
    test_multithreaded_incdec_no_crash();

    std::cout << "All tests passed.\n";
    return 0;
}
